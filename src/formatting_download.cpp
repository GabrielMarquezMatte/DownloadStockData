#include "formatting_download.hpp"
#include "../include/string_operations.hpp"


CotBovespa parse_line(const std::string_view &line)
{
    CotBovespa cotacao;
    cotacao.dt_pregao = string_to_tm(line.substr(2, 8));
    cotacao.cd_codbdi = string_to_int<2>(line.substr(10, 2));
    std::string_view codneg = line.substr(12, 12);
    copyBytesAligned(cotacao.cd_codneg, codneg.data());
    cotacao.cd_tpmerc = string_to_int<3>(line.substr(24, 3));
    std::string_view nm_speci = line.substr(39, 10);
    copyBytesAligned(cotacao.nm_speci, nm_speci.data());
    cotacao.prz_termo = string_to_int<3>(line.substr(49, 3));
    cotacao.prec_aber = string_to_double(line.substr(56, 13));
    cotacao.prec_max = string_to_double(line.substr(69, 13));
    cotacao.prec_min = string_to_double(line.substr(82, 13));
    cotacao.prec_med = string_to_double(line.substr(95, 13));
    cotacao.prec_fec = string_to_double(line.substr(108, 13));
    cotacao.prec_exer = string_to_double(line.substr(188, 13));
    cotacao.dt_datven = string_to_tm(line.substr(202, 8));
    cotacao.fat_cot = string_to_int<7>(line.substr(210, 7));
    std::string_view codisin = line.substr(230, 12);
    copyBytesAligned(cotacao.cd_codisin, codisin.data());
    cotacao.nr_dismes = string_to_int<3>(line.substr(242, 3));
    return cotacao;
}

LineType check_line(const std::string_view &line)
{
    std::string_view start = line.substr(0, 3);
    if (start == "00C")
        return LineType::START;
    if (start == "99C")
        return LineType::END;
    return LineType::QUOTE;
}

int read_quote_file(zip_archive &zip, moodycamel::ConcurrentQueue<CotBovespa> &queue, std::stop_token stop_token)
{
    char line[247];
    int count = 0;
    while (zip.next_line(line) && !stop_token.stop_requested())
    {
        count++;
        LineType type = check_line(line);
        switch (type)
        {
        case LineType::END:
            return count;
        case LineType::START:
            continue;
        case LineType::QUOTE:
            CotBovespa cotacao = parse_line(line);
            queue.enqueue(cotacao);
            break;
        }
    }
    return count;
}
