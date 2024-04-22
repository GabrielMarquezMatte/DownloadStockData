#pragma once
#include "../include/connection_pool.hpp"
#include "formatting_download.hpp"

void insert_into_table(ConnectionPool &pool, const std::vector<CotBovespa> &data);