#include <drogon/drogon.h>
#include "controllers/stock_data_controller.hpp"
int main()
{
    auto& app = drogon::app();
    app.registerController(std::make_shared<StockDataController>("dbname=testdb user=postgres password=postgres hostaddr=127.0.0.1 port=5432"));
    app.addListener("0.0.0.0", 80);
    app.setThreadNum(16);
    app.setLogPath("./").setLogLevel(trantor::Logger::kInfo);
    app.run();
}