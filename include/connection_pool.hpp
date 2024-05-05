#pragma once
#include <pqxx/connection>
#include <string>
#include <memory>
#include <boost/pool/object_pool.hpp>

class ConnectionPool
{
public:
    using connection_ptr = std::shared_ptr<pqxx::connection>;
    ConnectionPool(const std::string& connection_string, int pool_size);
    connection_ptr get_connection() noexcept;
private:
    std::string connection_string_;
    int pool_size_;
    boost::object_pool<pqxx::connection> pool;
};