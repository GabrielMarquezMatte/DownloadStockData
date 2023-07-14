#pragma once
#include <zip.h>
#include <string>
#include <vector>

class zip_archive
{
public:
    zip_archive(const std::string& content_buffer);
    ~zip_archive();
    std::vector<std::string> get_filenames();
    std::string get_file_content(const std::string& filename);
private:
    zip_t* zip_file;
    zip_source_t* zip_source;
};