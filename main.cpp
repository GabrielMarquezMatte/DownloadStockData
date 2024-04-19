#include <string>
#include <ctime>
#include <semaphore>
#include <iostream>
#include <thread>
#include <chrono>
#include <format>
#include <fstream>
#include <concurrentqueue/concurrentqueue.h>
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
    moodycamel::ConcurrentQueue<CotBovespa>& queue;
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

void process_lines_csv(moodycamel::ConcurrentQueue<CotBovespa>& queue, std::atomic<bool>& done)
{
    std::ofstream output_file("output.csv", std::ios::binary | std::ios::trunc | std::ios::out);
    output_file << "Date;BDI;Negotiation Code;ISIN Code;Specification;Market Type;Term;Opening Price;Max Price;Min Price;Average Price;Closing Price;Exercise Price;Expiration Date;Quote Factor;Days in Month\n";
    CotBovespa cotacao;
    while(!done)
    {
        while(queue.try_dequeue(cotacao))
        {
            char buffer[212];
            std::size_t size = parse_csv_line(cotacao, buffer);
            output_file.write(buffer, size);
        }
    }
}

void process_lines_none(moodycamel::ConcurrentQueue<CotBovespa>& queue, std::atomic<bool>& done)
{
    CotBovespa cotacao;
    while(!done)
    {
        while(queue.try_dequeue(cotacao))
        {
            // Do nothing
        }
    }
}

void download_quotes(DownloadParameters parameters)
{
    parameters.semaphore->acquire();
    char buffer[10];
    auto result = date_to_log(parameters.date, parameters.type, buffer);
    std::string_view date_str(buffer, result);
    auto start = std::chrono::high_resolution_clock::now();
    std::string url = get_url(parameters.date, parameters.type);
    zip_archive zip = download_zip(url);
    auto end = std::chrono::high_resolution_clock::now();
    spdlog::info("Downloaded {} for date {} in {} ms", url, date_str, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    std::atomic<bool> done = false;
    start = std::chrono::high_resolution_clock::now();
    int count = read_quote_file(zip, parameters.queue);
    parameters.count += count;
    end = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds execution_time = end - start;
    {
        std::lock_guard<std::mutex> lock(parameters.sum_mutex);
        parameters.total_time += execution_time;
    }
    parameters.semaphore->release();
    spdlog::info("Downloaded {} quotes in {} ms for date {}", count, std::chrono::duration_cast<std::chrono::milliseconds>(execution_time).count(), date_str);
}

int main()
{
    const int Months = 48;
    std::counting_semaphore<> semaphore(16);
    std::thread threads[Months];
    std::chrono::nanoseconds total_time = std::chrono::nanoseconds::zero();
    std::atomic<int> count = 0;
    std::mutex sum_mutex;
    std::tm start_date = {0, 0, 0, 1, 0, 2020 - 1900};
    date_t date = std::chrono::system_clock::from_time_t(std::mktime(&start_date));
    moodycamel::ConcurrentQueue<CotBovespa> queue;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    for (int i = 0; i < Months; i++)
    {
        if(date >= now)
        {
            break;
        }
        DownloadParameters parameters{&semaphore, sum_mutex, date, Type::MONTH, total_time, count, queue};
        threads[i] = std::thread(download_quotes, parameters);
        date += std::chrono::months(1);
    }
    std::atomic<bool> done = false;
    std::jthread writer(process_lines_csv, std::ref(queue), std::ref(done));
    for(auto& thread : threads)
    {
        thread.join();
    }
    done = true;
    writer.join();
    spdlog::info("Downloaded {} quotes in {} ms", count.load(), std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count());
    return 0;
}