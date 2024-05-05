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

void StockDataController::InsertCache(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.insert_or_assign(key, std::make_tuple(value, std::chrono::system_clock::now()));
}

std::string& StockDataController::GetCache(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::get<0>(m_cache.at(key));
}

bool StockDataController::CacheHit(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cache.find(key) != m_cache.end();
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
void StockDataController::GetStockData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string &&symbol, const std::string &start_date, const std::string &end_date)
{
    std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
    std::string key = symbol + start_date + end_date;
    if(CacheHit(key))
    {
        auto response = drogon::HttpResponse::newHttpResponse();
        response->setStatusCode(drogon::k200OK);
        response->setBody(GetCache(key));
        response->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        callback(response);
        return;
    }
    auto db_connection = m_app.getDbClient();
    const char *query = "SELECT dt_pregao, prec_aber, prec_max, prec_min, prec_fec FROM tcot_bovespa WHERE cd_codneg = $1 AND dt_pregao >= $2 AND dt_pregao <= $3 ORDER BY dt_pregao";
    auto future = db_connection->execSqlAsyncFuture(query, symbol, start_date, end_date);
    auto result = future.get();
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
    InsertCache(key, body);
    callback(response);
}
#ifdef _MSC_VER
#pragma warning(pop)
#pragma warning(default : 4100)
#endif