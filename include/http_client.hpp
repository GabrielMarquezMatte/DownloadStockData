#pragma once
#include <curl/curl.h>
#include <string>
#include <unordered_map>
#include <vector>

class http_client_pool
{
public:
    http_client_pool();
    ~http_client_pool();
    void get(const std::string &url, std::string *response);
    void perform();

private:
    CURLM *multi_handle;
    std::vector<std::string*> buffers;
    std::vector<CURL *> easy_handles;
};

class http_client
{
public:
    static std::string get(const std::string &url);
};