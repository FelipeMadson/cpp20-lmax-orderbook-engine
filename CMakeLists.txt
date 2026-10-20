cmake_minimum_required(VERSION 3.20)
project(cpp20_lmax_orderbook_engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Otimizações para compiladores modernos (GCC, Clang, MSVC)
if(MSVC)
    add_compile_options(/W4 /permissive- /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -march=native)
endif()

include_directories(include)

# Biblioteca estática do OrderBook
add_library(orderbook_core STATIC
    src/OrderBook.cpp
)

# Executável de Demonstração e Benchmark
add_executable(orderbook_benchmark
    src/main.cpp
)
target_link_libraries(orderbook_benchmark PRIVATE orderbook_core)

# Suíte de Testes
enable_testing()

add_executable(test_ring_buffer tests/test_ring_buffer.cpp)
add_test(NAME test_ring_buffer COMMAND test_ring_buffer)

add_executable(test_order_book tests/test_order_book.cpp)
target_link_libraries(test_order_book PRIVATE orderbook_core)
add_test(NAME test_order_book COMMAND test_order_book)

add_executable(test_latency tests/test_latency.cpp)
target_link_libraries(test_latency PRIVATE orderbook_core)
add_test(NAME test_latency COMMAND test_latency)
