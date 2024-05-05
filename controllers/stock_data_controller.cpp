#include "./stock_data_controller.hpp"
#include <pqxx/pqxx>

void StockDataController::CheckCache()
{
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto now = std::chrono::system_clock::now();
        for(auto it = m_cache.begin(); it != m_cache.end();)
        {
            if(std::chrono::duration_cast<std::chrono::minutes>(now - std::get<1>(it->second)).count() > 5)
            {
                it = m_cache.erase(it);
                continue;
            }
            ++it;
        }
    }
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
void StockDataController::GetStockData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, const std::string &symbol, const std::string &start_date, const std::string &end_date)
{
    std::string symbol_upper = symbol;
    std::transform(symbol_upper.begin(), symbol_upper.end(), symbol_upper.begin(), ::toupper);
    std::string key = symbol_upper + start_date + end_date;
    if(m_cache.find(key) != m_cache.end())
    {
        auto response = drogon::HttpResponse::newHttpResponse();
        response->setStatusCode(drogon::k200OK);
        response->setBody(std::get<0>(m_cache.at(key)));
        response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(response);
        return;
    }
    pqxx::connection connection{m_connection_string};
    const char *query = "SELECT dt_pregao, prec_aber, prec_max, prec_min, prec_fec FROM tcot_bovespa WHERE cd_codneg = $1 AND dt_pregao >= $2 AND dt_pregao <= $3 ORDER BY dt_pregao";
    pqxx::work work(connection);
    pqxx::result result = work.exec_params(query, symbol_upper, start_date, end_date);
    auto response = drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::k200OK);
    std::string body = "[";
    for (const auto &row : result)
    {
        char buffer[128];
        int size = std::snprintf(buffer, sizeof(buffer), "{\"date\":\"%s\",\"open\":%.2f,\"high\":%.2f,\"low\":%.2f,\"close\":%.2f}", row["dt_pregao"].c_str(), row["prec_aber"].as<double>(), row["prec_max"].as<double>(), row["prec_min"].as<double>(), row["prec_fec"].as<double>());
        body.append(buffer, size);
        body += ',';
    }
    if(body.size() > 1)
    {
        body.pop_back();
    }
    body += ']';
    response->setBody(body);
    response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    std::tuple<std::string, std::chrono::system_clock::time_point> value = std::make_tuple(body, std::chrono::system_clock::now());
    m_cache.insert_or_assign(key, value);
    callback(response);
}
#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(default : 4100)
#endif