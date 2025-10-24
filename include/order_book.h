#pragma once

#include <iostream>
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <limits>
#include <string>
#include <vector>
#include <numeric>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <memory>





#include "order_modify.h"
#include "common.h"
#include "order.h"
#include "trade.h"
#include "level_info.h"


class OrderBook
{
    private:
        struct OrderEntry
        {
            OrderPointer m_order { nullptr };
            OrderPointersList::iterator m_location;
        };

        std::map<Price, OrderPointersList, std::greater<Price>> m_bids;
        std::map<Price, OrderPointersList, std::less<Price>> m_asks;
        std::unordered_map<OrderId, OrderEntry> m_orders;

        bool canMatch(const Side side, const Price price) const;

        Trades matchOrders();
        

    public: 
        Trades addOrder(OrderPointer order);

        void cancelOrder(OrderId orderId);

        Trades matchOrder(OrderModify order);

        std::size_t size() const{return m_orders.size();}

        OrderbookLevelInfos getOrderInfos() const;
};