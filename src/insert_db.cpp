#include "insert_db.hpp"
#include <pqxx/pqxx>
#include <pqxx/stream_to>
#include <iostream>

static bool operator<(const std::tm &lhs, const std::tm &rhs) noexcept
{
    if (lhs.tm_year < rhs.tm_year)
        return true;
    if (lhs.tm_year > rhs.tm_year)
        return false;
    if (lhs.tm_mon < rhs.tm_mon)
        return true;
    if (lhs.tm_mon > rhs.tm_mon)
        return false;
    if (lhs.tm_mday < rhs.tm_mday)
        return true;
    if (lhs.tm_mday > rhs.tm_mday)
        return false;
    return false;
}

static bool operator==(const std::tm &lhs, const std::tm &rhs) noexcept
{
    return lhs.tm_year == rhs.tm_year && lhs.tm_mon == rhs.tm_mon && lhs.tm_mday == rhs.tm_mday;
}

static bool operator<=(const std::tm &lhs, const std::tm &rhs) noexcept
{
    return lhs < rhs || lhs == rhs;
}

void insert_into_table(connection_pool &pool, const std::vector<CotBovespa> &data)
{
    auto connection = pool.get_connection();
    auto min_date = std::min_element(data.begin(), data.end(), [](const CotBovespa &lhs, const CotBovespa &rhs)
                                     { return lhs.dt_pregao < rhs.dt_pregao; });
    auto max_date = std::max_element(data.begin(), data.end(), [](const CotBovespa &lhs, const CotBovespa &rhs)
                                     { return lhs.dt_pregao < rhs.dt_pregao; });
    pqxx::work transaction(*connection);
    std::ostringstream min_date_stream, max_date_stream;
    min_date_stream << std::put_time(&min_date->dt_pregao, "%Y-%m-%d");
    max_date_stream << std::put_time(&max_date->dt_pregao, "%Y-%m-%d");
    transaction.exec("DELETE FROM tcot_bovespa WHERE dt_pregao BETWEEN '" + min_date_stream.str() + "' AND '" + max_date_stream.str() + "';");
    pqxx::stream_to stream = pqxx::stream_to::raw_table(transaction, "tcot_bovespa", "dt_pregao,cd_codbdi,cd_codneg,cd_tpmerc,prz_termo,prec_aber,"
                                                                                     "prec_max,prec_min,prec_med,prec_fec,prec_exer,dt_datven,fat_cot,cd_codisin,nr_dismes,nm_speci");
    for (const auto &item : data)
    {
        // Convert std::tm to string for SQL statement
        std::ostringstream dt_pregao_stream, dt_datven_stream;
        dt_pregao_stream << std::put_time(&item.dt_pregao, "%Y-%m-%d");
        dt_datven_stream << std::put_time(&item.dt_datven, "%Y-%m-%d");
        stream.write_values(dt_pregao_stream.str(), item.cd_codbdi, item.cd_codneg, item.cd_tpmerc, item.prz_termo, item.prec_aber,
                            item.prec_max, item.prec_min, item.prec_med, item.prec_fec, item.prec_exer, dt_datven_stream.str(),
                            item.fat_cot, item.cd_codisin, item.nr_dismes, item.nm_speci);
    }
    try
    {
        stream.complete();
        transaction.commit();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        transaction.abort();
    }
    pool.return_connection(connection);
}