#pragma once
#include <concurrentqueue/concurrentqueue.h>
#include <atomic>
#include <stop_token>
#include <fstream>
#include "formatting_download.hpp"
#include "parse_data.hpp"
#include <arrow/io/file.h>
#include <parquet/stream_writer.h>
#include "../include/date_operations.hpp"
#include <pqxx/pqxx>

class OutputWriter
{
public:
    virtual void execute(moodycamel::ConcurrentQueue<CotBovespa> &queue, std::atomic<bool> &done, std::stop_token token) = 0;
};

class CsvWriter final : virtual public OutputWriter
{
private:
    std::ofstream m_output_file;

public:
    CsvWriter()
    {
        m_output_file.open("output.csv", std::ios::binary | std::ios::trunc | std::ios::out);
        m_output_file << "Date;BDI;Negotiation Code;ISIN Code;Specification;Market Type;Term;Opening Price;Max Price;Min Price;Average Price;Closing Price;Exercise Price;Expiration Date;Quote Factor;Days in Month\n";
    }
    void execute(moodycamel::ConcurrentQueue<CotBovespa> &queue, std::atomic<bool> &done, std::stop_token token) override
    {
        CotBovespa cotacao;
        bool ran = false;
        while ((!done || !ran) && !token.stop_requested())
        {
            while (queue.try_dequeue(cotacao) && !token.stop_requested())
            {
                char buffer[212];
                std::size_t size = parse_csv_line(cotacao, buffer);
                m_output_file.write(buffer, size);
            }
            ran = true;
        }
        m_output_file.close();
    }
};

class ParquetWriter final : virtual public OutputWriter
{
private:
    std::shared_ptr<arrow::io::FileOutputStream> m_output_file;
    std::shared_ptr<parquet::schema::GroupNode> m_schema;
    parquet::WriterProperties::Builder builder;
    static std::shared_ptr<parquet::schema::GroupNode> GetSchema()
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

public:
    ParquetWriter()
    {
        PARQUET_ASSIGN_OR_THROW(m_output_file, arrow::io::FileOutputStream::Open("output.parquet"));
        m_schema = GetSchema();
        builder.compression(parquet::Compression::SNAPPY);
        builder.version(parquet::ParquetVersion::PARQUET_2_LATEST);
    }
    void execute(moodycamel::ConcurrentQueue<CotBovespa> &queue, std::atomic<bool> &done, std::stop_token token) override
    {
        parquet::StreamWriter writer{parquet::ParquetFileWriter::Open(m_output_file, m_schema, builder.build())};
        CotBovespa cotacao;
        bool ran = false;
        while ((!done || !ran) && !token.stop_requested())
        {
            while (queue.try_dequeue(cotacao) && !token.stop_requested())
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
    }
};

class PostgresWriter final : virtual public OutputWriter
{
private:
    const char *m_create_temp_table = "CREATE TEMP TABLE temp_cotacoes AS TABLE tcot_bovespa WITH NO DATA";
    const char *m_insert_into_table = "INSERT INTO tcot_bovespa SELECT temp_cotacoes.* FROM temp_cotacoes LEFT JOIN tcot_bovespa ON temp_cotacoes.dt_pregao = tcot_bovespa.dt_pregao AND temp_cotacoes.cd_codneg = tcot_bovespa.cd_codneg AND temp_cotacoes.prz_termo = tcot_bovespa.prz_termo WHERE tcot_bovespa.dt_pregao IS NULL";
    pqxx::connection m_connection;
    pqxx::work m_work;

    void write_to_table(const CotBovespa &cotacao, pqxx::stream_to &stream)
    {
        std::size_t size = 0;
        while (cotacao.cd_codneg[size] != '\0' && cotacao.cd_codneg[size] != ' ' && size < 12)
        {
            size++;
        }
        std::string_view codneg{cotacao.cd_codneg, size};
        std::string_view codisin{cotacao.cd_codisin, 12};
        std::string_view speci{cotacao.nm_speci, 10};
        char date_buffer[11];
        char expiration_date_buffer[11];
        auto date = cotacao.dt_pregao;
        auto expiration_date = cotacao.dt_datven;
        std::string_view date_str(date_buffer, tm_to_string(date, date_buffer));
        std::string_view expiration_date_str(expiration_date_buffer, tm_to_string(expiration_date, expiration_date_buffer));
        stream.write_values(date_str, cotacao.prz_termo, codneg, cotacao.cd_tpmerc, cotacao.cd_codbdi,
                            codisin, speci, cotacao.prec_aber, cotacao.prec_max, cotacao.prec_min, cotacao.prec_med,
                            cotacao.prec_fec, cotacao.prec_exer, expiration_date_str, cotacao.fat_cot, cotacao.nr_dismes);
    }

public:
    PostgresWriter(const std::string &connection_string) : m_connection(connection_string), m_work(m_connection)
    {
    }
    void execute(moodycamel::ConcurrentQueue<CotBovespa> &queue, std::atomic<bool> &done, std::stop_token token) override
    {
        m_work.exec0(m_create_temp_table);
        CotBovespa cotacao;
        bool ran = false;
        auto start = std::chrono::high_resolution_clock::now();
        pqxx::table_path table_name{"tcot_bovespa"};
        pqxx::table_path columns{"dt_pregao", "prz_termo", "cd_codneg", "cd_tpmerc", "cd_codbdi", "cd_codisin",
                                 "nm_speci", "prec_aber", "prec_max", "prec_min", "prec_med", "prec_fec", "prec_exer",
                                 "dt_datven", "fat_cot", "nr_dismes"};
        pqxx::stream_to stream = pqxx::stream_to::table(m_work, table_name, columns);
        while ((!done || !ran) && !token.stop_requested())
        {
            while (queue.try_dequeue(cotacao) && !token.stop_requested())
            {
                write_to_table(cotacao, stream);
            }
            ran = true;
        }
        stream.complete();
        spdlog::info("Created temp table");
        spdlog::info("Inserting quotes into tcot_bovespa table");
        m_work.exec0(m_insert_into_table);
        m_work.commit();
        auto end = std::chrono::high_resolution_clock::now();
        spdlog::info("Inserted quotes in {} ms", std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
    }
};

class ConsoleWriter final : virtual public OutputWriter
{
public:
    void execute(moodycamel::ConcurrentQueue<CotBovespa> &queue, std::atomic<bool> &done, std::stop_token token) override
    {
        CotBovespa cotacao;
        bool ran = false;
        while ((!done || !ran) && !token.stop_requested())
        {
            while (queue.try_dequeue(cotacao) && !token.stop_requested())
            {
                char buffer[212];
                std::size_t size = parse_csv_line(cotacao, buffer);
                spdlog::info("{}", std::string_view(buffer, size));
            }
            ran = true;
        }
    }
};