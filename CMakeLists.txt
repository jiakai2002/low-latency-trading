cmake_minimum_required(VERSION 3.20)
project(trading VERSION 1.0.0 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # High optimization level with LTO
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto -march=native")
    # Extra warnings and strict compilation
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
elseif(MSVC)
    # High optimization level
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL")
    # Warning level 4
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library sources (ignore misc/)
file(GLOB_RECURSE LIB_SOURCES
    "src/*.cpp"
    "src/**/*.cpp"
)

# Create the library
add_library(lib STATIC ${LIB_SOURCES})

# Executables
add_executable(benchmark benchmark/benchmark.cpp)
add_executable(simulator examples/simulator.cpp)
add_executable(test_order_book tests/test_order_book.cpp)
add_executable(test_market_data_handler tests/test_market_data_handler.cpp)

# Link library
target_link_libraries(benchmark PRIVATE lib)
target_link_libraries(simulator PRIVATE lib)
target_link_libraries(test_order_book PRIVATE lib)
target_link_libraries(test_market_data_handler PRIVATE lib)
