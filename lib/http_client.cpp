#include "../include/http_client.hpp"
#include <iostream>
#include <curl/curl.h>
#include <stop_token>

size_t write_data(char *ptr, size_t size, size_t nmemb, std::string *stream)
{
    stream->append(ptr, size * nmemb);
    return size * nmemb;
}

#pragma warning(disable : 4100)
static size_t progress_callback(std::stop_token *clientp,
                                double dltotal,
                                double dlnow,
                                double ultotal,
                                double ulnow)
{
    return clientp->stop_requested();
}
#pragma warning(default : 4100)

std::string http_client::get(const char *url, std::stop_token token)
{
    CURL *easy_handle = curl_easy_init();
    if (easy_handle == nullptr)
    {
        return "";
    }
    std::string response;
    curl_easy_setopt(easy_handle, CURLOPT_URL, url);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(easy_handle, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(easy_handle, CURLOPT_PROGRESSFUNCTION, progress_callback);
    curl_easy_setopt(easy_handle, CURLOPT_PROGRESSDATA, &token);
    CURLcode code = curl_easy_perform(easy_handle);
    if (code != CURLE_OK)
    {
        curl_easy_cleanup(easy_handle);
        return "";
    }
    long response_code;
    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
    if (response_code != 200)
    {
        curl_easy_cleanup(easy_handle);
        return "";
    }
    curl_easy_cleanup(easy_handle);
    return response;
}
