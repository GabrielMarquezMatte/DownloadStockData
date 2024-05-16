find_package(CURL CONFIG REQUIRED)
find_package(libpqxx CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES_SRC "*.cpp")

add_library(download_src STATIC ${SOURCES_SRC})
target_link_libraries(download_src PRIVATE download_lib CURL::libcurl libpqxx::pqxx)
target_link_libraries(download_src PRIVATE unofficial::concurrentqueue::concurrentqueue)
target_link_libraries(download_src PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Parquet::parquet_static,Parquet::parquet_shared>")
target_link_libraries(download_src PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Arrow::arrow_static,Arrow::arrow_shared>")