#include "../include/date_operations.hpp"
int days_to_epoch(std::tm &date)
{
    std::tm epoch = {0, 0, 0, 1, 0, 70};
    std::time_t epoch_time = std::mktime(&epoch);
    std::time_t date_time = std::mktime(&date);
    return static_cast<int>((date_time - epoch_time) / (60 * 60 * 24));
}

int days_to_epoch(std::chrono::system_clock::time_point &date)
{
    return std::chrono::duration_cast<std::chrono::days>(date.time_since_epoch()).count();
}