find_package(CURL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_path(READERWRITERQUEUE_INCLUDE_DIRS "readerwriterqueue/atomicops.h")


file(GLOB_RECURSE SOURCES_SRC "*.cpp")

add_library(download_src STATIC ${SOURCES_SRC})
target_link_libraries(download_src PRIVATE download_lib CURL::libcurl libpqxx::pqxx)
target_include_directories(download_src PRIVATE ${READERWRITERQUEUE_INCLUDE_DIRS})