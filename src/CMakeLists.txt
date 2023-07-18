find_package(CURL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_library(formatting_download STATIC formatting_download.cpp formatting_download.hpp)
target_link_libraries(formatting_download PRIVATE http_client zip_archive CURL::libcurl)

add_library(insert_db insert_db.cpp insert_db.hpp)
target_link_libraries(insert_db PRIVATE libpqxx::pqxx)