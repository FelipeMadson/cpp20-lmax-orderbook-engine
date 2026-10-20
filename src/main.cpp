#include "orderbook/OrderBook.hpp"
#include "orderbook/LockFreeRingBuffer.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace orderbook;

int main() {
    std::cout << "========================================================\n";
    std::cout << "  C++20 LMAX High-Frequency Trading (HFT) Order Book    \n";
    std::cout << "  Author: Felipe Madison (@FelipeMadson)               \n";
    std::cout << "  Architecture: Lock-Free RingBuffer + Price-Time FIFO  \n";
    std::cout << "========================================================\n\n";

    OrderBook book;
    LockFreeRingBuffer<Order, 65536> ringBuffer;

    std::cout << "[OrderBook] Inicializando benchmark de micro-latencia (100.000 ordens)...\n";

    constexpr size_t NumOrders = 100000;
    std::vector<uint64_t> latenciesNs;
    latenciesNs.reserve(NumOrders);

    size_t totalTrades = 0;

    auto benchmarkStart = std::chrono::high_resolution_clock::now();

    for (size_t i = 1; i <= NumOrders; ++i) {
        Side side = (i % 2 == 0) ? Side::Buy : Side::Sell;
        Price price = 10000 + (i % 50); // Preços oscilando em torno de $100.00
        Quantity qty = 10 + (i % 5) * 5;

        auto t0 = std::chrono::high_resolution_clock::now();
        auto reports = book.addOrder(i, side, OrderType::Limit, price, qty);
        auto t1 = std::chrono::high_resolution_clock::now();

        totalTrades += reports.size();
        latenciesNs.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    auto benchmarkEnd = std::chrono::high_resolution_clock::now();
    auto totalDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(benchmarkEnd - benchmarkStart).count();

    std::sort(latenciesNs.begin(), latenciesNs.end());

    double meanNs = std::accumulate(latenciesNs.begin(), latenciesNs.end(), 0.0) / latenciesNs.size();
    uint64_t p50Ns = latenciesNs[latenciesNs.size() * 50 / 100];
    uint64_t p90Ns = latenciesNs[latenciesNs.size() * 90 / 100];
    uint64_t p99Ns = latenciesNs[latenciesNs.size() * 99 / 100];
    uint64_t minNs = latenciesNs.front();
    uint64_t maxNs = latenciesNs.back();

    std::cout << "\n--- Resultados do Benchmark de Execucao ---\n";
    std::cout << "  Total de Ordens Ingeridas: " << NumOrders << "\n";
    std::cout << "  Total de Execucoes/Trades: " << totalTrades << "\n";
    std::cout << "  Tempo Total de Execucao:   " << totalDurationMs << " ms\n";
    std::cout << "  Vazao Efetiva:             " << (NumOrders * 1000.0 / (totalDurationMs > 0 ? totalDurationMs : 1)) << " ordens/segundo\n";
    std::cout << "-------------------------------------------\n";
    std::cout << "  Latencia Minima: " << minNs << " ns (" << (minNs / 1000.0) << " us)\n";
    std::cout << "  Latencia Media:  " << meanNs << " ns (" << (meanNs / 1000.0) << " us)\n";
    std::cout << "  Latencia P50:    " << p50Ns << " ns (" << (p50Ns / 1000.0) << " us)\n";
    std::cout << "  Latencia P90:    " << p90Ns << " ns (" << (p90Ns / 1000.0) << " us)\n";
    std::cout << "  Latencia P99:    " << p99Ns << " ns (" << (p99Ns / 1000.0) << " us)\n";
    std::cout << "  Latencia Maxima: " << maxNs << " ns (" << (maxNs / 1000.0) << " us)\n";
    std::cout << "-------------------------------------------\n\n";

    auto depth = book.getDepth(3);
    std::cout << "Livro de Ofertas Resumido (Top 3 Niveis):\n";
    std::cout << "  [ASKS / Vendas]\n";
    for (auto it = depth.asks.rbegin(); it != depth.asks.rend(); ++it) {
        std::cout << "    Preco: $" << (it->price / 100.0) << " | Qtd: " << it->quantity << " | Ordens: " << it->orderCount << "\n";
    }
    std::cout << "  ------ SPREAD: $" << (book.getSpread().value_or(0) / 100.0) << " ------\n";
    std::cout << "  [BIDS / Compras]\n";
    for (const auto& level : depth.bids) {
        std::cout << "    Preco: $" << (level.price / 100.0) << " | Qtd: " << level.quantity << " | Ordens: " << level.orderCount << "\n";
    }

    return 0;
}
