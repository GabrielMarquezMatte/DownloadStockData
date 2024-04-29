#include <string>
#include <ctime>
#include <semaphore>
#include <iostream>
#include <thread>
#include <chrono>
#include <format>
#include <fstream>
#include <concurrentqueue/concurrentqueue.h>
#include <arrow/io/file.h>
#include <parquet/stream_writer.h>
#include <spdlog/spdlog.h>
#include <http_client.hpp>
#include <zip_archive.hpp>
#include <formatting_download.hpp>
#include <parse_data.hpp>
#include <string_operations.hpp>
#include "connection_pool.hpp"
#include <pqxx/pqxx>

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
    std::counting_semaphore<> &semaphore;
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

int days_to_epoch(std::tm &date)
{
    std::tm epoch = {0, 0, 0, 1, 0, 70};
    std::time_t epoch_time = std::mktime(&epoch);
    std::time_t date_time = std::mktime(&date);
    return static_cast<int>((date_time - epoch_time) / (60 * 60 * 24));
}

int days_to_epoch(std::chrono::system_clock::time_point &date)
{
    return std::chrono::duration_cast<std::chrono::days>(date.time_since_epoch()).count();
}

void process_lines_csv(moodycamel::ConcurrentQueue<CotBovespa> &queue, const std::atomic<bool> &done)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::ofstream output_file("output.csv", std::ios::binary | std::ios::trunc | std::ios::out);
    output_file << "Date;BDI;Negotiation Code;ISIN Code;Specification;Market Type;Term;Opening Price;Max Price;Min Price;Average Price;Closing Price;Exercise Price;Expiration Date;Quote Factor;Days in Month\n";
    CotBovespa cotacao;
    bool ran = false;
    while (!done || !ran)
    {
        while (queue.try_dequeue(cotacao))
        {
            char buffer[212];
            std::size_t size = parse_csv_line(cotacao, buffer);
            output_file.write(buffer, size);
        }
        ran = true;
    }
    output_file.close();
    std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
    spdlog::info("Wrote output.csv in {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - now).count());
}

