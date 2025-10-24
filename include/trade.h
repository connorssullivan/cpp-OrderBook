#pragma once

#include "common.h"

struct TradeInfo
{
    OrderId m_orderId;
    Price m_price;
    Quantity m_quantity;
};

class Trade
{
    private:
        TradeInfo m_bidTrade;
        TradeInfo m_askTrade;
    public:
        Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
            : m_bidTrade { bidTrade }, m_askTrade { askTrade } {}

        const TradeInfo& getBidTrade() const { return m_bidTrade; }
        const TradeInfo& getAskTrade() const { return m_askTrade; }
};

using Trades = std::vector<Trade>;