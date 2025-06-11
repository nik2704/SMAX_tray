/**
 * @file MessageAPI.h
 * @brief Defines the MessageAPI structure representing a message exchanged in the API.
 */

#pragma once

#include <string>

/**
 * @struct MessageAPI
 * @brief Represents a single message in the conversation API.
 */
struct MessageAPI {
    std::string id;        ///< Unique identifier of the message.
    std::string topic_id;  ///< Identifier of the topic or conversation this message belongs to.
    std::string role;      ///< Role.
    std::string content;   ///< Text content of the message.
};