std::shared_ptr<parquet::schema::GroupNode> GetSchema()
{
    parquet::schema::NodeVector fields;
    fields.push_back(parquet::schema::PrimitiveNode::Make("Date", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("BDI", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Negotiation Code", parquet::Repetition::REQUIRED, parquet::Type::FIXED_LEN_BYTE_ARRAY, parquet::ConvertedType::NONE, 16));
    fields.push_back(parquet::schema::PrimitiveNode::Make("ISIN Code", parquet::Repetition::REQUIRED, parquet::Type::FIXED_LEN_BYTE_ARRAY, parquet::ConvertedType::NONE, 16));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Specification", parquet::Repetition::REQUIRED, parquet::Type::FIXED_LEN_BYTE_ARRAY, parquet::ConvertedType::NONE, 16));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Market Type", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Term", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Opening Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Max Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Min Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Average Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Closing Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Exercise Price", parquet::Repetition::REQUIRED, parquet::Type::DOUBLE));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Expiration Date", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Quote Factor", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    fields.push_back(parquet::schema::PrimitiveNode::Make("Days in Month", parquet::Repetition::REQUIRED, parquet::Type::INT32, parquet::ConvertedType::INT_32));
    return std::static_pointer_cast<parquet::schema::GroupNode>(parquet::schema::GroupNode::Make("cotacoes", parquet::Repetition::REQUIRED, fields));
}

void process_lines_parquet(moodycamel::ConcurrentQueue<CotBovespa> &queue, const std::atomic<bool> &done)
{
    std::shared_ptr<arrow::io::FileOutputStream> output_file;
    PARQUET_ASSIGN_OR_THROW(output_file, arrow::io::FileOutputStream::Open("output.parquet"));
    std::shared_ptr<parquet::schema::GroupNode> schema = GetSchema();
    parquet::WriterProperties::Builder builder;
    builder.compression(parquet::Compression::SNAPPY);
    builder.version(parquet::ParquetVersion::PARQUET_2_LATEST);
    parquet::StreamWriter writer{parquet::ParquetFileWriter::Open(output_file, schema, builder.build())};
    CotBovespa cotacao;
    bool ran = false;
    auto start = std::chrono::high_resolution_clock::now();
    while (!done || !ran)
    {
        while (queue.try_dequeue(cotacao))
        {
            int date = days_to_epoch(cotacao.dt_pregao);
            int expiration_date = days_to_epoch(cotacao.dt_datven);
            writer << date;
            writer << cotacao.cd_codbdi;
            writer << cotacao.cd_codneg;
            writer << cotacao.cd_codisin;
            writer << cotacao.nm_speci;
            writer << cotacao.cd_tpmerc;
            writer << cotacao.prz_termo;
            writer << cotacao.prec_aber;
            writer << cotacao.prec_max;
            writer << cotacao.prec_min;
            writer << cotacao.prec_med;
            writer << cotacao.prec_fec;
            writer << cotacao.prec_exer;
            writer << expiration_date;
            writer << cotacao.fat_cot;
            writer << cotacao.nr_dismes;
            writer.EndRow();
        }
        ran = true;
    }
    auto end = std::chrono::high_resolution_clock::now();
    spdlog::info("Wrote output.parquet in {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void process_lines_none(moodycamel::ConcurrentQueue<CotBovespa> &queue, const std::atomic<bool> &done)
{
    CotBovespa cotacao;
    bool ran = false;
    while (!done || !ran)
    {
        while (queue.try_dequeue(cotacao))
        {
            // Do nothing
        }
        ran = true;
    }
}

void process_lines_postgres(moodycamel::ConcurrentQueue<CotBovespa> &queue, const std::atomic<bool> &done)
{
    const constexpr char *connection_string = "dbname=testdb user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
    const constexpr char *create_temp_table = "CREATE TEMP TABLE temp_cotacoes AS TABLE tcot_bovespa WITH NO DATA";
    ConnectionPool pool(connection_string, 16);
    CotBovespa cotacao;
    auto conn = pool.get_connection();
    pqxx::work txn(*conn);
    txn.exec0(create_temp_table);
    auto start = std::chrono::high_resolution_clock::now();
    bool ran = false;
    pqxx::stream_to stream = pqxx::stream_to::table(txn, {"temp_cotacoes"}, {"dt_pregao", "prz_termo", "cd_codneg", "cd_tpmerc", "cd_codbdi", "cd_codisin", "nm_speci", "prec_aber", "prec_max", "prec_min", "prec_med", "prec_fec", "prec_exer", "dt_datven", "fat_cot", "nr_dismes"});
    while (!done || !ran)
    {
        while (queue.try_dequeue(cotacao))
        {
            std::string_view codneg{cotacao.cd_codneg, 12};
            std::string_view codisin{cotacao.cd_codisin, 12};
            std::string_view speci{cotacao.nm_speci, 10};
            char date_buffer[11];
            char expiration_date_buffer[11];
            auto date = cotacao.dt_pregao;
            auto expiration_date = cotacao.dt_datven;
            std::string_view date_str(date_buffer, tm_to_string(date, date_buffer));
            std::string_view expiration_date_str(expiration_date_buffer, tm_to_string(expiration_date, expiration_date_buffer));
            stream.write_values(date_str, cotacao.prz_termo, codneg, cotacao.cd_tpmerc, cotacao.cd_codbdi, codisin, speci, cotacao.prec_aber, cotacao.prec_max, cotacao.prec_min, cotacao.prec_med, cotacao.prec_fec, cotacao.prec_exer, expiration_date_str, cotacao.fat_cot, cotacao.nr_dismes);
        }
        ran = true;
    }
    stream.complete();
    spdlog::info("Created temp table");
    const constexpr char *insert_into_table = "INSERT INTO tcot_bovespa SELECT * FROM temp_cotacoes ON CONFLICT DO NOTHING";
    spdlog::info("Inserting quotes into tcot_bovespa table");
    txn.exec0(insert_into_table);
    txn.commit();
    auto end = std::chrono::high_resolution_clock::now();
    spdlog::info("Inserted quotes in {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

void download_quotes(DownloadParameters parameters)
{
    parameters.semaphore.acquire();
    char buffer[10];
    auto result = date_to_log(parameters.date, parameters.type, buffer);
    std::string_view date_str(buffer, result);
    auto start = std::chrono::high_resolution_clock::now();
    std::string url = get_url(parameters.date, parameters.type);
    std::string content = http_client::get(url.data());
    if (content.empty())
    {
        spdlog::error("Failed to download {} for date {}", url, date_str);
        parameters.semaphore.release();
        return;
    }
    zip_archive zip(content);
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
    parameters.semaphore.release();
    spdlog::info("Downloaded {} quotes in {} ms for date {}", count, std::chrono::duration_cast<std::chrono::milliseconds>(execution_time).count(), date_str);
}

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        spdlog::error("Usage: {} <start_date> <end_date>", argv[0]);
        return 1;
    }
    std::counting_semaphore<> semaphore(16);
    std::chrono::nanoseconds total_time = std::chrono::nanoseconds::zero();
    std::atomic<int> count = 0;
    std::mutex sum_mutex;
    std::istringstream start_date_stream(argv[1]);
    std::istringstream end_date_stream(argv[2]);
    date_t start_date;
    start_date_stream >> std::chrono::parse("%Y-%m-%d", start_date);
    if(!start_date_stream)
    {
        spdlog::error("Invalid start date");
        return 1;
    }
    date_t end_date;
    end_date_stream >> std::chrono::parse("%Y-%m-%d", end_date);
    if(!end_date_stream)
    {
        spdlog::error("Invalid end date");
        return 1;
    }
    if(start_date > end_date)
    {
        spdlog::error("Start date is greater than end date");
        return 1;
    }
    moodycamel::ConcurrentQueue<CotBovespa> queue;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::vector<std::thread> threads;
    threads.reserve(std::chrono::duration_cast<std::chrono::months>(end_date - start_date).count() + 1);
    while(start_date <= now && start_date <= end_date)
    {
        DownloadParameters parameters{semaphore, sum_mutex, start_date, Type::MONTH, total_time, count, queue};
        threads.emplace_back(download_quotes, parameters);
        start_date += std::chrono::months(1);
    }
    std::atomic<bool> done = false;
    std::jthread writer(process_lines_parquet, std::ref(queue), std::ref(done));
    for (auto &thread : threads)
    {
        thread.join();
    }
    done = true;
    writer.join();
    spdlog::info("Downloaded {} quotes in {} ms", count.load(), std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count());
    return 0;
}