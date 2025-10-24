#pragma once

#include <vector>
#include "common.h"


struct LevelInfo 
{
    Price price_;
    Quantity quantity_;
};

using LevelInfos = std::vector<LevelInfo>;

class OrderbookLevelInfos
{
    private:
        LevelInfos m_bids;
        LevelInfos m_asks;
    public:
        OrderbookLevelInfos(const LevelInfos& bids, const LevelInfos& asks) : m_bids {bids}, m_asks {asks}
        {
            // Pass
        }

        const LevelInfos& getBids() const { return m_bids; }
        const LevelInfos& getAsks() const { return m_asks; }

};