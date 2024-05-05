#pragma once
#include <string>
#include <drogon/HttpController.h>
#include <unordered_map>
#include <chrono>
#include <tuple>
#include <thread>
class StockDataController final : public drogon::HttpController<StockDataController, false>
{
private:
    std::string m_connection_string;
    std::unordered_map<std::string, std::tuple<std::string, std::chrono::system_clock::time_point>> m_cache;
    std::jthread m_cache_thread{&StockDataController::CheckCache, this};
    void CheckCache();
public:
    StockDataController(std::string &&connection_string) : m_connection_string(std::move(connection_string)) {};
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(StockDataController::GetStockData, "/{symbol}/{start_date}/{end_date}", drogon::Get);
    METHOD_LIST_END

    void GetStockData(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback, const std::string &symbol, const std::string &start_date, const std::string &end_date);
};