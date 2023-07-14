#include "../include/http_client.hpp"
#include <iostream>

size_t write_data(char *ptr, size_t size, size_t nmemb, std::string *stream)
{
    stream->append(ptr, size * nmemb);
    return size * nmemb;
}

http_client_pool::http_client_pool()
{
    curl_global_init(CURL_GLOBAL_ALL);
    multi_handle = curl_multi_init();
}

http_client_pool::~http_client_pool()
{
    for (CURL *easy_handle : easy_handles)
    {
        curl_multi_remove_handle(multi_handle, easy_handle);
        curl_easy_cleanup(easy_handle);
    }
    curl_multi_cleanup(multi_handle);
    curl_global_cleanup();
}

void http_client_pool::get(const std::string &url, std::string *response)
{
    buffers.push_back(response);
    CURL *easy_handle = curl_easy_init();
    curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, response);
    curl_multi_add_handle(multi_handle, easy_handle);
    easy_handles.push_back(easy_handle);
}

void http_client_pool::perform()
{
    int still_running = 0;
    do
    {
        CURLMcode code = curl_multi_perform(multi_handle, &still_running);
        if (code != CURLM_OK)
        {
            std::cerr << "curl_multi_perform() failed: " << curl_multi_strerror(code) << std::endl;
            return;
        }
        if (!still_running)
        {
            break;
        }
        struct timeval timeout;
        int rc;
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;
        long curl_timeo = -1;
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        curl_multi_timeout(multi_handle, &curl_timeo);
        if (curl_timeo >= 0)
        {
            timeout.tv_sec = curl_timeo / 1000;
            if (timeout.tv_sec > 1)
            {
                timeout.tv_sec = 1;
            }
            else
            {
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
            }
        }
        curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
        rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        switch (rc)
        {
        case -1:
            break;
        case 0:
        default:
            curl_multi_perform(multi_handle, &still_running);
            break;
        }
    } while (still_running);
}

std::string http_client::get(const std::string &url)
{
    CURL *easy_handle = curl_easy_init();
    std::string response;
    curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(easy_handle, CURLOPT_WRITEDATA, &response);
    CURLcode code = curl_easy_perform(easy_handle);
    if (code != CURLE_OK)
    {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(code) << std::endl;
    }
    curl_easy_cleanup(easy_handle);
    return response;
}
