#include "../include/connection_pool.hpp"
#include <boost/pool/object_pool.hpp>

ConnectionPool::ConnectionPool(const std::string& connection_string, int pool_size)
    : connection_string_(connection_string)
    , pool_size_(pool_size)
    , pool(pool_size) {}

ConnectionPool::connection_ptr ConnectionPool::get_connection() noexcept
{
    auto conn = pool.construct(connection_string_);
    return connection_ptr(conn, [this](pqxx::connection* conn) {
        pool.destroy(conn);
    });
}