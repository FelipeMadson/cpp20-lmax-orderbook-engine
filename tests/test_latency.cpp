#include "orderbook/OrderBook.hpp"
#include <cassert>
#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

using namespace orderbook;

int main() {
    OrderBook book;
    constexpr size_t WarmupOrders = 5000;
    constexpr size_t TestOrders = 20000;

    // Warmup
    for (size_t i = 1; i <= WarmupOrders; ++i) {
        book.addOrder(i, (i % 2 == 0 ? Side::Buy : Side::Sell), OrderType::Limit, 10000 + (i % 20), 10);
    }

    std::vector<uint64_t> latenciesNs;
    latenciesNs.reserve(TestOrders);

    for (size_t i = WarmupOrders + 1; i <= WarmupOrders + TestOrders; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();
        book.addOrder(i, (i % 2 == 0 ? Side::Buy : Side::Sell), OrderType::Limit, 10000 + (i % 20), 10);
        auto t1 = std::chrono::high_resolution_clock::now();

        latenciesNs.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    double meanNs = std::accumulate(latenciesNs.begin(), latenciesNs.end(), 0.0) / latenciesNs.size();
    std::cout << "[LatencyTest] Amostras: " << TestOrders << " | Latencia Media: " << meanNs << " ns (" << (meanNs / 1000.0) << " us)\n";

    // Afirmação razoável para ambientes CI e máquinas locais (< 50 us mesmo em ambientes virtuais)
    assert(meanNs < 50000.0 && "A latência média deve estar estritamente na faixa de microssegundos.");

    std::cout << "[PASS] test_latency validado com sucesso!\n";
    return 0;
}
