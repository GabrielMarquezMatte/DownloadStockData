#pragma once
#include <string>

class http_client
{
public:
    static std::string get(const char *url);
};