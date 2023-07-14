#include "../include/connection_pool.hpp"

connection_pool::connection_pool(const std::string& connection_string, int pool_size)
    : connection_string_(connection_string)
    , pool_size_(pool_size)
{
    for(int i = 0; i < pool_size_; ++i)
    {
        connections_.push(std::make_shared<pqxx::connection>(connection_string_));
    }
}

connection_pool::connection_ptr connection_pool::get_connection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !connections_.empty(); });
    auto connection = connections_.front();
    connections_.pop();
    return connection;
}

void connection_pool::return_connection(connection_ptr connection)
{
    std::unique_lock<std::mutex> lock(mutex_);
    connections_.push(connection);
    cv_.notify_one();
}
