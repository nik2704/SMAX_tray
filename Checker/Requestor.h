/// @file Requestor.h
/// @brief Declares the smax::Requestor class responsible for performing HTTP GET requests with basic authentication.

#pragma once
#include <optional>
#include <string>

namespace smax {

/**
 * @class Requestor
 * @brief Provides static methods for making authenticated HTTP GET requests.
 *
 * The Requestor class encapsulates logic for sending GET requests with basic authentication.
 * It returns the response as a string if the request succeeds.
 */
class Requestor {
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
};

} // namespace smax
