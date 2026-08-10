#include "MerkelMain.h"
#include <iostream>
#include <vector>
#include "OrderBookEntry.h"
#include "CSVReader.h"
#include <fstream>
#include <random>

MerkelMain::MerkelMain()
{

}

void MerkelMain::init()
{
    int input;
    currentTime = orderBook.getEarliestTime();

    // wallet.insertCurrency("BTC", 1000); // Removed hardcoded funds

    while(signedInUser.empty())
    {
        showLoginMenu();
    }

    reloadWallet(); // Load user wallet

    while(true)
    {
        printMenu();
        input = getUserOption();
        processUserOption(input);
    }
}

void MerkelMain::showLoginMenu()
{
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "Enter option: ";
    std::string line;
    std::getline(std::cin, line);
    int option = 0;
    try {
        option = std::stoi(line);
    } catch(...) {}

    if (option == 1) registerUser();
    else if (option == 2) login();
    else std::cout << "Invalid option." << std::endl;
}

void MerkelMain::registerUser()
{
    std::string email, fullname, password;
    std::cout << "Enter email: ";
    std::getline(std::cin, email);
    std::cout << "Enter full name: ";
    std::getline(std::cin, fullname);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    // Check duplicates
    std::ifstream file("users.csv");
    std::string line;
    while(std::getline(file, line))
    {
        std::vector<std::string> tokens = CSVReader::tokenise(line, ',');
        if (tokens.size() >= 3)
        {
            if (tokens[2] == email)
            {
                std::cout << "User with this email already exists." << std::endl;
                return;
            }
        }
    }
    file.close();

    // Generate username
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<long long> dist(1000000000, 9999999999);
    std::string username = std::to_string(dist(gen));
    std::cout << "Your generated username is: " << username << std::endl;

    // Hash password
    std::hash<std::string> hasher;
    size_t hash = hasher(password);

    // Save
    std::ofstream outfile("users.csv", std::ios::app);
    outfile << username << "," << hash << "," << email << "," << fullname << std::endl;
    outfile.close();

    std::cout << "Registration successful! You can now login." << std::endl;
}

void MerkelMain::login()
{
    std::string username, password;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    std::hash<std::string> hasher;
    size_t hash = hasher(password);
    std::string hashStr = std::to_string(hash);

    std::ifstream file("users.csv");
    std::string line;
    bool found = false;
    while(std::getline(file, line))
    {
        std::vector<std::string> tokens = CSVReader::tokenise(line, ',');
        if (tokens.size() >= 2)
        {
            if (tokens[0] == username && tokens[1] == hashStr)
            {
                signedInUser = username;
                found = true;
                break;
            }
        }
    }
    file.close();

    if (found)
    {
        std::cout << "Login successful! Welcome " << signedInUser << std::endl;
    }
    else
    {
        std::cout << "Invalid username or password." << std::endl;
    }
}


void MerkelMain::printMenu()
{
    // 1 print help
    std::cout << "1: Print help " << std::endl;
    // 2 print exchange stats
    std::cout << "2: Print exchange stats" << std::endl;
    // 3 make an offer
    std::cout << "3: Make an offer " << std::endl;
    // 7 print candlesticks
    std::cout << "7: Print candlesticks " << std::endl;
    // 8 deposit/withdraw
    std::cout << "8: Deposit/Withdraw " << std::endl;
    // 9 run simulation
    std::cout << "9: Run Simulation " << std::endl;
    // 4 make a bid 
    std::cout << "4: Make a bid " << std::endl;
    // 5 print wallet
    std::cout << "5: Print wallet " << std::endl;
    // 6 continue   
    std::cout << "6: Continue " << std::endl;

    std::cout << "============== " << std::endl;

    std::cout << "Current time is: " << currentTime << std::endl;
}

void MerkelMain::enterDeposit()
{
    std::string product;
    std::string amountStr;
    std::cout << "Enter currency to deposit (e.g. BTC): ";
    std::getline(std::cin, product);
    std::cout << "Enter amount: ";
    std::getline(std::cin, amountStr);

    try {
        double amount = std::stod(amountStr);
        OrderBookEntry obe(0, amount, currentTime, product, OrderBookType::deposit, signedInUser);
        logTransaction(obe);
        std::cout << "Deposited " << amount << " " << product << std::endl;
    } catch (...) {
        std::cout << "Invalid amount." << std::endl;
    }
}

