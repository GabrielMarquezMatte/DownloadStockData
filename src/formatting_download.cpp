#include "formatting_download.hpp"
#include <string>

std::tm parse_date(const std::string_view& date)
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

int parse_int(const std::string_view& str)
{
    int value = 0;
    for (size_t i = 0; i < str.size(); i++)
    {
        if(str[i] >= '0' && str[i] <= '9') value = value * 10 + (str[i] - '0');
    }
    return value;
}

double parse_double(const std::string_view& str)
{
    uint64_t result = (str[0] - '0') * 1'000'000'000'000LL +
                      (str[1] - '0') * 100'000'000'000LL +
                      (str[2] - '0') * 10'000'000'000LL +
                      (str[3] - '0') * 1'000'000'000LL +
                      (str[4] - '0') * 100'000'000LL +
                      (str[5] - '0') * 10'000'000LL +
                      (str[6] - '0') * 1'000'000LL +
                      (str[7] - '0') * 100'000LL +
                      (str[8] - '0') * 10'000LL +
                      (str[9] - '0') * 1'000LL +
                      (str[10] - '0') * 100LL +
                      (str[11] - '0') * 10LL +
                      (str[12] - '0');
    return result * 0.01;
}

CotBovespa parse_line(const std::string_view &line)
{
    CotBovespa cotacao;
    cotacao.dt_pregao = parse_date(line.substr(2, 8));
    cotacao.cd_codbdi = parse_int(line.substr(10, 2));
    std::string_view codneg = line.substr(12, 12);
    std::copy(codneg.begin(), codneg.end(), cotacao.cd_codneg);
    cotacao.cd_tpmerc = parse_int(line.substr(24, 3));
    std::string_view nm_speci = line.substr(39, 10);
    std::copy(nm_speci.begin(), nm_speci.end(), cotacao.nm_speci);
    cotacao.prz_termo = parse_int(line.substr(49, 3));
    cotacao.prec_aber = parse_double(line.substr(56, 13));
    cotacao.prec_max = parse_double(line.substr(69, 13));
    cotacao.prec_min = parse_double(line.substr(82, 13));
    cotacao.prec_med = parse_double(line.substr(95, 13));
    cotacao.prec_fec = parse_double(line.substr(108, 13));
    cotacao.prec_exer = parse_double(line.substr(188, 13));
    cotacao.dt_datven = parse_date(line.substr(202, 8));
    cotacao.fat_cot = parse_int(line.substr(210, 7));
    std::string_view codisin = line.substr(230, 12);
    std::copy(codisin.begin(), codisin.end(), cotacao.cd_codisin);
    cotacao.nr_dismes = parse_int(line.substr(242, 3));
    return cotacao;
}

LineType check_line(const std::string_view& line)
{
    std::string_view start = line.substr(0, 3);
    if(start == "00C") return LineType::START;
    if(start == "99C") return LineType::END;
    return LineType::QUOTE;
}

zip_archive download_zip(const std::string_view &url)
{
    std::string content = http_client::get(url.data());
    return zip_archive(content);
}

void read_lines(zip_archive &zip, moodycamel::ReaderWriterQueue<CotBovespa> &queue)
{
    char line[247];
    while(zip.next_line(line))
    {
        LineType type = check_line(line);
        switch(type)
        {
            case LineType::END:
                return;
            case LineType::START:
                continue;
            case LineType::QUOTE:
                CotBovespa cotacao = parse_line(line);
                queue.enqueue(cotacao);
                break;
        }
    }
}

LineType next_quote(zip_archive &zip, CotBovespa &cotacao)
{
    char line[247];
    while(zip.next_line(line))
    {
        LineType type = check_line(line);
        if(type != LineType::QUOTE) return type;
        cotacao = parse_line(line);
        return type;
    }
    return LineType::END;
}