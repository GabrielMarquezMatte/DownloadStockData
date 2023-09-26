#include <restbed>
#include <pqxx/pqxx>
#include <iostream>
#include <memory>
#include <cxxopts.hpp>
#include "include/connection_pool.hpp"
#include "controllers/stock_data_controller.hpp"
std::shared_ptr<connection_pool> pool;

void no_content_response(const std::shared_ptr<restbed::Session> session)
{
    session->close(restbed::BAD_REQUEST, "{'error': 'Bad request'}", {{"Content-Length", "24"}, {"Content-Type", "application/json"}});
}

void get_stock_data(const std::shared_ptr<restbed::Session> session)
{
    const auto &request = session->get_request();
    if (request->get_path_parameter("symbol").empty())
    {
        no_content_response(session);
        return;
    }
    if (request->get_query_parameter("start_date").empty())
    {
        no_content_response(session);
        return;
    }
    if (request->get_query_parameter("end_date").empty())
    {
        no_content_response(session);
        return;
    }
    std::string symbol = request->get_path_parameter("symbol");
    std::string start_date = request->get_query_parameter("start_date");
    std::string end_date = request->get_query_parameter("end_date");
    std::string data = GetStockData(pool, symbol, start_date, end_date);
    session->close(restbed::OK, data, {{"Content-Length", std::to_string(data.length())}, {"Content-Type", "application/json"}});
    return;
}

int main(int argc, char **argv)
{
    std::string connection_string = "dbname=testdb user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
    int pool_size = 30;
    int port = 8080;
    unsigned int worker_limit = std::thread::hardware_concurrency();
    cxxopts::Options options("StockDataApi", "API for getting stock data");
    options.add_options()
        ("c,connection", "Connection string", cxxopts::value<std::string>(connection_string))
        ("p,pool", "Pool size", cxxopts::value<int>(pool_size))
        ("w,workers", "Worker limit", cxxopts::value<unsigned int>(worker_limit))
        ("port", "Port", cxxopts::value<int>(port))
        ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        return 0;
    }
    pool = std::make_shared<connection_pool>(connection_string, pool_size);
    std::shared_ptr<restbed::Resource> stock_data_resource = std::make_shared<restbed::Resource>();
    stock_data_resource->set_path("/stock_data/{symbol: .*}");
    stock_data_resource->set_method_handler("GET", get_stock_data);
    std::shared_ptr<restbed::Settings> settings = std::make_shared<restbed::Settings>();
    settings->set_port((const uint16_t)port);
    settings->set_default_header("Connection", "close");
    settings->set_default_header("Access-Control-Allow-Origin", "*");
    settings->set_keep_alive(false);
    if (worker_limit > 1)
    {
        settings->set_worker_limit(worker_limit);
    }
    restbed::Service service;
    service.publish(stock_data_resource);
    service.start(settings);
    return EXIT_SUCCESS;
}