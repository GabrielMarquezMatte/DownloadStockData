find_package(CURL CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)

add_library(zip_archive zip_archive.cpp ${PROJECT_SOURCE_DIR}/include/zip_archive.hpp)
target_link_libraries(zip_archive PRIVATE libzip::zip)

add_library(http_client http_client.cpp ${PROJECT_SOURCE_DIR}/include/http_client.hpp)
target_link_libraries(http_client PRIVATE CURL::libcurl)

add_library(connection_pool connection_pool.cpp ${PROJECT_SOURCE_DIR}/include/connection_pool.hpp)
target_link_libraries(connection_pool PRIVATE libpqxx::pqxx)