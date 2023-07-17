#pragma once
#include "../include/connection_pool.hpp"
#include "formatting_download.hpp"
#include <pqxx/pqxx>
#include <iostream>

void insert_into_table(connection_pool &pool, const std::vector<CotBovespa> &data)
{
    auto connection = pool.get_connection();
    const char *query = "INSERT INTO tcot_bovespa (dt_pregao, cd_codbdi, cd_codneg, cd_tpmerc, prz_termo, prec_aber, "
                        "prec_max, prec_min, prec_med, prec_fec, prec_exer, dt_datven, fat_cot, cd_codisin, nr_dismes, nm_speci) "
                        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14, $15, $16) "
                        "ON CONFLICT (dt_pregao, cd_codneg, prz_termo) DO UPDATE SET "
                        "cd_codbdi = EXCLUDED.cd_codbdi, cd_tpmerc = EXCLUDED.cd_tpmerc, prec_aber = EXCLUDED.prec_aber, "
                        "prec_max = EXCLUDED.prec_max, prec_min = EXCLUDED.prec_min, prec_med = EXCLUDED.prec_med, "
                        "prec_fec = EXCLUDED.prec_fec, prec_exer = EXCLUDED.prec_exer, dt_datven = EXCLUDED.dt_datven, "
                        "fat_cot = EXCLUDED.fat_cot, cd_codisin = EXCLUDED.cd_codisin, nr_dismes = EXCLUDED.nr_dismes, "
                        "nm_speci = EXCLUDED.nm_speci";
    connection->prepare("insert_cotbovespa", query);
    pqxx::work transaction(*connection);
    for (const auto &item : data)
    {
        // Convert std::tm to string for SQL statement
        std::ostringstream dt_pregao_stream, dt_datven_stream;
        dt_pregao_stream << std::put_time(&item.dt_pregao, "%Y-%m-%d");
        dt_datven_stream << std::put_time(&item.dt_datven, "%Y-%m-%d");

        // Execute prepared SQL statement with values
        transaction.exec_prepared("insert_cotbovespa", dt_pregao_stream.str(), item.cd_codbdi, item.cd_codneg, item.cd_tpmerc, item.prz_termo, item.prec_aber,
                                  item.prec_max, item.prec_min, item.prec_med, item.prec_fec, item.prec_exer, dt_datven_stream.str(),
                                  item.fat_cot, item.cd_codisin, item.nr_dismes, item.nm_speci);
    }
    transaction.commit();
    connection->unprepare("insert_cotbovespa");
    pool.return_connection(connection);
}