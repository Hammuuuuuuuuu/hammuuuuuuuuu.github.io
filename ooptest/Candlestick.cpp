#include "Candlestick.h"
#include <iostream>
#include <iomanip>

Candlestick::Candlestick(std::string _date,
                double _open,
                double _high,
                double _low,
                double _close,
                std::string _product,
                OrderBookType _type)
: date(_date), open(_open), high(_high), low(_low), close(_close), product(_product), type(_type)
{
}

void Candlestick::print()
{
    std::cout << date << " " << product << " " << (type == OrderBookType::ask ? "Ask" : "Bid")
              << " O:" << open << " H:" << high << " L:" << low << " C:" << close << std::endl;
}
