#pragma once
#include <string>
#include <drogon/HttpController.h>
#include <unordered_map>
#include <tuple>
#include <thread>

class StockDataController final : public drogon::HttpController<StockDataController, false>
{
private:
    std::unordered_map<std::string, std::tuple<drogon::HttpResponsePtr, std::chrono::system_clock::time_point>> m_cache;
    std::jthread m_cache_thread{&StockDataController::CheckCache, this};
    drogon::HttpAppFramework &m_app;
    std::mutex m_mutex;
    void CheckCache();
    void InsertCache(const std::string &key, const drogon::HttpResponsePtr &value);
    drogon::HttpResponsePtr& GetCache(const std::string &key);
    bool CacheHit(const std::string &key);
public:
    StockDataController(drogon::HttpAppFramework &app) : m_app(app) {}
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(StockDataController::GetStockData, "/{symbol}/{start_date}/{end_date}", drogon::Get, "RateLimitFilter");
    METHOD_LIST_END
    void GetStockData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, std::string &&symbol, const std::string &start_date, const std::string &end_date);
};