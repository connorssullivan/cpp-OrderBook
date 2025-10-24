#pragma once

#include <vector>


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




