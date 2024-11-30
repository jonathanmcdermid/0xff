#include <fstream>
#include <iostream>
#include <string>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t total_size = size * nmemb;
    ((std::string*)userp)->append((char*)contents, total_size);
    return total_size;
}

std::string makePOSTRequest(const std::string& url)
{
    CURL* curl = curl_easy_init();
    if (!curl) { throw std::runtime_error("Failed to initialize cURL"); }

    CURLcode res;
    std::string response; // To hold the response from the server

    struct curl_slist* headers = nullptr;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Specify the callback function to capture the response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // TODO: Disable SSL verification (INSECURE)
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);

    // Perform the request
    res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        throw std::runtime_error("POST request failed: " + std::string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    return response;
}

void saveAuthResponse(const nlohmann::json& responseJson)
{
    std::ofstream outFile(std::getenv("AUTH_RESPONSE_PATH"));
    outFile << responseJson.dump(4);
    outFile.close();
}

nlohmann::json loadAuthResponse()
{
    std::ifstream inFile(std::getenv("AUTH_RESPONSE_PATH"));
    nlohmann::json responseJson;
    inFile >> responseJson;
    return responseJson;
}

void getAccessToken(const std::string& refreshToken)
{
    std::string url = "https://login.questrade.com/oauth2/token?grant_type=refresh_token&refresh_token=" + refreshToken;

    try
    {
        std::string response = makePOSTRequest(url);
        auto jsonData = nlohmann::json::parse(response);
        saveAuthResponse(jsonData);
        
        std::cout << "Access Token: " << jsonData["access_token"] << std::endl;
        std::cout << "New Refresh Token: " << jsonData["refresh_token"] << std::endl;
        std::cout << "API Server URL: " << jsonData["api_server"] << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) { throw std::invalid_argument("1 argument required.\nUsage: <project root>"); }

    std::string authResponsePath = std::string(argv[1]) + "/auth_response.json";

#ifdef _WIN32
    if (_putenv_s("AUTH_RESPONSE_PATH", authResponsePath.c_str()) != 0) { throw std::runtime_error("Failed to set project root."); }
#else
    if (setenv("AUTH_RESPONSE_PATH", authResponsePath.c_str(), 1) != 0) { throw std::runtime_error("Failed to set project root."); }
#endif
    
    try
    {
        nlohmann::json savedResponse = loadAuthResponse();
        std::string refreshToken = savedResponse["refresh_token"];
        std::cout << "Loaded Refresh Token: " << refreshToken << std::endl;

        getAccessToken(refreshToken);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error loading saved response: " << ex.what() << std::endl;
    }


    return 0;
}
