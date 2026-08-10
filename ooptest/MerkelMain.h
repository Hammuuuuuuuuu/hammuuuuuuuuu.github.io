#pragma once

#include <vector>
#include "OrderBookEntry.h"
#include "OrderBook.h"
#include "Wallet.h"


class MerkelMain
{
    public:
        MerkelMain();
        /** Call this to start the sim */
        void init();
    private: 
        void printMenu();
        void printHelp();
        void printMarketStats();
        void printCandlesticks();
        void enterAsk();
        void enterBid();
        void printWallet();
        void gotoNextTimeframe();
        int getUserOption();
        void processUserOption(int userOption);

        // Login/Register
        void showLoginMenu();
        void registerUser();
        void login();

        void enterDeposit();
        void enterWithdraw();
        void logTransaction(OrderBookEntry& e);
        void reloadWallet();
        void runSimulation();

        std::string currentTime;
        std::string signedInUser;
        std::vector<OrderBookEntry> userTransactions;

//        OrderBook orderBook{"20200317.csv"};
	OrderBook orderBook{"20200601.csv"};
        Wallet wallet;

};
