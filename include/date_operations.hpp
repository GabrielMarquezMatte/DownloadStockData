#pragma once
#include <chrono>
#include <ctime>
int days_to_epoch(std::tm &date);
int days_to_epoch(std::chrono::system_clock::time_point &date);