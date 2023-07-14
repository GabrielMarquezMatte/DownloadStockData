#pragma once
#include "../include/http_client.hpp"
#include <ctime>

typedef struct CotBovespa_t
{
    std::tm dt_pregao;
    int cd_codbdi;
    std::string cd_codneg;
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
    std::string cd_codisin;
    int nr_dismes;
} CotBovespa;

std::vector<CotBovespa> download_and_parse(http_client &client, const std::string &url);