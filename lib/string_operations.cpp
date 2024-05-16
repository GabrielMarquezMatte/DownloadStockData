#include "../include/string_operations.hpp"
#include <xmmintrin.h>
#include <immintrin.h>
std::tm string_to_tm(const std::string_view &date)
{
    std::tm tm;
    tm.tm_year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0') - 1900;
    tm.tm_mon = (date[4] - '0') * 10 + (date[5] - '0') - 1;
    tm.tm_mday = (date[6] - '0') * 10 + (date[7] - '0');
    tm.tm_hour = 0;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return tm;
}
std::chrono::system_clock::time_point string_to_time_point(const std::string_view &date)
{
    std::tm date_tm = string_to_tm(date);
    return std::chrono::system_clock::from_time_t(std::mktime(&date_tm));
}
static constexpr const unsigned long long multipliers[13] = {1'000'000'000'000LL, 100'000'000'000LL, 10'000'000'000LL, 1'000'000'000LL, 100'000'000LL, 10'000'000LL, 1'000'000LL, 100'000LL, 10'000LL, 1'000LL, 100LL, 10LL, 1};
double string_to_double(const std::string_view &str)
{
    unsigned long long result = 0;
    for (int i = 0; i < 13; ++i)
    {
        result += (str[i] - '0') * multipliers[i];
    }
    return result * 0.01;
}
void copyBytesAligned(char *dest, const char *src)
{
    __m128 data = _mm_loadu_ps(reinterpret_cast<const float *>(src));
    _mm_store_ps(reinterpret_cast<float *>(dest), data);
}
std::size_t tm_to_string(const std::tm& tm, char output[11])
{
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon + 1;
    int day = tm.tm_mday;
    output[0] = '0' + static_cast<char>(year / 1000);
    output[1] = '0' + static_cast<char>((year % 1000) / 100);
    output[2] = '0' + static_cast<char>((year % 100) / 10);
    output[3] = '0' + static_cast<char>(year % 10);
    output[4] = '-';
    output[5] = '0' + static_cast<char>(month / 10);
    output[6] = '0' + static_cast<char>(month % 10);
    output[7] = '-';
    output[8] = '0' + static_cast<char>(day / 10);
    output[9] = '0' + static_cast<char>(day % 10);
    output[10] = '\0';
    return 10;
}
std::size_t int_to_string(int value, char output[10])
{
    if(value < 0)
    {
        return 0;
    }
    if(value == 0)
    {
        output[0] = '0';
        return 1;
    }
    char* p = output;
    while(value > 0)
    {
        *p++ = (value % 10) + '0';
        value /= 10;
    }
    *p = '\0';
    char* start = output;
    char* end = p - 1;
    while(start < end)
    {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
    return p - output;
}