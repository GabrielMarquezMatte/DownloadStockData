#include <string>
#include <ctime>
#include <semaphore>
#include <iostream>
#include <thread>
#include <chrono>
#include <format>
#include <fstream>
#include <spdlog/spdlog.h>
#include <http_client.hpp>
#include <zip_archive.hpp>
#include <formatting_download.hpp>
#include <parse_data.hpp>

using date_t = std::chrono::system_clock::time_point;

static const std::string base_url = "https://bvmf.bmfbovespa.com.br/InstDados/SerHist/COTAHIST_";

enum class Type
{
    DAY,
    MONTH,
    YEAR,
};

enum class OutputType
{
    None,
    CSV,
};

struct DownloadParameters
{
    std::counting_semaphore<>* semaphore;
    std::mutex& sum_mutex;
    const date_t date;
    const Type type;
    std::chrono::nanoseconds& total_time;
    std::atomic<int>& count;
    OutputType output_type = OutputType::CSV;
};

struct LineProcessorParameters
{
    moodycamel::ReaderWriterQueue<CotBovespa>* queue;
    std::atomic<bool>* done;
    int* inner_count;
    OutputType output_type;
    std::ofstream* output_file;
};

std::size_t date_to_log(const date_t &date, Type type, char buffer[10])
{
    switch (type)
    {
        case Type::DAY:
            return std::format_to_n(buffer, 10, "{:%d/%m/%Y}", date).size;
        case Type::MONTH:
            return std::format_to_n(buffer, 7, "{:%m/%Y}", date).size;
        case Type::YEAR:
            return std::format_to_n(buffer, 4, "{:%Y}", date).size;
    }
    std::unreachable();
}

std::size_t date_to_string(const date_t &date, Type type, char buffer[8])
{
    switch (type)
    {
        case Type::DAY:
            return std::format_to_n(buffer, 8, "{:%d%m%Y}", date).size;
        case Type::MONTH:
            return std::format_to_n(buffer, 6, "{:%m%Y}", date).size;
        case Type::YEAR:
            return std::format_to_n(buffer, 4, "{:%Y}", date).size;
    }
    std::unreachable();
}

constexpr char type_to_char(Type type)
{
    switch (type)
    {
        case Type::DAY:
            return 'D';
        case Type::MONTH:
            return 'M';
        case Type::YEAR:
            return 'A';
    }
    std::unreachable();
}

std::string get_url(const date_t &date,const Type type)
{
    char buffer[8];
    std::size_t result = date_to_string(date, type, buffer);
    std::string date_str(buffer, result);
    return base_url + type_to_char(type) + date_str + ".ZIP";
}

void process_lines(const LineProcessorParameters& parameters)
{
    OutputType output_type = parameters.output_type;
    std::ofstream* output_file = parameters.output_file;
    moodycamel::ReaderWriterQueue<CotBovespa>* queue = parameters.queue;
    std::atomic<bool>* done = parameters.done;
    int inner_count = 0;
    if(output_type == OutputType::CSV)
    {
        (*output_file) << "Date;BDI;Negotiation Code;ISIN Code;Specification;Market Type;Term;Opening Price;Max Price;Min Price;Average Price;Closing Price;Exercise Price;Expiration Date;Quote Factor;Days in Month\n";
    }
    CotBovespa cotacao;
    while (!*done)
    {
        while (queue->try_dequeue(cotacao))
        {
            if(output_type == OutputType::CSV)
            {
                char buffer[212];
                std::size_t size = parse_csv_line(cotacao, buffer);
                output_file->write(buffer, size);
            }
            inner_count++;
        }
    }
    (*parameters.inner_count) = inner_count;
}

void download_quotes(DownloadParameters parameters)
{
    parameters.semaphore->acquire();
    char buffer[10];
    auto result = date_to_log(parameters.date, parameters.type, buffer);
    std::string_view date_str(buffer, result);
    LineProcessorParameters line_parameters;
    if(parameters.output_type == OutputType::CSV)
    {
        char date_str2[8];
        result = date_to_string(parameters.date, parameters.type, date_str2);
        std::string file_name = "./output_" + std::string(date_str2, result) + ".csv";
        std::ofstream* output_file = new std::ofstream(file_name, std::ios::binary | std::ios::trunc);
        if(output_file == nullptr || !output_file->is_open())
        {
            if(output_file != nullptr)
            {
                delete output_file;
            }
            parameters.semaphore->release();
            spdlog::error("Failed to open file for date {}", date_str);
            return;
        }
        line_parameters.output_file = output_file;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::string url = get_url(parameters.date, parameters.type);
    zip_archive zip = download_zip(url);
    auto end = std::chrono::high_resolution_clock::now();
    spdlog::info("Downloaded {} for date {} in {} ms", url, date_str, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    moodycamel::ReaderWriterQueue<CotBovespa> queue(1000);
    std::atomic<bool> done = false;
    int inner_count = 0;
    line_parameters.queue = &queue;
    line_parameters.done = &done;
    line_parameters.output_type = parameters.output_type;
    line_parameters.inner_count = &inner_count;
    start = std::chrono::high_resolution_clock::now();
    std::jthread writer(read_lines, std::ref(zip), std::ref(queue), std::ref(done));
    std::jthread reader(process_lines, line_parameters);
    writer.join();
    reader.join();
    line_parameters.output_file->close();
    end = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds execution_time = end - start;
    {
        std::lock_guard<std::mutex> lock(parameters.sum_mutex);
        parameters.total_time += execution_time;
        parameters.count += inner_count;
    }
    parameters.semaphore->release();
    spdlog::info("Downloaded {} quotes in {} ms for date {}", inner_count, std::chrono::duration_cast<std::chrono::milliseconds>(execution_time).count(), date_str);
    if(line_parameters.output_file != nullptr)
    {
        delete line_parameters.output_file;
    }
}

int main()
{
    const int Months = 24;
    std::counting_semaphore<> semaphore(24);
    std::thread threads[Months];
    std::chrono::nanoseconds total_time = std::chrono::nanoseconds::zero();
    std::atomic<int> count = 0;
    std::mutex sum_mutex;
    std::tm start_date = {0, 0, 0, 1, 0, 2020 - 1900};
    date_t date = std::chrono::system_clock::from_time_t(std::mktime(&start_date));
    for (int i = 0; i < Months; i++)
    {
        DownloadParameters parameters{&semaphore, sum_mutex, date, Type::MONTH, total_time, count};
        threads[i] = std::thread(download_quotes, parameters);
        date += std::chrono::months(1);
    }
    for(auto& thread : threads)
    {
        thread.join();
    }
    spdlog::info("Downloaded {} quotes in {} ms", count.load(), std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count());
    return 0;
}