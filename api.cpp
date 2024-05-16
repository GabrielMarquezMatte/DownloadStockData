#include <drogon/drogon.h>
#include <drogon/RateLimiter.h>
#include <chrono>
#include "controllers/stock_data_controller.hpp"
#include "filters/RateLimitFilter.h"
using namespace std::chrono_literals;
int main()
{
    auto& app = drogon::app();
    app.setLogPath("./logs").setLogLevel(trantor::Logger::kInfo);
    app.loadConfigFile("./config.json");
    app.registerFilter(std::make_shared<RateLimitFilter>(100, 1s));
    app.registerController(std::make_shared<StockDataController>(app));
    app.run();
}