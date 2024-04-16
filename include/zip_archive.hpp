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
    std::unique_ptr<char[]> buffer;
    zip_t* zip_file;
    zip_source_t* zip_source;
    zip_file_t* inner_file;
};