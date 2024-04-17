#include "../include/zip_archive.hpp"
#include <iostream>
#include <memory>

inline void close_zip_archive(zip_t* zip_file, zip_file_t* inner_file, char* file_buffer)
{
    if(inner_file != nullptr)
    {
        zip_fclose(inner_file);
    }
    if(zip_file != nullptr)
    {
        zip_close(zip_file);
    }
    if(file_buffer != nullptr)
    {
        delete[] file_buffer;
    }
}

zip_archive::zip_archive(const std::string_view& content_buffer)
{
    this->buffer = std::make_unique<char[]>(content_buffer.size());
    std::memcpy(buffer.get(), content_buffer.data(), content_buffer.size());
    zip_source = zip_source_buffer_create(buffer.get(), content_buffer.size(), 0, nullptr);
    if (zip_source == nullptr)
    {
        throw std::runtime_error("Failed to create zip source buffer");
    }
    zip_file = zip_open_from_source(zip_source, 0, nullptr);
    if (zip_file == nullptr)
    {
        throw std::runtime_error("Failed to open zip file");
    }
    inner_file = zip_fopen_index(zip_file, 0, 0);
    if (inner_file == nullptr)
    {
        close_zip_archive(zip_file, inner_file, file_buffer);
        throw std::runtime_error("Failed to open inner file");
    }
    zip_stat_t stat;
    if(zip_stat_index(zip_file, 0, 0, &stat) == -1)
    {
        close_zip_archive(zip_file, inner_file, file_buffer);
        throw std::runtime_error("Failed to get file stat");
    }
    file_buffer_size = stat.size;
    file_buffer = new char[stat.size];
    if(file_buffer == nullptr)
    {
        close_zip_archive(zip_file, inner_file, file_buffer);
        throw std::runtime_error("Failed to allocate file buffer");
    }
    if(zip_fread(inner_file, file_buffer, stat.size) == -1)
    {
        close_zip_archive(zip_file, inner_file, file_buffer);
        throw std::runtime_error("Failed to read file");
    }
    file_buffer_index = 0;
}

zip_archive::~zip_archive()
{
    close_zip_archive(zip_file, inner_file, file_buffer);
}

bool zip_archive::next_line(char line[247])
{
    if(file_buffer_index + 247 > file_buffer_size)
    {
        return false;
    }
    std::memcpy(line, file_buffer + file_buffer_index, 247);
    file_buffer_index += 247;
    return true;
}
