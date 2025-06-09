/// @file NetworkClient.h
/// @brief Declares the smax::NetworkClient class responsible for performing HTTP GET requests with basic authentication.

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <windows.h>
#include <wininet.h>
#include <wincrypt.h>
#include "../JSONhelper/json.h"
#include "../JSONhelper/json_builder.h"
#include "../JSONhelper/JSONhelper.h"

namespace smax {

struct RequestResult {
    int code = -1;
    std::string result;
    std::string error_message;
};

/**
 * @class NetworkClient
 * @brief Provides static methods for making authenticated HTTP GET requests.
 *
 * The NetworkClient class encapsulates logic for sending GET requests with basic authentication.
 * It returns the response as a string if the request succeeds.
 */
class NetworkClient {
public:
    /**
     * @brief Sends an HTTP request with basic authentication.
     *
     * @param method The HTTP method (e.g., "GET", "POST").
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @param body The body of the request (for POST requests).
     * @param stream If true, the request is treated as a streaming request.
     * @param onChunk Optional callback function to handle chunks of data as they arrive.
     * @return A RequestResult struct with HTTP status code, result body and error message (if any).
     */
    static RequestResult sendRequest(const std::string& method,
                                        const std::string& url,
                                        const std::string& username,
                                        const std::string& password,
                                        const std::string& body,
                                        bool stream = false,
                                        std::function<void(const std::string&)> onChunk = {}
                                    );
    /**
     * @brief Sends an HTTP GET request with basic authentication.
     *
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @return The response object.
     */
    static RequestResult get(const std::string& url, const std::string& username, const std::string& password);

    /**
     * @brief Sends an HTTP POST request with a JSON body and basic authentication.
     *
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @param json_body The JSON body to be sent in the POST request.
     * @return A PostResult struct with HTTP status code, result body and error message (if any).
     */
    static RequestResult post(const std::string& url, const std::string& username, const std::string& password, const std::string& json_body);
    
    /**
     * @brief Sends an HTTP POST request with a JSON body and basic authentication, suitable for streaming responses.
     *
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @param json_body The JSON body to be sent in the POST request.
     * @param onChunk Optional callback function to handle chunks of data as they arrive.
     * @return A RequestResult struct with HTTP status code, result body and error message (if any).
     */
    static RequestResult postStream(
        const std::string& url,
        const std::string& username,
        const std::string& password,
        const std::string& json_body,
        std::function<void(const std::string&)> onChunk
    );

    /**
     * @brief Sends an HTTP DELETE request with basic authentication.
     *
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @return A RequestResult struct with HTTP status code, result body and error message (if any).
     */
    static RequestResult del(const std::string& url, const std::string& username, const std::string& password);

    static std::vector<std::string> extractIDsFromJSON(const std::string& json_str);

private:
    static RequestResult sendStreamingRequest(HINTERNET hRequest, std::function<void(const std::string&)> onChunk);

    static RequestResult sendStandardRequest(HINTERNET hRequest);
};

} // namespace smax
