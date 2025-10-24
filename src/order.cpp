#include "order.h"

void Order::Fill(Quantity quantity)
{
    if (quantity > getRemainingQuantity())
        throw std::logic_error(
            std::format("Order ({}) cannot be filled more then remaining quantity", getOrderId())
        );
    m_remainingQuantity -= quantity;
}