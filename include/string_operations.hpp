#pragma once
#ifndef STRING_OPERATIONS_HPP
#define STRING_OPERATIONS_HPP
#include <string>
#include <ctime>
#include <chrono>
std::tm string_to_tm(const std::string_view &date);
std::chrono::system_clock::time_point string_to_time_point(const std::string_view &date);
template <std::size_t N>
int string_to_int(const std::string_view &str)
{
    static_assert(N <= 7, "parse_int: N must be less than or equal to 7");
    int result = 0;
    for (int i = 0; i < N; ++i)
    {
        result = result * 10 + (str[i] - '0');
    }
    return result;
}
double string_to_double(const std::string_view &str);
void copyBytesAligned(char *dest, const char *src);
std::size_t tm_to_string(const std::tm& tm, char output[11]);
std::size_t int_to_string(int value, char output[10]);
template<std::size_t DoublePrecision>
std::size_t double_to_string(double value, char output[DoublePrecision + 10])
{
    if(value < 0)
    {
        output[0] = '0';
        return 1;
    }
    std::size_t size = 0;
    int integer = static_cast<int>(value);
    double fraction = value - integer;
    size += int_to_string(integer, output);
    output[size++] = '.';
    for(int i = 0; i < DoublePrecision - 1; i++)
    {
        fraction *= 10;
        char digit = static_cast<char>(fraction);
        output[size++] = '0' + digit;
        fraction -= digit;
    }
    return size;
}
#endif