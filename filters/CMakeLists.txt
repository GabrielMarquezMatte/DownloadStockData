find_package(Drogon CONFIG REQUIRED)
file(GLOB_RECURSE SOURCES_SRC "*.cpp")

add_library(stock_filters STATIC ${SOURCES_SRC})
target_link_libraries(stock_filters PRIVATE Drogon::Drogon)