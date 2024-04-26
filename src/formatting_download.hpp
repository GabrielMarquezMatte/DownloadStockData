#pragma once
#include "../include/http_client.hpp"
#include "../include/zip_archive.hpp"
#include <concurrentqueue/concurrentqueue.h>
#include <ctime>

struct CotBovespa
{
    std::tm dt_pregao;
    int cd_codbdi;
    char cd_codneg[16];
    char cd_codisin[16];
    char nm_speci[16];
    int cd_tpmerc;
    int prz_termo = -1;
    double prec_aber;
    double prec_max;
    double prec_min;
    double prec_med;
    double prec_fec;
    double prec_exer;
    std::tm dt_datven;
    int fat_cot;
    int nr_dismes;
};

enum class LineType
{
    START,
    QUOTE,
    END,
};

zip_archive download_zip(const std::string_view &url);
int read_quote_file(zip_archive& zip, moodycamel::ConcurrentQueue<CotBovespa>& queue);