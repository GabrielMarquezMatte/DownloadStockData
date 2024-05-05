#include <drogon/drogon.h>
#include <drogon/RateLimiter.h>
#include <chrono>
#include "controllers/stock_data_controller.hpp"
#include "filters/RateLimitFilter.h"
using namespace std::chrono_literals;
int main()
{
    auto& app = drogon::app();
    app.addListener("0.0.0.0", 80);
    app.setThreadNum(16);
    app.setLogPath("./").setLogLevel(trantor::Logger::kInfo);
    app.loadConfigFile("./config.json");
    app.registerFilter(std::make_shared<RateLimitFilter>(10, 1s));
    app.registerController(std::make_shared<StockDataController>(app));
    app.run();
}