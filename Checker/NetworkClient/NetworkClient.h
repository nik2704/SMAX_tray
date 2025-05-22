/// @file NetworkClient.h
/// @brief Declares the smax::NetworkClient class responsible for performing HTTP GET requests with basic authentication.

#pragma once

#include <optional>
#include <string>
#include <vector>
#include "JSONhelper/json.h"
#include "JSONhelper/json_builder.h"
#include "JSONhelper/JSONhelper.h"

namespace smax {

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
     * @brief Sends an HTTP GET request with basic authentication.
     *
     * @param url The full URL to which the request should be sent.
     * @param username The username for basic authentication.
     * @param password The password for basic authentication.
     * @return The response body as a string if the request is successful; std::nullopt otherwise.
     */
    static std::optional<std::string> get(const std::string& url, const std::string& username, const std::string& password);
    static std::vector<std::string> extractIDsFromJSON(const std::string& json_str);
};

} // namespace smax
