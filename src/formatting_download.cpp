#include "formatting_download.hpp"
#include "../include/zip_archive.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

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

int parse_int(std::string str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
    return str.empty() ? -1 : std::stoi(str);
}

CotBovespa parse_line(const std::string &line)
{
    CotBovespa cotacao;
    cotacao.dt_pregao = parse_date(line.substr(2, 8));
    cotacao.cd_codbdi = std::stoi(line.substr(10, 2));
    cotacao.cd_codneg = line.substr(12, 12);
    remove_spaces(cotacao.cd_codneg);
    cotacao.cd_tpmerc = std::stoi(line.substr(24, 3));
    cotacao.prz_termo = parse_int(line.substr(49, 3));
    cotacao.prec_aber = std::stod(line.substr(56, 13)) / 100;
    cotacao.prec_max = std::stod(line.substr(69, 13)) / 100;
    cotacao.prec_min = std::stod(line.substr(82, 13)) / 100;
    cotacao.prec_med = std::stod(line.substr(95, 13)) / 100;
    cotacao.prec_fec = std::stod(line.substr(108, 13)) / 100;
    cotacao.prec_exer = std::stod(line.substr(188, 13)) / 100;
    cotacao.dt_datven = parse_date(line.substr(202, 8));
    cotacao.fat_cot = std::stoi(line.substr(210, 7));
    cotacao.cd_codisin = line.substr(230, 12);
    remove_spaces(cotacao.cd_codisin);
    cotacao.nr_dismes = std::stoi(line.substr(242, 3));
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
