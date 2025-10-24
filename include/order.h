#pragma once

#include "common.h"
#include <memory>
#include <list>
#include <format>



class Order
{
    private:
        OrderType m_orderType;
        OrderId m_orderId;
        Side m_side;
        Price m_price;
        Quantity m_initialQuantity;
        Quantity m_remainingQuantity;

    public:
        Order(const OrderType orderType, const OrderId orderId, const Side side, const Price price, const Quantity quantity) 
        : m_orderType { orderType } , m_orderId { orderId }
        , m_side { side }, m_price { price }, m_initialQuantity { quantity }
        , m_remainingQuantity { quantity}
        { 
            // pass
        }

        OrderType getOrderType() const { return m_orderType; }
        OrderId getOrderId() const { return m_orderId; }
        Side getSide() const { return m_side; }
        Price getPrice() const { return m_price; }
        Quantity getInitialQuantity() const { return m_initialQuantity; }
        Quantity getRemainingQuantity() const { return m_remainingQuantity; }
        Quantity getFilledQuantity() const { return (getInitialQuantity() - getRemainingQuantity()); }
        bool isFilled() const { return getRemainingQuantity() == 0; }

        void Fill(Quantity quantity);

};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointersList = std::list<OrderPointer>;