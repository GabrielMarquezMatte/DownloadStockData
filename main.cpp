#include <http_client.hpp>
#include <zip_archive.hpp>
#include <connection_pool.hpp>
#include <formatting_download.hpp>
#include <insert_db.hpp>
#include <thread_pool.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <pqxx/pqxx>
#include <cxxopts.hpp>

static const std::string base_url = "https://bvmf.bmfbovespa.com.br/InstDados/SerHist/COTAHIST_";
static std::mutex mtx;

bool operator<(const std::tm &lhs, const std::tm &rhs)
{
    if (lhs.tm_year < rhs.tm_year)
        return true;
    if (lhs.tm_year > rhs.tm_year)
        return false;
    if (lhs.tm_mon < rhs.tm_mon)
        return true;
    if (lhs.tm_mon > rhs.tm_mon)
        return false;
    if (lhs.tm_mday < rhs.tm_mday)
        return true;
    if (lhs.tm_mday > rhs.tm_mday)
        return false;
    return false;
}

bool operator==(const std::tm &lhs, const std::tm &rhs)
{
    return lhs.tm_year == rhs.tm_year && lhs.tm_mon == rhs.tm_mon && lhs.tm_mday == rhs.tm_mday;
}

bool operator<=(const std::tm &lhs, const std::tm &rhs)
{
    return lhs < rhs || lhs == rhs;
}

std::string date_to_string(const std::tm &date, const std::string &format)
{
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), format.c_str(), &date);
    return std::string(buffer);
}

std::string get_url(const std::tm &date, const bool annual)
{
    std::string url = base_url;
    url += annual ? "A" : "D";
    url += annual ? date_to_string(date, "%Y") : date_to_string(date, "%d%m%Y");
    url += ".ZIP";
    return url;
}

std::tm get_current_date()
{
    std::time_t now = std::time(nullptr);
    std::tm date;
#ifdef _WIN32
    localtime_s(&date, &now);
#else
    localtime_r(&now, &date);
#endif
    return date;
}

std::tm get_date(const std::string &date, const std::string &format = "%Y-%m-%d")
{
    std::tm date_tm;
    std::istringstream ss(date);
    ss >> std::get_time(&date_tm, format.c_str());
    date_tm.tm_hour = 0;
    date_tm.tm_min = 0;
    date_tm.tm_sec = 0;
    date_tm.tm_isdst = -1;
    date_tm.tm_wday = 0;
    date_tm.tm_yday = 0;
    return date_tm;
}

std::tm max_date_table(connection_pool &pool)
{
    auto connection = pool.get_connection();
    pqxx::work work(*connection);
    pqxx::result result = work.exec("SELECT MAX(dt_pregao)+1 FROM tcot_bovespa");
    auto res1 = result[0];
    auto res2 = res1[0];
    if (res2.is_null())
    {
        return get_date("2000-01-01");
    }
    auto value = res2.as<std::string>();
    if (value.empty())
    {
        return get_date("2000-01-01");
    }
    std::tm date = get_date(value);
    return date;
}

void execute_for_date(connection_pool &pool, http_client &client, const std::tm &date, const bool annual = false, const bool verbose = false)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto date_str = date_to_string(date, "%d/%m/%Y");
    std::string url = get_url(date, annual);
    if(verbose)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = get_current_date();
        std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [EXECUTE_FOR_DATE] - Downloading data for " << date_str << "\n";
    }
    auto cotacoes = download_and_parse(client, url);
    if(verbose)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = get_current_date();
        std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [EXECUTE_FOR_DATE] - Retrieved " << cotacoes.size() << " lines for " << date_str << "\n";
    }
    insert_into_table(pool, cotacoes);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if(verbose)
    {
        std::unique_lock<std::mutex> lock(mtx);
        auto now = get_current_date();
        std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [EXECUTE_FOR_DATE] - Done for " << date_to_string(date, "%d/%m/%Y") << " in " << elapsed.count() << " ms\n";
    }
}

