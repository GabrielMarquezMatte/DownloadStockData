find_package(Drogon CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
file(GLOB_RECURSE SOURCES_SRC "*.cpp")

add_library(stock_controllers STATIC ${SOURCES_SRC})
target_link_libraries(stock_controllers PRIVATE Drogon::Drogon libpqxx::pqxx download_lib)