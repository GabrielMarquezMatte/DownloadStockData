#pragma once
#include <string>
#include <stop_token>

class http_client
{
public:
    static std::string get(const char *url, std::stop_token token);
};