void MerkelMain::enterWithdraw()
{
    std::string product;
    std::string amountStr;
    std::cout << "Enter currency to withdraw (e.g. BTC): ";
    std::getline(std::cin, product);
    std::cout << "Enter amount: ";
    std::getline(std::cin, amountStr);

    try {
        double amount = std::stod(amountStr);
        if (wallet.containsCurrency(product, amount))
        {
            OrderBookEntry obe(0, amount, currentTime, product, OrderBookType::withdrawal, signedInUser);
            logTransaction(obe);
            std::cout << "Withdrew " << amount << " " << product << std::endl;
        } else {
            std::cout << "Insufficient funds." << std::endl;
        }
    } catch (...) {
        std::cout << "Invalid amount." << std::endl;
    }
}

void MerkelMain::logTransaction(OrderBookEntry& e)
{
    userTransactions.push_back(e);
    std::ofstream file("transactions.csv", std::ios::app);
    // Format: timestamp,product,type,price,amount,username
    std::string typeStr;
    if (e.orderType == OrderBookType::ask) typeStr = "ask";
    else if (e.orderType == OrderBookType::bid) typeStr = "bid";
    else if (e.orderType == OrderBookType::asksale) typeStr = "asksale";
    else if (e.orderType == OrderBookType::bidsale) typeStr = "bidsale";
    else if (e.orderType == OrderBookType::deposit) typeStr = "deposit";
    else if (e.orderType == OrderBookType::withdrawal) typeStr = "withdrawal";
    else typeStr = "unknown";

    file << e.timestamp << "," << e.product << "," << typeStr << "," << e.price << "," << e.amount << "," << e.username << std::endl;
    file.close();

    // Update wallet immediately for direct actions
    if (e.orderType == OrderBookType::deposit)
    {
        wallet.insertCurrency(e.product, e.amount);
    }
    if (e.orderType == OrderBookType::withdrawal)
    {
        wallet.removeCurrency(e.product, e.amount);
    }
    if (e.orderType == OrderBookType::asksale || e.orderType == OrderBookType::bidsale)
    {
        wallet.processSale(e);
    }
}

void MerkelMain::reloadWallet()
{
    wallet = Wallet();
    userTransactions.clear();

    std::ifstream file("transactions.csv");
    std::string line;
    while(std::getline(file, line))
    {
         std::vector<std::string> tokens = CSVReader::tokenise(line, ',');
         if (tokens.size() >= 6)
         {
             if (tokens[5] == signedInUser)
             {
                 double price = std::stod(tokens[3]);
                 double amount = std::stod(tokens[4]);
                 OrderBookEntry obe(price, amount, tokens[0], tokens[1],
                                    OrderBookEntry::stringToOrderBookType(tokens[2]),
                                    tokens[5]);

                 userTransactions.push_back(obe);

                 // Apply to wallet
                 if (obe.orderType == OrderBookType::deposit) wallet.insertCurrency(obe.product, obe.amount);
                 if (obe.orderType == OrderBookType::withdrawal) wallet.removeCurrency(obe.product, obe.amount);
                 if (obe.orderType == OrderBookType::asksale || obe.orderType == OrderBookType::bidsale) wallet.processSale(obe);
             }
         }
    }
}

void MerkelMain::printCandlesticks()
{
    std::string product;
    std::string typeStr;
    std::cout << "Enter product (e.g., ETH/BTC): ";
    std::getline(std::cin, product);

    std::cout << "Enter type (ask/bid): ";
    std::getline(std::cin, typeStr);

    OrderBookType type = OrderBookType::unknown;
    if (typeStr == "ask") type = OrderBookType::ask;
    else if (typeStr == "bid") type = OrderBookType::bid;
    else {
        std::cout << "Invalid type." << std::endl;
        return;
    }

    std::vector<Candlestick> candles = orderBook.computeCandlesticks(product, type);

    if (candles.empty())
    {
        std::cout << "No data found." << std::endl;
        return;
    }

    std::cout << "Candlestick Data for " << product << " (" << typeStr << "):" << std::endl;
    for (Candlestick& c : candles)
    {
        c.print();
        // Text-based plot (simple visualization)
        // Normalize height to range 0-10 based on price
        // This is a very simple relative visualization
        // A better plot would require knowing the global min/max of the data set.
    }

    // Visualize
    std::cout << "\nPlot:" << std::endl;
    double globalMax = 0;
    double globalMin = 1e9;

    for (Candlestick& c : candles)
    {
        if (c.high > globalMax) globalMax = c.high;
        if (c.low < globalMin) globalMin = c.low;
    }

    for (Candlestick& c : candles)
    {
        std::cout << c.date << " | ";
        if (globalMax == globalMin) {
            std::cout << "*" << std::endl;
            continue;
        }

        // Simple bar representing range from low to high
        int range = 20; // width of plot
        int startPos = (int)(((c.low - globalMin) / (globalMax - globalMin)) * range);
        int length = (int)(((c.high - c.low) / (globalMax - globalMin)) * range);
        if (length == 0) length = 1;

        for (int i = 0; i < startPos; i++) std::cout << " ";
        for (int i = 0; i < length; i++) std::cout << "#";
        std::cout << " (" << c.low << "-" << c.high << ")" << std::endl;
    }
}

