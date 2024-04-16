#include "../include/http_client.hpp"
#include <iostream>
#include <curl/curl.h>

size_t write_data(char *ptr, size_t size, size_t nmemb, std::string *stream)
{
    stream->append(ptr, size * nmemb);
    return size * nmemb;
}

std::string http_client::get(const char *url)
{
    CURL *easy_handle = curl_easy_init();
    if (easy_handle == nullptr)
    {
        std::cerr << "curl_easy_init() failed\n";
        return "";
    }
    std::string response;
    curl_easy_setopt(easy_handle, CURLOPT_URL, url);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
    CURLcode code = curl_easy_perform(easy_handle);
    if (code != CURLE_OK)
    {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(code) << '\n';
        curl_easy_cleanup(easy_handle);
        return "";
    }
    long response_code;
    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
    {
        std::cerr << "Response code: " << response_code << '\n';
        curl_easy_cleanup(easy_handle);
        return "";
    }
    curl_easy_cleanup(easy_handle);
    return response;
}
