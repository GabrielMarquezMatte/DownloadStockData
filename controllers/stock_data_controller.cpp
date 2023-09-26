#include "./stock_data_controller.hpp"
#include "../include/connection_pool.hpp"
#include "../src/formatting_download.hpp"
#include <pqxx/pqxx>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

std::string GetStockData(std::shared_ptr<connection_pool> &pool,const std::string &symbol, const std::string &start_date, const std::string &end_date)
{
    const std::string query = "SELECT * FROM tcot_bovespa WHERE cd_codneg = $1 AND dt_pregao BETWEEN $2 AND $3 ORDER BY dt_pregao ASC";
    auto connection = pool->get_connection();
    try
    {
        pqxx::work work(*connection);
        pqxx::result result = work.exec_params(query, symbol, start_date, end_date);
        nlohmann::json json_data;
        for (auto row : result)
        {
            nlohmann::json json_cot;
            json_cot["dt_pregao"] = row["dt_pregao"].as<std::string>();
            json_cot["cd_codbdi"] = row["cd_codbdi"].as<int>();
            json_cot["cd_codneg"] = row["cd_codneg"].as<std::string>();
            json_cot["cd_codisin"] = row["cd_codisin"].as<std::string>();
            json_cot["nm_speci"] = row["nm_speci"].as<std::string>();
            json_cot["cd_tpmerc"] = row["cd_tpmerc"].as<int>();
            json_cot["prz_termo"] = row["prz_termo"].as<int>();
            json_cot["prec_aber"] = row["prec_aber"].as<double>();
            json_cot["prec_max"] = row["prec_max"].as<double>();
            json_cot["prec_min"] = row["prec_min"].as<double>();
            json_cot["prec_med"] = row["prec_med"].as<double>();
            json_cot["prec_fec"] = row["prec_fec"].as<double>();
            json_cot["prec_exer"] = row["prec_exer"].as<double>();
            json_cot["dt_datven"] = row["dt_datven"].as<std::string>();
            json_cot["fat_cot"] = row["fat_cot"].as<int>();
            json_cot["nr_dismes"] = row["nr_dismes"].as<int>();
            json_data.push_back(json_cot);
        }
        work.commit();
        pool->return_connection(connection);
        return json_data.dump();
    }
    catch (const std::exception &e)
    {
        pool->return_connection(connection);
        std::cerr << e.what() << '\n';
        return "[]";
    }
}
