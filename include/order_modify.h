#pragma once

#include "common.h"
#include "order.h"

class OrderModify
{
    private:
        OrderId m_orderId;
        Price m_price;
        Side m_side;
        Quantity m_quantity;

    public:
        OrderModify(const OrderId orderId, const Price price, const Side side, const Quantity quantity)
            : m_orderId { orderId }, m_price { price }, m_side { side }, m_quantity { quantity }
            { 
                // Pass
            }
        
        OrderId getOrderId() const { return m_orderId; }
        Price getPrice() const { return m_price; }
        Side getSide() const { return m_side; }
        Quantity getQuantity() const { return m_quantity; }

        OrderPointer toOrderPointer(OrderType type) const
        {
            return std::make_shared<Order>(type, getOrderId(), getSide(), getPrice(), getQuantity());
        }
        
        
};