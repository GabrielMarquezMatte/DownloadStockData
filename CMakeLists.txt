cmake_minimum_required(VERSION 3.26.3)
project(DownloadStockData VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -pedantic")
endif()

include(${PROJECT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake)

include(CTest)
enable_testing()

find_package(libpqxx CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-concurrentqueue CONFIG REQUIRED)
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)
find_package(Drogon CONFIG REQUIRED)

add_subdirectory(${PROJECT_SOURCE_DIR}/lib)
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
add_subdirectory(${PROJECT_SOURCE_DIR}/controllers)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE libpqxx::pqxx)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE download_lib download_src)
target_link_libraries(${PROJECT_NAME} PRIVATE cxxopts::cxxopts)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PRIVATE unofficial::concurrentqueue::concurrentqueue)
target_link_libraries(${PROJECT_NAME} PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Parquet::parquet_static,Parquet::parquet_shared>")
target_link_libraries(${PROJECT_NAME} PRIVATE "$<IF:$<BOOL:${ARROW_BUILD_STATIC}>,Arrow::arrow_static,Arrow::arrow_shared>")

FILE(GLOB API_SOURCE_FILES api.cpp ${PROJECT_SOURCE_DIR}/controllers/*.cpp)

add_executable(StockDataApi api.cpp)
target_link_libraries(StockDataApi PRIVATE libpqxx::pqxx download_lib Drogon::Drogon stock_controllers)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