void MerkelMain::printHelp()
{
    std::cout << "Help - your aim is to make money. Analyse the market and make bids and offers. " << std::endl;
}

void MerkelMain::printMarketStats()
{
    for (std::string const& p : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << p << std::endl;
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, 
                                                                p, currentTime);
        std::cout << "Asks seen: " << entries.size() << std::endl;
        std::cout << "Max ask: " << OrderBook::getHighPrice(entries) << std::endl;
        std::cout << "Min ask: " << OrderBook::getLowPrice(entries) << std::endl;



    }
    // std::cout << "OrderBook contains :  " << orders.size() << " entries" << std::endl;
    // unsigned int bids = 0;
    // unsigned int asks = 0;
    // for (OrderBookEntry& e : orders)
    // {
    //     if (e.orderType == OrderBookType::ask)
    //     {
    //         asks ++;
    //     }
    //     if (e.orderType == OrderBookType::bid)
    //     {
    //         bids ++;
    //     }  
    // }    
    // std::cout << "OrderBook asks:  " << asks << " bids:" << bids << std::endl;

}

void MerkelMain::enterAsk()
{
    std::cout << "Make an ask - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterAsk Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2], 
                currentTime, 
                tokens[0], 
                OrderBookType::ask 
            );
            obe.username = signedInUser; // Use signed in user
            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
                logTransaction(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterAsk Bad input " << std::endl;
        }   
    }
}

void MerkelMain::enterBid()
{
    std::cout << "Make an bid - enter the amount: product,price, amount, eg  ETH/BTC,200,0.5" << std::endl;
    std::string input;
    std::getline(std::cin, input);

    std::vector<std::string> tokens = CSVReader::tokenise(input, ',');
    if (tokens.size() != 3)
    {
        std::cout << "MerkelMain::enterBid Bad input! " << input << std::endl;
    }
    else {
        try {
            OrderBookEntry obe = CSVReader::stringsToOBE(
                tokens[1],
                tokens[2], 
                currentTime, 
                tokens[0], 
                OrderBookType::bid 
            );
            obe.username = signedInUser; // Use signed in user

            if (wallet.canFulfillOrder(obe))
            {
                std::cout << "Wallet looks good. " << std::endl;
                orderBook.insertOrder(obe);
                logTransaction(obe);
            }
            else {
                std::cout << "Wallet has insufficient funds . " << std::endl;
            }
        }catch (const std::exception& e)
        {
            std::cout << " MerkelMain::enterBid Bad input " << std::endl;
        }   
    }
}

