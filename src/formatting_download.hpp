#pragma once
#ifndef FORMATTING_DOWNLOAD_HPP
#define FORMATTING_DOWNLOAD_HPP
#include "../include/http_client.hpp"
#include "../include/zip_archive.hpp"
#include <concurrentqueue/concurrentqueue.h>
#include <ctime>
#include <sstream>
#include <iomanip>

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
    std::ostream& operator<<(std::ostream& os) const
    {
        os << to_string();
        return os;
    }
    std::string to_string() const
    {
        std::stringstream ss;
        ss << "CotBovespa: {"
           << "dt_pregao: " << std::put_time(&dt_pregao, "%Y-%m-%d")
           << ", cd_codbdi: " << cd_codbdi
           << ", cd_codneg: " << cd_codneg
           << ", cd_codisin: " << cd_codisin
           << ", nm_speci: " << nm_speci
           << ", cd_tpmerc: " << cd_tpmerc
           << ", prz_termo: " << prz_termo
           << ", prec_aber: " << prec_aber
           << ", prec_max: " << prec_max
           << ", prec_min: " << prec_min
           << ", prec_med: " << prec_med
           << ", prec_fec: " << prec_fec
           << ", prec_exer: " << prec_exer
           << ", dt_datven: " << std::put_time(&dt_datven, "%Y-%m-%d")
           << ", fat_cot: " << fat_cot
           << ", nr_dismes: " << nr_dismes
           << "}";
        return ss.str();
    }
};

enum class LineType
{
    START,
    QUOTE,
    END,
};

int read_quote_file(zip_archive& zip, moodycamel::ConcurrentQueue<CotBovespa>& queue, std::stop_token stop_token);
#endif