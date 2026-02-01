cmake_minimum_required(VERSION 3.10)
project(vsnap VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
if (WIN32)
    target_link_libraries(${PROJECT_NAME} bcrypt)
    if (MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _HAS_STD_BYTE=0)
    endif()
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
endif()