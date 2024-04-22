find_package(CURL CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

file(GLOB_RECURSE SOURCES_LIB "*.cpp")

add_library(download_lib ${SOURCES_LIB})
target_link_libraries(download_lib PRIVATE libzip::zip CURL::libcurl libpqxx::pqxx Boost::system)