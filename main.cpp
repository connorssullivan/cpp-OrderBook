#include "order_book.h"
#include <iostream>



int main()
{
    OrderBook orderbook;
    const OrderId orderId {1};
    orderbook.addOrder(std::make_shared<Order>(OrderType::GoodTilCancel, orderId, Side::Buy, 100, 10));

    std::cout << orderbook.size() << "\n";
    orderbook.cancelOrder(orderId);
    std::cout << orderbook.size() << "\n";

    return 0;
}
