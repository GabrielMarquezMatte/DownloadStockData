#pragma once
#include <pqxx/connection>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>

class connection_pool
{
public:
    using connection_ptr = std::shared_ptr<pqxx::connection>;
    connection_pool(const std::string& connection_string, int pool_size);
    connection_ptr get_connection();
    void return_connection(connection_ptr connection);

private:
    std::string connection_string_;
    int pool_size_;
    std::queue<connection_ptr> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
};