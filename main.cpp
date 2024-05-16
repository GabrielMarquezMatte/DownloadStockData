#include <string>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <format>
#include <iostream>
#include <concurrentqueue/concurrentqueue.h>
#include <spdlog/spdlog.h>
#include "./include/zip_archive.hpp"
#include "./include/thread_pool.hpp"
#include "./src/formatting_download.hpp"
#include "./src/writers.hpp"

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
    PARQUET,
    POSTGRES,
};

struct DownloadParameters
{
    std::mutex &sum_mutex;
    const date_t date;
    const Type type;
    std::chrono::nanoseconds &total_time;
    std::atomic<int> &count;
    moodycamel::ConcurrentQueue<CotBovespa> &queue;
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

std::string get_url(const date_t &date, const Type type)
{
    char buffer[8];
    std::size_t result = date_to_string(date, type, buffer);
    std::string date_str(buffer, result);
    return base_url + type_to_char(type) + date_str + ".ZIP";
}

void download_quotes(DownloadParameters parameters, std::stop_token token)
{
    char buffer[10];
    auto result = date_to_log(parameters.date, parameters.type, buffer);
    std::string_view date_str(buffer, result);
    auto start = std::chrono::high_resolution_clock::now();
    std::string url = get_url(parameters.date, parameters.type);
    std::string content = http_client::get(url.data(), token);
    if (content.empty())
    {
        spdlog::error("Failed to download {} for date {}", url, date_str);
        return;
    }
    zip_archive zip(content);
    auto end = std::chrono::high_resolution_clock::now();
    spdlog::info("Downloaded {} for date {} in {} ms", url, date_str, std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    std::atomic<bool> done = false;
    start = std::chrono::high_resolution_clock::now();
    int count = read_quote_file(zip, parameters.queue, token);
    parameters.count += count;
    end = std::chrono::high_resolution_clock::now();
    std::chrono::nanoseconds execution_time = end - start;
    {
        std::lock_guard<std::mutex> lock(parameters.sum_mutex);
        parameters.total_time += execution_time;
    }
    spdlog::info("Downloaded {} quotes in {} ms for date {}", count, std::chrono::duration_cast<std::chrono::milliseconds>(execution_time).count(), date_str);
}

void stop_requested(std::stop_source &source)
{
    std::thread([&source]()
                {
        std::string line;
        while (std::getline(std::cin, line))
        {
            if (line == "q" || line == "quit" || line == "exit")
            {
                source.request_stop();
                spdlog::warn("Stop requested");
                return;
            }
        } })
        .detach();
}

std::optional<Type> parse_type(const char type)
{
    switch (type)
    {
    case 'D':
        return Type::DAY;
    case 'M':
        return Type::MONTH;
    case 'A':
        return Type::YEAR;
    default:
        return std::nullopt;
    }
}

std::optional<OutputType> parse_output_type(const char type)
{
    switch (type)
    {
    case 'C':
        return OutputType::CSV;
    case 'P':
        return OutputType::PARQUET;
    case 'N':
        return OutputType::None;
    case 'S':
        return OutputType::POSTGRES;
    default:
        return std::nullopt;
    }
}

std::shared_ptr<OutputWriter> GetWriter(OutputType type, const std::string &connection_string)
{
    switch (type)
    {
    case OutputType::CSV:
        return std::make_shared<CsvWriter>();
    case OutputType::PARQUET:
        return std::make_shared<ParquetWriter>();
    case OutputType::POSTGRES:
        return std::make_shared<PostgresWriter>(connection_string);
    case OutputType::None:
        return std::make_shared<ConsoleWriter>();
    default:
        std::unreachable();
    }
}

int main(int argc, char **argv)
{
    if (argc < 5)
    {
        spdlog::error("Usage: {} <start_date> <end_date> <type> <output_type>", argv[0]);
        return 1;
    }
    std::istringstream start_date_stream(argv[1]);
    std::istringstream end_date_stream(argv[2]);
    date_t start_date;
    start_date_stream >> std::chrono::parse("%Y-%m-%d", start_date);
    if (!start_date_stream)
    {
        spdlog::error("Invalid start date");
        return 1;
    }
    date_t end_date;
    end_date_stream >> std::chrono::parse("%Y-%m-%d", end_date);
    if (!end_date_stream)
    {
        spdlog::error("Invalid end date");
        return 1;
    }
    if (start_date > end_date)
    {
        spdlog::error("Start date is greater than end date");
        return 1;
    }
    std::optional<Type> type = parse_type(argv[3][0]);
    if (!type.has_value())
    {
        spdlog::error("Invalid type {} must be D, M or A", argv[3][0]);
        return 1;
    }
    std::optional<OutputType> output_type = parse_output_type(argv[4][0]);
    if (!output_type.has_value())
    {
        spdlog::error("Invalid output type {} must be CSV, PARQUET, POSTGRES or NONE", argv[4]);
        return 1;
    }
    std::chrono::nanoseconds total_time = std::chrono::nanoseconds::zero();
    std::atomic<int> count = 0;
    std::mutex sum_mutex;
    moodycamel::ConcurrentQueue<CotBovespa> queue;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::stop_source source;
    std::stop_token token = source.get_token();
    stop_requested(source);
    ThreadPool pool;
    std::vector<std::future<void>> futures;
    while (start_date <= now && start_date <= end_date)
    {
        DownloadParameters parameters{sum_mutex, start_date, *type, total_time, count, queue};
        futures.push_back(pool.enqueue(download_quotes, parameters, token));
        switch (*type)
        {
        case Type::DAY:
            start_date += std::chrono::hours(24);
            break;
        case Type::MONTH:
            start_date += std::chrono::months(1);
            break;
        case Type::YEAR:
            start_date += std::chrono::years(1);
            break;
        }
    }
    std::atomic<bool> done = false;
    std::shared_ptr<OutputWriter> writer = GetWriter(*output_type, "dbname=testdb user=postgres password=postgres hostaddr=127.0.0.1 port=5432");
    if(writer == nullptr)
    {
        spdlog::error("Invalid output type");
        return 1;
    }
    std::jthread writer_thread([&writer, &queue, &done, token]()
                               { writer->execute(queue, done, token); });
    for (auto &thread : futures)
    {
        thread.wait();
    }
    done = true;
    writer_thread.join();
    spdlog::info("Downloaded {} quotes in {} ms", count.load(), std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count());
    return 0;
}