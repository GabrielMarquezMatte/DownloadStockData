#include "../include/zip_archive.hpp"
#include <iostream>

zip_archive::zip_archive(const std::string& content_buffer)
{
    zip_source = zip_source_buffer_create(content_buffer.c_str(), content_buffer.size(), 0, nullptr);
    if (zip_source == nullptr)
    {
        throw std::runtime_error("Failed to create zip source buffer");
    }
    zip_file = zip_open_from_source(zip_source, 0, nullptr);
}

zip_archive::~zip_archive()
{
    zip_close(zip_file);
}

std::vector<std::string> zip_archive::get_filenames()
{
    std::vector<std::string> filenames;
    zip_int64_t num_entries = zip_get_num_entries(zip_file, ZIP_FL_UNCHANGED);
    for (zip_int64_t i = 0; i < num_entries; i++)
    {
        const char *filename = zip_get_name(zip_file, i, ZIP_FL_ENC_GUESS);
        if (filename == nullptr)
        {
            throw std::runtime_error("Failed to get filename");
        }
        filenames.push_back(filename);
    }
    return filenames;
}

std::string zip_archive::get_file_content(const std::string& filename)
{
    std::string content;
    zip_stat_t stat;
    if (zip_stat(zip_file, filename.c_str(), ZIP_FL_ENC_GUESS, &stat) != 0)
    {
        throw std::runtime_error("Failed to get file stat");
    }
    zip_file_t *file = zip_fopen(zip_file, filename.c_str(), ZIP_FL_ENC_GUESS);
    if (file == nullptr)
    {
        throw std::runtime_error("Failed to open file");
    }
    std::unique_ptr<char[]> buffer(new char[stat.size]);
    if (zip_fread(file, buffer.get(), stat.size) < 0)
    {
        zip_fclose(file);
        zip_close(zip_file);
        zip_source_free(zip_source);
        throw std::runtime_error("Failed to read file");
    }
    content = std::string(buffer.get(), stat.size);
    zip_fclose(file);
    return content;
}