void run_for_dates(const std::string &dates, connection_pool &pool, http_client &client, thread_pool& threads, bool annual = false, bool verbose = false)
{
    std::istringstream ss(dates);
    std::string date;
    while (std::getline(ss, date, ','))
    {
        std::tm date_tm = get_date(date);
        threads.enqueue(execute_for_date, std::ref(pool), std::ref(client), date_tm, annual, verbose);
    }
}

void run_for_range(const std::string &start_date, const std::string &end_date, connection_pool &pool, http_client &client, thread_pool& threads, bool verbose = false)
{
    std::tm start = get_date(start_date);
    std::tm end = get_date(end_date);
    std::tm current = start;
    while (current <= end)
    {
        threads.enqueue(execute_for_date, std::ref(pool), std::ref(client), current, false, verbose);
        current.tm_mday++;
        std::mktime(&current);
    }
}

int main(int argc, char **argv, char **envp)
{
    bool annual = false;
    bool verbose = false;
    int num_threads = std::thread::hardware_concurrency();
    std::string connection_string = "dbname=testdb user=postgres password=Dom,080203 hostaddr=127.0.0.1 port=5432";
    cxxopts::Options options("Bovespa downloader", "Download Bovespa data");
    options.add_options()
    ("a,annual", "Download annual data", cxxopts::value<bool>(annual))
    ("v,verbose", "Verbose output", cxxopts::value<bool>(verbose))
    ("d,dates", "Download data for specific dates", cxxopts::value<std::string>())
    ("s,start-date", "Start date for download", cxxopts::value<std::string>())
    ("e,end-date", "End date for download", cxxopts::value<std::string>())
    ("c,connection", "Connection string", cxxopts::value<std::string>(connection_string))
    ("t,threads", "Number of threads", cxxopts::value<int>(num_threads)->default_value(std::to_string(num_threads)))
    ("h,help", "Print usage");
    auto result = options.parse(argc, argv);
    if (result.count("help"))
    {
        std::cout << options.help() << "\n";
        return 0;
    }
    connection_pool pool(connection_string, num_threads);
    http_client client;
    thread_pool threads(num_threads);
    if (result.count("dates"))
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::string dates = result["dates"].as<std::string>();
        run_for_dates(dates, pool, client, threads, annual, verbose);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (verbose)
        {
            std::unique_lock<std::mutex> lock(mtx);
            auto now = get_current_date();
            std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [MAIN] - Done in " << elapsed.count() << " ms\n";
        }
        return 0;
    }
    if (result.count("start-date") && result.count("end-date"))
    {
        auto start = std::chrono::high_resolution_clock::now();
        std::string start_date = result["start-date"].as<std::string>();
        std::string end_date = result["end-date"].as<std::string>();
        run_for_range(start_date, end_date, pool, client, threads, verbose);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        if (verbose)
        {
            std::unique_lock<std::mutex> lock(mtx);
            auto now = get_current_date();
            std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [MAIN] - Done in " << elapsed.count() << " ms\n";
        }
        return 0;
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::tm date = max_date_table(pool);
    std::tm current_date = get_current_date();
    if (verbose)
    {
        auto now = get_current_date();
        {
            std::unique_lock<std::mutex> lock(mtx);
            std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [MAIN] - Max date in table: " << date_to_string(date, "%d/%m/%Y") << "\n";
            std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [MAIN] - Current date: " << date_to_string(current_date, "%d/%m/%Y") << "\n";
        }
    }
    run_for_range(date_to_string(date, "%Y-%m-%d"), date_to_string(current_date, "%Y-%m-%d"), pool, client, threads, verbose);
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    if (verbose)
    {
        auto now = get_current_date();
        {
            std::unique_lock<std::mutex> lock(mtx);
            std::cout << date_to_string(now,"%Y-%m-%d %H:%M:%S") << " - [MAIN] - Done in " << elapsed.count() << " ms\n";
        }
    }
    return 0;
}