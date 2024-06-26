#pragma once
#include <zip.h>
#include <string>
#include <memory>

class zip_archive
{
public:
    zip_archive(const std::string_view& content_buffer);
    ~zip_archive();
    bool next_line(char line[247]);
private:
    zip_t* zip_file;
    zip_source_t* zip_source;
    zip_file_t* inner_file;
    char* file_buffer = nullptr;
    size_t file_buffer_index = 0;
    size_t file_buffer_size = 0;
};