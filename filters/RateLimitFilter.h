#pragma once
#include <drogon/HttpFilter.h>
#include <unordered_map>
#include <tuple>
#include <thread>

class RateLimitFilter final : public drogon::HttpFilter<RateLimitFilter, false>
{
private:
    std::unordered_map<uint32_t, std::tuple<int, std::chrono::system_clock::time_point>> m_rate_limit;
    std::mutex m_mutex;
    int m_max_requests;
    std::chrono::seconds m_period;
public:
    RateLimitFilter(int max_requests, std::chrono::seconds period) : m_max_requests(max_requests), m_period(period) {}
    void doFilter(const drogon::HttpRequestPtr &req,
                  drogon::FilterCallback &&fcb,
                  drogon::FilterChainCallback &&fccb) override;
};

