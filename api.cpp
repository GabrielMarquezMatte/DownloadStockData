#include <restbed>
#include <pqxx/pqxx>
#include <iostream>
#include <memory>
#include <cxxopts.hpp>
#include "include/connection_pool.hpp"
#include "controllers/stock_data_controller.hpp"
std::shared_ptr<connection_pool> pool;

constexpr const char *BAD_REQUEST_RESPONSE = "{'error': 'Bad request'}";
constexpr const char *APPLICATION_JSON = "application/json";

enum VALIDATION_ERROR : char
{
    INVALID_DATE = 0,
    INVALID_SYMBOL = 1,
    INVALID_START_DATE = 2,
    INVALID_END_DATE = 3,
    NO_ERROR = 4
};

void no_content_response(const std::shared_ptr<restbed::Session> session)
{
    session->close(restbed::BAD_REQUEST, BAD_REQUEST_RESPONSE, {{"Content-Length", "24"}, {"Content-Type", APPLICATION_JSON}});
}

void validation_error_response(const std::shared_ptr<restbed::Session> session, const std::string &error_message)
{
    session->close(restbed::UNPROCESSABLE_ENTITY, error_message, {{"Content-Length", std::to_string(error_message.length())}, {"Content-Type", APPLICATION_JSON}});
}

bool authenticate_user(const std::shared_ptr<restbed::Session> session, const std::string &api_key)
{
    if (api_key.empty())
    {
        return true;
    }
    const auto& request = session->get_request();
    const auto& headers = request->get_headers();
    const auto& auth_header = headers.find("X-Api-Key");
    if (auth_header == headers.end())
    {
        session->close(restbed::UNAUTHORIZED, "{'error': 'Please provide an API key'}", {{"Content-Length", "38"}, {"Content-Type", APPLICATION_JSON}});
        return false;
    }
    if (auth_header->second != api_key)
    {
        session->close(restbed::UNAUTHORIZED, "{'error': 'Invalid API key'}", {{"Content-Length", "28"}, {"Content-Type", APPLICATION_JSON}});
        return false;
    }
    return true;
}

VALIDATION_ERROR validate_date(const std::string &start_date, const std::string &end_date)
{
    std::istringstream start_date_stream(start_date);
    std::istringstream end_date_stream(end_date);
    std::tm start_date_tm = {};
    std::tm end_date_tm = {};
    start_date_stream >> std::get_time(&start_date_tm, "%Y-%m-%d");
    end_date_stream >> std::get_time(&end_date_tm, "%Y-%m-%d");
    if (start_date_stream.fail() || end_date_stream.fail())
    {
        if (start_date_stream.fail())
        {
            return VALIDATION_ERROR::INVALID_START_DATE;
        }
        else
        {
            return VALIDATION_ERROR::INVALID_END_DATE;
        }
    }
    std::time_t start_date_time = std::mktime(&start_date_tm);
    std::time_t end_date_time = std::mktime(&end_date_tm);
    if (start_date_time > end_date_time)
    {
        return VALIDATION_ERROR::INVALID_DATE;
    }
    return VALIDATION_ERROR::NO_ERROR;
}

bool validate_request(const std::shared_ptr<restbed::Session> &session, const std::string &symbol, const std::string &start_date, const std::string &end_date)
{
    if (symbol.empty() || start_date.empty() || end_date.empty())
    {
        no_content_response(session);
        return false;
    }
    VALIDATION_ERROR date_validation_error = validate_date(start_date, end_date);
    if (date_validation_error != VALIDATION_ERROR::NO_ERROR)
    {
        std::string error_message;
        switch (date_validation_error)
        {
        case VALIDATION_ERROR::INVALID_DATE:
            error_message = "{'error': 'Invalid date'}";
            break;
        case VALIDATION_ERROR::INVALID_START_DATE:
            error_message = "{'error': 'Invalid start date'}";
            break;
        case VALIDATION_ERROR::INVALID_END_DATE:
            error_message = "{'error': 'Invalid end date'}";
            break;
        default:
            error_message = "{'error': 'Unknown error'}";
            break;
        }
        validation_error_response(session, error_message);
        return false;
    }
    return true;
}

void get_stock_data(const std::shared_ptr<restbed::Session> &session)
{
    const auto &request = session->get_request();
    std::string symbol = request->get_path_parameter("symbol");
    std::string start_date = request->get_query_parameter("start_date");
    std::string end_date = request->get_query_parameter("end_date");
    if (!validate_request(session, symbol, start_date, end_date))
        return;
    std::string data = GetStockData(pool, symbol, start_date, end_date);
    session->close(restbed::OK, data, {{"Content-Length", std::to_string(data.length())}, {"Content-Type", APPLICATION_JSON}});
}

int main(int argc, char **argv)
{
    std::string connection_string = "dbname=testdb user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
    int pool_size = 30;
    int port = 8080;
    unsigned int worker_limit = std::thread::hardware_concurrency();
    std::string api_key;
    cxxopts::Options options("StockDataApi", "API for getting stock data");
    options.add_options()
        ("c,connection", "Connection string", cxxopts::value<std::string>(connection_string))
        ("p,pool", "Pool size", cxxopts::value<int>(pool_size))
        ("w,workers", "Worker limit", cxxopts::value<unsigned int>(worker_limit))
        ("port", "Port", cxxopts::value<int>(port))
        ("api_key", "API key", cxxopts::value<std::string>(api_key))
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
    stock_data_resource->set_authentication_handler([api_key](const std::shared_ptr<restbed::Session> session, const std::function<void(const std::shared_ptr<restbed::Session>)>& callback)
                                                    {
                                                        bool authenticated = authenticate_user(session, api_key);
                                                        if (!authenticated)
                                                        {
                                                            return;
                                                        }
                                                        callback(session);
                                                    });
    std::shared_ptr<restbed::Settings> settings = std::make_shared<restbed::Settings>();
    settings->set_port((uint16_t)port);
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