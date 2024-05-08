#include "./stock_data_controller.hpp"

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

void StockDataController::InsertCache(const std::string &key, const drogon::HttpResponsePtr &value)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cache.insert_or_assign(key, std::make_tuple(value, std::chrono::system_clock::now()));
}

drogon::HttpResponsePtr& StockDataController::GetCache(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return std::get<0>(m_cache.at(key));
}

bool StockDataController::CacheHit(const std::string &key)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_cache.find(key) != m_cache.end();
}

#pragma warning(disable : 4100)
void StockDataController::GetStockData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string &&symbol, const std::string &start_date, const std::string &end_date)
{
    std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
    std::string key = symbol + start_date + end_date;
    if(CacheHit(key))
    {
        callback(GetCache(key));
        return;
    }
    auto db_connection = m_app.getDbClient();
    const char *query = "SELECT dt_pregao, prec_aber, prec_max, prec_min, prec_fec FROM tcot_bovespa WHERE cd_codneg = $1 AND dt_pregao >= $2 AND dt_pregao <= $3 ORDER BY dt_pregao";
    auto future = db_connection->execSqlAsyncFuture(query, symbol, start_date, end_date);
    auto result = future.get();
    Json::Value root;
    int index = 0;
    for (const auto &row : result)
    {
        Json::Value body;
        body["date"] = row["dt_pregao"].c_str();
        body["open"] = row["prec_aber"].as<double>();
        body["high"] = row["prec_max"].as<double>();
        body["low"] = row["prec_min"].as<double>();
        body["close"] = row["prec_fec"].as<double>();
        root.insert(index++, body);
    }
    auto response = drogon::HttpResponse::newHttpJsonResponse(root);
    InsertCache(key, response);
    callback(response);
}
#pragma warning(default : 4100)