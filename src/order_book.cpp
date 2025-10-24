#include "order_book.h"

bool OrderBook::canMatch(const Side side, const Price price) const 
{
    if (side == Side::Buy)
    {
        if (m_asks.empty())
            return false;
        
        const auto& [bestAsk, _] = *m_asks.begin();
        return price >= bestAsk;
    }

    else
    {
        if (m_bids.empty())
            return false;

        const auto& [bestBid, _] = *m_bids.begin();
        return price <= bestBid;
    }
}

Trades OrderBook::matchOrders()
{
    Trades trades;
    trades.reserve(m_orders.size());

    while (true)
    {
        if (m_bids.empty() || m_asks.empty())
            break;

        auto& [bidPrice, bids] = *m_bids.begin();
        auto& [askPrice, asks] = *m_asks.begin(); 

        if (bidPrice < askPrice)
            break;

        while (!bids.empty() && !asks.empty())
        {
            auto& bid = bids.front();
            auto& ask = asks.front();

            Quantity min_quantity = std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

            bid->Fill(min_quantity);
            ask->Fill(min_quantity);

            if (bid->isFilled())
            {
                bids.pop_front();
                m_orders.erase(bid->getOrderId());
            }

            if (ask->isFilled())
            {
                asks.pop_front();
                m_orders.erase(ask->getOrderId());
            }

            if (bids.empty())
                m_bids.erase(bidPrice);
            
            if (asks.empty())
                m_asks.erase(askPrice);

            trades.push_back(Trade{ 
                TradeInfo { bid->getOrderId(), bid->getPrice(), min_quantity },
                TradeInfo { ask->getOrderId(), bid->getPrice(), min_quantity }
            });
        }

        if (!m_bids.empty())
        {
            auto& [_, bids] = *m_bids.begin();
            auto& order = bids.front();
            if (order->getOrderType() == OrderType::FillAndKill)
                cancelOrder(order->getOrderId());
        }

        if (!m_asks.empty())
        {
            auto& [_, asks] = *m_asks.begin();
            auto& order = asks.front();
            if (order->getOrderType() == OrderType::FillAndKill)
                cancelOrder(order->getOrderId());
        }

    }
    return trades;
}

Trades OrderBook::addOrder(OrderPointer order)
{
    if (m_orders.contains(order->getOrderId()))
        return {};
    
        // Fill and kill means execute immitiatly, if not dont add the order 
    if (order->getOrderType() == OrderType::FillAndKill && !canMatch(order->getSide(), order->getPrice()))
        return {};

    OrderPointersList::iterator iterator;

    if (order->getSide() == Side::Buy)
    {
        auto& orders = m_bids[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    else 
    {
        auto& orders = m_asks[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    m_orders.insert({ order->getOrderId(), OrderEntry{ order, iterator } });
    return matchOrders();
}

void OrderBook::cancelOrder(OrderId orderId)
{
    if (!m_orders.contains(orderId))
        return;
    
    auto& [order, iterator ] = m_orders.at(orderId);

    if (order->getSide() == Side::Sell)
    {
        auto price = order->getPrice();
        auto& orders = m_asks.at(price);
        orders.erase(iterator);
        if (orders.empty())
            m_asks.erase(price);
    }

    else 
    {
        auto price = order->getPrice();
        auto& orders = m_bids.at(price);
        orders.erase(iterator);
        if(orders.empty())
            m_bids.erase(price);
    }

        m_orders.erase(orderId);
}

Trades OrderBook::matchOrder(OrderModify order)
{
    if (!m_orders.contains(order.getOrderId()))
        return {};
    
    const auto& [existingOrder, _] = m_orders.at(order.getOrderId());
    cancelOrder(order.getOrderId());
    return addOrder(order.toOrderPointer(existingOrder->getOrderType()));
}


OrderbookLevelInfos OrderBook::getOrderInfos() const
{
    LevelInfos bidInfos, askInfos;
    bidInfos.reserve(m_orders.size());
    askInfos.reserve(m_orders.size());

    auto CreateLevelInfo = [](Price price, const OrderPointersList& orders) {
        return LevelInfo { price, 
            std::accumulate(
                orders.begin(), 
                orders.end(), 
                (Quantity)0,
                [](std::size_t runningSum, const OrderPointer& order) {
                    return runningSum + order->getRemainingQuantity();
                }
            ) 
        };
    };

    for (const auto& [price, orders] : m_bids)
        bidInfos.push_back(CreateLevelInfo(price, orders));

    for (const auto& [price, orders] : m_asks)
        askInfos.push_back(CreateLevelInfo(price, orders));

        return OrderbookLevelInfos{ bidInfos, askInfos };
}
