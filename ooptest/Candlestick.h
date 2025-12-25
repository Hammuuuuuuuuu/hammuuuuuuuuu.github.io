#pragma once
#include <string>
#include "OrderBookEntry.h"

class Candlestick {
public:
    /** Constructor to initialize a candlestick object */
    Candlestick(std::string _date,
                double _open,
                double _high,
                double _low,
                double _close,
                std::string _product,
                OrderBookType _type);

    // Data members (public for easy access in tables)
    std::string date;
    double open;
    double high;
    double low;
    double close;
    std::string product;
    OrderBookType type;

    /** Helper function to print the candlestick data in a table row format */
    void print();
};