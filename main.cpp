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
#include <variant>
#include <optional>
#include <tuple>
#include <format>

enum class OrderType 
{
    GoodTilCancel,
    FillAndKill
};

enum class Side
{
    Buy,
    Sell
};

using Price = std::int32_t;
using Quantity = std::int32_t;
using OrderId = std::int64_t;

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

        void Fill(Quantity quantity)
        {
            if (quantity > getRemainingQuantity())
                throw std::logic_error(
                    std::format("Order ({}) cannot be filled more then remaining quantity", getOrderId())
                );
            m_remainingQuantity -= quantity;
        }

};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointersList = std::list<OrderPointer>;

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

        bool canMatch(const Side side, const Price price) const 
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

        Trades matchOrders()
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

    public: 
        Trades addOrder(OrderPointer order)
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

        void cancelOrder(OrderId orderId)
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

        Trades matchOrder(OrderModify order)
        {
            if (!m_orders.contains(order.getOrderId()))
                return {};
            
            const auto& [existingOrder, _] = m_orders.at(order.getOrderId());
            cancelOrder(order.getOrderId());
            return addOrder(order.toOrderPointer(existingOrder->getOrderType()));
        }

        std::size_t size() const
        {
            return m_orders.size();
        }

        OrderbookLevelInfos getOrderInfos() const
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


};

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
