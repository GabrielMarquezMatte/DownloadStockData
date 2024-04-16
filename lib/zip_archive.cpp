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
}

bool zip_archive::next_line(char line[247])
{
    if (zip_fread(inner_file, line, 247) == -1)
    {
        zip_fclose(inner_file);
        inner_file = nullptr;
        return false;
    }
    return true;
}
