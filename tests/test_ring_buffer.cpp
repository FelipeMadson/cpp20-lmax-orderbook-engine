#include "orderbook/LockFreeRingBuffer.hpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

using namespace orderbook;

void testBasicPushPop() {
    LockFreeRingBuffer<int, 4> rb;

    assert(rb.empty());
    assert(rb.size() == 0);

    assert(rb.push(10));
    assert(rb.push(20));
    assert(rb.push(30));
    assert(rb.push(40));

    // Capacidade é 4, próximo push deve falhar por backpressure
    assert(!rb.push(50));
    assert(rb.size() == 4);

    int val = 0;
    assert(rb.pop(val) && val == 10);
    assert(rb.pop(val) && val == 20);
    assert(rb.size() == 2);

    assert(rb.push(60));
    assert(rb.pop(val) && val == 30);
    assert(rb.pop(val) && val == 40);
    assert(rb.pop(val) && val == 60);

    assert(rb.empty());
    assert(!rb.pop(val));

    std::cout << "[PASS] testBasicPushPop\n";
}

void testConcurrentSPSC() {
    LockFreeRingBuffer<uint64_t, 1024> rb;
    constexpr size_t TotalItems = 50000;

    std::thread producer([&]() {
        for (size_t i = 1; i <= TotalItems; ++i) {
            while (!rb.push(i)) {
                std::this_thread::yield();
            }
        }
    });

    std::vector<uint64_t> consumed;
    consumed.reserve(TotalItems);

    std::thread consumer([&]() {
        uint64_t val = 0;
        while (consumed.size() < TotalItems) {
            if (rb.pop(val)) {
                consumed.push_back(val);
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    assert(consumed.size() == TotalItems);
    for (size_t i = 0; i < TotalItems; ++i) {
        assert(consumed[i] == (i + 1));
    }

    std::cout << "[PASS] testConcurrentSPSC (50.000 itens sem perda)\n";
}

int main() {
    testBasicPushPop();
    testConcurrentSPSC();
    std::cout << "Todos os testes de LockFreeRingBuffer passaram com sucesso!\n";
    return 0;
}
