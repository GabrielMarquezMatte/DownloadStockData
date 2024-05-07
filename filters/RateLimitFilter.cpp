#include "RateLimitFilter.h"
void RateLimitFilter::doFilter(const drogon::HttpRequestPtr &req,
                         drogon::FilterCallback &&fcb,
                         drogon::FilterChainCallback &&fccb)
{
    const trantor::InetAddress &addr = req->peerAddr();
    uint32_t ip = addr.ipNetEndian();
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_rate_limit.find(ip);
    if(it == m_rate_limit.end())
    {
        m_rate_limit.insert_or_assign(ip, std::make_tuple(0, std::chrono::system_clock::now()));
        fccb();
        return;
    }
    auto now = std::chrono::system_clock::now();
    auto& [count, last_time] = it->second;
    if(count >= m_max_requests)
    {
        if(std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count() < m_period.count())
        {
            auto response = drogon::HttpResponse::newHttpResponse();
            response->setStatusCode(drogon::k429TooManyRequests);
            response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
            fcb(response);
            return;
        }
        count = 0;
    }
    if(std::chrono::duration_cast<std::chrono::seconds>(now - last_time).count() >= m_period.count())
    {
        last_time = now;
        count = 0;
    }
    ++count;
    fccb();
}