void MerkelMain::printWallet()
{
    std::cout << wallet.toString() << std::endl;

    std::cout << "Recent Transactions:" << std::endl;
    int count = 0;
    // Show last 5
    for (auto it = userTransactions.rbegin(); it != userTransactions.rend(); ++it)
    {
        if (count >= 5) break;
        std::cout << it->timestamp << " " << it->product << " "
                  << (it->orderType == OrderBookType::deposit ? "Deposit" :
                      (it->orderType == OrderBookType::withdrawal ? "Withdrawal" :
                       (it->orderType == OrderBookType::asksale ? "Sold" :
                        (it->orderType == OrderBookType::bidsale ? "Bought" : "Unknown"))))
                  << " Amount: " << it->amount << " Price: " << it->price << std::endl;
        count++;
    }

    // Stats
    int asks = 0;
    int bids = 0;
    double totalSpent = 0;

    // Check all orders in OrderBook? Or User Transactions?
    // Task 3.4 says "stats of user activity (no of asks and bids... total money spent)"
    // Asks/Bids are Orders. Sales are Transactions.
    // I need to count asks/bids from OrderBook?
    // But OrderBook doesn't easily give me *my* orders unless I search.
    // I can assume userTransactions contains everything if I logged everything.
    // But I only log Transactions (Sales/Deposit).
    // "no of asks and bids" implies active orders or placed orders.
    // I should iterate `orderBook` to find my active orders.
    // Or I should log "ask" and "bid" placement to `transactions.csv` but NOT apply to wallet.
    // Plan: log placement of ask/bid to `transactions.csv` (OrderBookType::ask/bid).
    // In `logTransaction`, I already handle `ask`/`bid` by doing nothing to wallet.
    // So I should call `logTransaction` in `enterAsk/Bid`.

    // Let's implement the stats assuming `userTransactions` contains asks/bids too.
    for (const auto& t : userTransactions)
    {
        if (t.orderType == OrderBookType::ask) asks++;
        if (t.orderType == OrderBookType::bid) bids++;
        if (t.orderType == OrderBookType::bidsale) // Money spent to buy
        {
            totalSpent += t.amount * t.price;
        }
    }
    std::cout << "Stats: Asks: " << asks << " Bids: " << bids << " Total Spent: " << totalSpent << std::endl;
}
        
void MerkelMain::gotoNextTimeframe()
{
    std::cout << "Going to next time frame. " << std::endl;
    for (std::string p : orderBook.getKnownProducts())
    {
        std::cout << "matching " << p << std::endl;
        std::vector<OrderBookEntry> sales =  orderBook.matchAsksToBids(p, currentTime);
        std::cout << "Sales: " << sales.size() << std::endl;
        for (OrderBookEntry& sale : sales)
        {
            std::cout << "Sale price: " << sale.price << " amount " << sale.amount << std::endl; 
            if (sale.username == signedInUser)
            {
                // update the wallet
                // wallet.processSale(sale); // logTransaction does this
                logTransaction(sale);
            }
        }
        
    }

    currentTime = orderBook.getNextTime(currentTime);
}
 
int MerkelMain::getUserOption()
{
    int userOption = 0;
    std::string line;
    std::cout << "Type in 1-6" << std::endl;
    std::getline(std::cin, line);
    try{
        userOption = std::stoi(line);
    }catch(const std::exception& e)
    {
        // 
    }
    std::cout << "You chose: " << userOption << std::endl;
    return userOption;
}

void MerkelMain::processUserOption(int userOption)
{
    if (userOption == 0) // bad input
    {
        std::cout << "Invalid choice. Choose 1-6" << std::endl;
    }
    if (userOption == 1) 
    {
        printHelp();
    }
    if (userOption == 2) 
    {
        printMarketStats();
    }
    if (userOption == 3) 
    {
        enterAsk();
    }
    if (userOption == 4) 
    {
        enterBid();
    }
    if (userOption == 5) 
    {
        printWallet();
    }
    if (userOption == 6) 
    {
        gotoNextTimeframe();
    }
    if (userOption == 7)
    {
        printCandlesticks();
    }
    if (userOption == 8)
    {
        std::cout << "1. Deposit" << std::endl;
        std::cout << "2. Withdraw" << std::endl;
        int subopt = getUserOption();
        if (subopt == 1) enterDeposit();
        if (subopt == 2) enterWithdraw();
    }
    if (userOption == 9)
    {
        runSimulation();
    }
}

void MerkelMain::runSimulation()
{
    std::cout << "Running simulation..." << std::endl;
    for (std::string p : orderBook.getKnownProducts())
    {
        std::cout << "Product: " << p << std::endl;
        std::vector<OrderBookEntry> entries = orderBook.getOrders(OrderBookType::ask, p, currentTime);
        double price = 100.0;
        if (!entries.empty())
        {
             price = entries[0].price; // Use first found price as anchor
        }

        // Generate 5 bids and 5 asks
        for (int i=0; i<5; ++i)
        {
             try {
                 OrderBookEntry ask(price * (1.0 + (i+1)*0.01), 1.0, currentTime, p, OrderBookType::ask, signedInUser);
                 OrderBookEntry bid(price * (1.0 - (i+1)*0.01), 1.0, currentTime, p, OrderBookType::bid, signedInUser);

                 orderBook.insertOrder(ask);
                 logTransaction(ask);
                 orderBook.insertOrder(bid);
                 logTransaction(bid);
             } catch (...) {}
        }
    }
    std::cout << "Simulation complete. 5 asks and 5 bids added for all products." << std::endl;
}
