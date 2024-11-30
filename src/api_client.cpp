#include "api_client.hpp"

#include <iostream>
#include <sstream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

// Constructor
APIClient::APIClient(const std::string& token, const std::string& baseURL) : apiToken(token), apiBaseURL(baseURL) {}

// Helper function to perform GET requests
std::string APIClient::makeGETRequest(const std::string& endpoint)
{
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize cURL");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (apiBaseURL + endpoint).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, {"Authorization: Bearer " + apiToken});
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, void* userdata) {
        ((std::string*)userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) { throw std::runtime_error("GET request failed: " + std::string(curl_easy_strerror(res))); }

    return response;
}

// Helper function to perform POST requests
std::string APIClient::makePOSTRequest(const std::string& endpoint, const std::string& payload)
{
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize cURL");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, (apiBaseURL + endpoint).c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, {"Authorization: Bearer " + apiToken, "Content-Type: application/json"});
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, [](char* ptr, size_t size, size_t nmemb, void* userdata) {
        ((std::string*)userdata)->append(ptr, size * nmemb);
        return size * nmemb;
    });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) { throw std::runtime_error("POST request failed: " + std::string(curl_easy_strerror(res))); }

    return response;
}

// Fetch market data
MarketData APIClient::getMarketData(const std::string& symbol)
{
    std::string endpoint = "/v1/markets/quotes?symbol=" + symbol; // Example endpoint
    std::string response = makeGETRequest(endpoint);

    // Parse the JSON response
    Json::Value jsonData;
    Json::CharReaderBuilder readerBuilder;
    std::string errs;
    std::istringstream sstream(response);
    if (!Json::parseFromStream(readerBuilder, sstream, &jsonData, &errs)) { throw std::runtime_error("Failed to parse market data: " + errs); }

    return {symbol, jsonData["currentPrice"].asDouble(), jsonData["lastOrderPrice"].asDouble()};
}

// Place an order
void APIClient::placeOrder(const std::string& symbol, int quantity, double price)
{
    std::string endpoint = "/v1/accounts/orders"; // Example endpoint
    Json::Value orderData;
    orderData["symbol"] = symbol;
    orderData["quantity"] = quantity;
    orderData["price"] = price;

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, orderData);

    makePOSTRequest(endpoint, payload);
}
