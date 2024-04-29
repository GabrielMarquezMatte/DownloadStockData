#pragma once
#ifndef PARSE_DATA_HPP
#define PARSE_DATA_HPP
#include "formatting_download.hpp"
#include "../include/string_operations.hpp"

std::size_t parse_csv_line(const CotBovespa& quote, char output[212])
{
    std::size_t size = 0;
    size += tm_to_string(quote.dt_pregao, output + size);
    output[size++] = ';';
    size += int_to_string(quote.cd_codbdi, output + size);
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
        output[size++] = quote.nm_speci[i];
    }
    output[size++] = ';';
    size += int_to_string(quote.cd_tpmerc, output + size);
    output[size++] = ';';
    size += int_to_string(quote.prz_termo, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_aber, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_max, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_min, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_med, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_fec, output + size);
    output[size++] = ';';
    size += double_to_string<3>(quote.prec_exer, output + size);
    output[size++] = ';';
    size += tm_to_string(quote.dt_datven, output + size);
    output[size++] = ';';
    size += int_to_string(quote.fat_cot, output + size);
    output[size++] = ';';
    size += int_to_string(quote.nr_dismes, output + size);
    output[size++] = '\n';
    return size;
}
#endif