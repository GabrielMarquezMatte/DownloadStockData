#pragma once
#include "formatting_download.hpp"
std::size_t parse_tm(const std::tm& tm, char output[11])
{
    if(tm.tm_year < -1900 || tm.tm_year > 8099)
    {
        return 0;
    }
    return std::strftime(output, 11, "%Y-%m-%d", &tm);
}

std::size_t parse_int(int value, char output[10])
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

template<std::size_t DoublePrecision>
std::size_t parse_double(double value, char output[DoublePrecision])
{
    if(value < 0)
    {
        output[0] = '0';
        return 1;
    }
    std::size_t size = 0;
    int integer = static_cast<int>(value);
    double fraction = value - integer;
    size += parse_int(integer, output);
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

std::size_t parse_csv_line(const CotBovespa& quote, char output[212])
{
    std::size_t size = 0;
    size += parse_tm(quote.dt_pregao, output + size);
    output[size++] = ';';
    size += parse_int(quote.cd_codbdi, output + size);
    output[size++] = ';';
    for(int i = 0; i < 12; i++)
    {
        if(quote.cd_codneg[i] == ' ')
        {
            break;
        }
        output[size++] = quote.cd_codneg[i];
    }
    output[size++] = ';';
    for(int i = 0; i < 12; i++)
    {
        if(quote.cd_codisin[i] == ' ')
        {
            break;
        }
        output[size++] = quote.cd_codisin[i];
    }
    output[size++] = ';';
    for(int i = 0; i < 10; i++)
    {
        if(quote.nm_speci[i] == ' ')
        {
            break;
        }
        output[size++] = quote.nm_speci[i];
    }
    output[size++] = ';';
    size += parse_int(quote.cd_tpmerc, output + size);
    output[size++] = ';';
    size += parse_int(quote.prz_termo, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_aber, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_max, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_min, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_med, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_fec, output + size);
    output[size++] = ';';
    size += parse_double<3>(quote.prec_exer, output + size);
    output[size++] = ';';
    size += parse_tm(quote.dt_datven, output + size);
    output[size++] = ';';
    size += parse_int(quote.fat_cot, output + size);
    output[size++] = ';';
    size += parse_int(quote.nr_dismes, output + size);
    output[size++] = '\n';
    return size;
}