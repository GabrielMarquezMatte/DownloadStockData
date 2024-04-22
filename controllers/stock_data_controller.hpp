#pragma once
#include <memory>
#include "../include/connection_pool.hpp"

std::string GetStockData(std::shared_ptr<ConnectionPool> &pool,const std::string &symbol, const std::string &start_date, const std::string &end_date);