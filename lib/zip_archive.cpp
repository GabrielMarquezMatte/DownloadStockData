#include "../include/zip_archive.hpp"
#include <iostream>
#include <memory>

zip_archive::zip_archive(const std::string_view& content_buffer)
{
    this->buffer = std::make_unique<char[]>(content_buffer.size());
    std::copy(content_buffer.begin(), content_buffer.end(), buffer.get());
    zip_source = zip_source_buffer_create(buffer.get(), content_buffer.size(), 0, nullptr);
    if (zip_source == nullptr)
    {
        throw std::runtime_error("Failed to create zip source buffer");
    }
    zip_file = zip_open_from_source(zip_source, 0, nullptr);
    if (zip_file == nullptr)
    {
        zip_source_free(zip_source);
        throw std::runtime_error("Failed to open zip file");
    }
    inner_file = zip_fopen_index(zip_file, 0, 0);
    if (inner_file == nullptr)
    {
        zip_close(zip_file);
        zip_source_free(zip_source);
        throw std::runtime_error("Failed to open inner file");
    }
}

zip_archive::~zip_archive()
{
    if(inner_file != nullptr)
    {
        zip_fclose(inner_file);
    }
    zip_close(zip_file);
    if(file_buffer != nullptr)
    {
        delete[] file_buffer;
    }
}

bool zip_archive::next_line(char line[247])
{
    if(file_buffer == nullptr)
    {
        zip_stat_t stat;
        zip_stat_index(zip_file, 0, 0, &stat);
        file_buffer_size = stat.size;
        file_buffer = new char[stat.size];
        if(file_buffer == nullptr)
        {
            return false;
        }
        if(zip_fread(inner_file, file_buffer, stat.size) == -1)
        {
            return false;
        }
    }
    if(file_buffer_index + 247 > file_buffer_size)
    {
        return false;
    }
    std::copy(file_buffer + file_buffer_index, file_buffer + file_buffer_index + 247, line);
    file_buffer_index += 247;
    return true;
}
