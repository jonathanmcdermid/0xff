#ifndef API_CLIENT_HPP
#define API_CLIENT_HPP

#include <stdexcept>
#include <string>
#include <vector>

// A simple struct for market data
struct MarketData
{
    std::string symbol;
    double currentPrice;
    double lastOrderPrice;
};

// APIClient class definition
class APIClient
{
  public:
    // Constructor to initialize with API credentials
    APIClient(const std::string& apiToken, const std::string& apiBaseURL);

    // Method to fetch market data for a specific symbol
    MarketData getMarketData(const std::string& symbol);

    // Method to place an order
    void placeOrder(const std::string& symbol, int quantity, double price);

  private:
    // Helper methods for API communication
    std::string makeGETRequest(const std::string& endpoint);
    std::string makePOSTRequest(const std::string& endpoint, const std::string& payload);

    // Member variables
    std::string apiToken;   // API token for authentication
    std::string apiBaseURL; // Base URL for API requests
};

#endif // API_CLIENT_HPP
