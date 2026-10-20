#include "orderbook/OrderBook.hpp"
#include <cassert>
#include <iostream>

using namespace orderbook;

void testRestingOrdersAndSpread() {
    OrderBook book;

    // Adiciona Bid a 100 e Ask a 105
    auto r1 = book.addOrder(1, Side::Buy, OrderType::Limit, 10000, 10);
    assert(r1.empty());

    auto r2 = book.addOrder(2, Side::Sell, OrderType::Limit, 10500, 10);
    assert(r2.empty());

    assert(book.getBestBid() == 10000);
    assert(book.getBestAsk() == 10500);
    assert(book.getSpread() == 500); // $5.00 de spread

    std::cout << "[PASS] testRestingOrdersAndSpread\n";
}

void testPriceTimeFifoMatching() {
    OrderBook book;

    // Dois asks no mesmo preco (10500), ordem 1 chegou antes da ordem 2
    book.addOrder(1, Side::Sell, OrderType::Limit, 10500, 10);
    book.addOrder(2, Side::Sell, OrderType::Limit, 10500, 15);

    // Ordem de compra grande o suficiente para casar a primeira e parte da segunda
    auto trades = book.addOrder(3, Side::Buy, OrderType::Limit, 10500, 15);

    assert(trades.size() == 2);
    // Primeiro trade deve ser com a ordem 1 (FIFO Time priority)
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].matchQuantity == 10);

    // Segundo trade deve ser com a ordem 2 (parcial)
    assert(trades[1].sellOrderId == 2);
    assert(trades[1].matchQuantity == 5);

    // Ordem 2 deve restar com 10 unidades
    auto depth = book.getDepth(1);
    assert(depth.asks.size() == 1);
    assert(depth.asks[0].quantity == 10);

    std::cout << "[PASS] testPriceTimeFifoMatching\n";
}

void testOrderCancellation() {
    OrderBook book;

    book.addOrder(1, Side::Buy, OrderType::Limit, 9900, 20);
    assert(book.totalOrders() == 1);
    assert(book.getBestBid() == 9900);

    assert(book.cancelOrder(1));
    assert(book.totalOrders() == 0);
    assert(!book.getBestBid().has_value());

    // Cancelar ordem inexistente deve retornar false
    assert(!book.cancelOrder(999));

    std::cout << "[PASS] testOrderCancellation\n";
}

void testMarketOrderSweep() {
    OrderBook book;

    book.addOrder(1, Side::Sell, OrderType::Limit, 10000, 10);
    book.addOrder(2, Side::Sell, OrderType::Limit, 10100, 10);
    book.addOrder(3, Side::Sell, OrderType::Limit, 10200, 10);

    // Market Buy de 25 unidades deve varrer nivel 1 (10), nivel 2 (10) e pegar 5 do nivel 3
    auto trades = book.addOrder(4, Side::Buy, OrderType::Market, 0, 25);

    assert(trades.size() == 3);
    assert(trades[0].matchPrice == 10000 && trades[0].matchQuantity == 10);
    assert(trades[1].matchPrice == 10100 && trades[1].matchQuantity == 10);
    assert(trades[2].matchPrice == 10200 && trades[2].matchQuantity == 5);

    assert(book.getBestAsk() == 10200);
    auto depth = book.getDepth(1);
    assert(depth.asks[0].quantity == 5);

    std::cout << "[PASS] testMarketOrderSweep\n";
}

int main() {
    testRestingOrdersAndSpread();
    testPriceTimeFifoMatching();
    testOrderCancellation();
    testMarketOrderSweep();
    std::cout << "Todos os testes de OrderBook passaram com sucesso!\n";
    return 0;
}
