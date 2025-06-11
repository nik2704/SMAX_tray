/**
 * @file ChatMessage.h
 * @brief Defines the MessageChat structure representing a chat message.
 */

#pragma once

#include <string>

namespace smax {

/**
 * @struct MessageChat
 * @brief Represents a chat message with basic details.
 */
struct MessageChat {
    std::string id;        ///< Unique identifier of the message.
    std::string text;      ///< Text content of the message.
    bool from_user;        ///< Indicates if the message was sent by the user (true) or by the system/assistant (false).
    std::string timestamp; ///< Timestamp of when the message was created or received.
};

} // namespace smax
