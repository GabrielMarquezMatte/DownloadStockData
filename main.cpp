#include <string>
#include <ctime>
#include <semaphore>
#include <iostream>
#include <thread>
#include <chrono>
#include <spdlog/spdlog.h>
#include <http_client.hpp>
#include <zip_archive.hpp>
#include <formatting_download.hpp>

static constexpr const std::string_view base_url = "https://bvmf.bmfbovespa.com.br/InstDados/SerHist/COTAHIST_";

enum class Type
{
    DAY,
    MONTH,
    YEAR,
};

std::size_t date_to_string(const std::tm &date, Type type, char buffer[8])
{
    switch (type)
    {
        case Type::DAY:
            return std::strftime(buffer, 8, "%d%m%Y", &date);
        case Type::MONTH:
            return std::strftime(buffer, 8, "%m%Y", &date);
        case Type::YEAR:
            return std::strftime(buffer, 8, "%Y", &date);
    }
    std::unreachable();
}

char type_to_char(Type type)
{
    switch (type)
    {
    case Type::DAY:
        return 'D';
    case Type::MONTH:
        return 'M';
    case Type::YEAR:
        return 'A';
    }
    std::unreachable();
}

std::string get_url(const std::tm &date, Type type)
{
    char buffer[8];
    date_to_string(date, type, buffer);
    return std::string(base_url) + type_to_char(type) + buffer + ".ZIP";
}

void download_quotes(std::counting_semaphore<>* semaphore, const std::tm date, const Type type, std::chrono::nanoseconds& total_time, std::atomic<int>& count)
{
    semaphore->acquire();
    std::string url = get_url(date, type);
    zip_archive zip = download_zip(url);
    CotBovespa cotacao;
    std::chrono::time_point start = std::chrono::high_resolution_clock::now();
    while (next_quote(zip, cotacao) != LineType::END)
    {
        count++;
    }
    std::chrono::time_point end = std::chrono::high_resolution_clock::now();
    total_time += end - start;
    semaphore->release();
}

int main()
{
    std::tm date = {};
    date.tm_mday = 1;
    date.tm_mon = 0;
    date.tm_year = 120;
    std::counting_semaphore<> semaphore(12);
    std::jthread threads[24];
    std::chrono::nanoseconds total_time = std::chrono::nanoseconds::zero();
    std::atomic<int> count = 0;
    for (int i = 0; i < 24; i++)
    {
        date.tm_mon = i % 12;
        threads[i] = std::jthread(download_quotes, &semaphore, date, Type::MONTH, std::ref(total_time), std::ref(count));
    }
    for (int i = 0; i < 24; i++)
    {
        threads[i].join();
    }
    spdlog::info("Downloaded {} quotes in {} ms", count.load(), std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count());
    return 0;
}