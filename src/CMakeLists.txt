find_package(CURL CONFIG REQUIRED)

add_library(formatting_download STATIC formatting_download.cpp formatting_download.hpp)
target_link_libraries(formatting_download PRIVATE http_client zip_archive CURL::libcurl)
