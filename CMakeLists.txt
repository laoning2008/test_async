cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

project(test_async CXX)

CPMAddPackage(
    NAME asio
    GITHUB_REPOSITORY chriskohlhoff/asio
    GIT_TAG asio-1-24-0)
CPMAddPackage(
    NAME libunifex
    GITHUB_REPOSITORY facebookexperimental/libunifex
    GIT_TAG 591ec09e7d51858ad05be979d4034574215f5971)
CPMAddPackage(
    NAME async_simple
    GITHUB_REPOSITORY alibaba/async_simple
    GIT_TAG 1.0
    PATCH_COMMAND git checkout . && git apply ${CMAKE_CURRENT_SOURCE_DIR}/patch/async_simple_build_error.patch
    OPTIONS "ASYNC_SIMPLE_ENABLE_TESTS OFF" "ASYNC_SIMPLE_BUILD_DEMO_EXAMPLE OFF")
    

add_executable(test_async src/main.cxx)

target_include_directories(test_async PRIVATE ${asio_SOURCE_DIR}/asio/include)
target_link_libraries(test_async PRIVATE unifex async_simple)