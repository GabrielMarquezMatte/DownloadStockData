#include "formatting_download.hpp"
#include "../include/zip_archive.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#define white_space(c) ((c) == ' ' || (c) == '\t')
#define valid_digit(c) ((c) >= '0' && (c) <= '9')

std::tm parse_date(const std::string &date, const std::string &format = "%Y%m%d")
{
    std::tm tm = {};
    std::stringstream ss(date);
    ss >> std::get_time(&tm, format.c_str());
    return tm;
}

void remove_spaces(std::string &str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

int fast_atoi(const char *p) {
    int x = 0;
    bool neg = false;
    if (*p == '-') {
        neg = true;
        ++p;
    }
    while (*p >= '0' && *p <= '9') {
        x = (x*10) + (*p - '0');
        ++p;
    }
    if (neg) {
        x = -x;
    }
    return x;
}

double fast_atof(const char *p)
{
    int frac;
    double sign, value, scale;
    while (white_space(*p) ) {
        p += 1;
    }
    sign = 1.0;
    if (*p == '-') {
        sign = -1.0;
        p += 1;

    } else if (*p == '+') {
        p += 1;
    }
    for (value = 0.0; valid_digit(*p); p += 1) {
        value = value * 10.0 + (*p - '0');
    }
    if (*p == '.') {
        double pow10 = 10.0;
        p += 1;
        while (valid_digit(*p)) {
            value += (*p - '0') / pow10;
            pow10 *= 10.0;
            p += 1;
        }
    }
    frac = 0;
    scale = 1.0;
    if ((*p == 'e') || (*p == 'E')) {
        unsigned int expon;
        p += 1;
        if (*p == '-') {
            frac = 1;
            p += 1;

        } else if (*p == '+') {
            p += 1;
        }
        for (expon = 0; valid_digit(*p); p += 1) {
            expon = expon * 10 + (*p - '0');
        }
        if (expon > 308) expon = 308;
        while (expon >= 50) { scale *= 1E50; expon -= 50; }
        while (expon >=  8) { scale *= 1E8;  expon -=  8; }
        while (expon >   0) { scale *= 10.0; expon -=  1; }
    }
    return sign * (frac ? (value / scale) : (value * scale));
}

int parse_int(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    return str.empty() ? -1 : fast_atoi(str.c_str());
}

CotBovespa parse_line(const std::string &line)
{
    CotBovespa cotacao;
    cotacao.dt_pregao = parse_date(line.substr(2, 8).c_str());
    cotacao.cd_codbdi = fast_atoi(line.substr(10, 2).c_str());
    cotacao.cd_codneg = line.substr(12, 12);
    remove_spaces(cotacao.cd_codneg);
    cotacao.cd_tpmerc = fast_atoi(line.substr(24, 3).c_str());
    cotacao.nm_speci = line.substr(39, 10);
    cotacao.prz_termo = parse_int(line.substr(49, 3).c_str());
    cotacao.prec_aber = fast_atof(line.substr(56, 13).c_str()) / 100;
    cotacao.prec_max = fast_atof(line.substr(69, 13).c_str()) / 100;
    cotacao.prec_min = fast_atof(line.substr(82, 13).c_str()) / 100;
    cotacao.prec_med = fast_atof(line.substr(95, 13).c_str()) / 100;
    cotacao.prec_fec = fast_atof(line.substr(108, 13).c_str()) / 100;
    cotacao.prec_exer = fast_atof(line.substr(188, 13).c_str()) / 100;
    cotacao.dt_datven = parse_date(line.substr(202, 8).c_str());
    cotacao.fat_cot = fast_atoi(line.substr(210, 7).c_str());
    cotacao.cd_codisin = line.substr(230, 12);
    remove_spaces(cotacao.cd_codisin);
    cotacao.nr_dismes = fast_atoi(line.substr(242, 3).c_str());
    return cotacao;
}

bool check_line(const std::string& line)
{
    if(line.size() < 245)
    {
        return false;
    }
    bool valid = false;
    for(int i = 42;i < 60; i++)
    {
        if(line[i] != ' ')
        {
            valid = true;
            break;
        }
    }
    return valid;
}

std::vector<CotBovespa> download_and_parse(http_client &client, const std::string &url)
{
    std::vector<CotBovespa> cotacoes;
    std::string content = client.get(url);
    if(content.empty())
    {
        return cotacoes;
    }
    zip_archive zip(content);
    auto filenames = zip.get_filenames();
    if(filenames.empty())
    {
        return cotacoes;
    }
    std::string filename = filenames[0];
    std::string file_content = zip.get_file_content(filename);
    std::stringstream ss(file_content);
    std::string line;
    std::getline(ss, line);
    while (std::getline(ss, line))
    {
        if(!check_line(line))
        {
            continue;
        }
        cotacoes.push_back(parse_line(line));
    }
    return cotacoes;
}
