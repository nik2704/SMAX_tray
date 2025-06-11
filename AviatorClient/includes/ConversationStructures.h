/**
 * @file ConversationStructures.h
 * @brief Contains data structures used for managing conversations and context in SMAX Aviator.
 */

#pragma once

#include <string>
#include <vector>

namespace smax {

/**
 * @struct ConversationParameters
 * @brief Holds metadata required to identify and describe a conversation.
 */
struct ConversationParameters {
    std::string id;     ///< Unique identifier for the conversation.
    std::string tag;    ///< Tag or label for categorizing the conversation.
    std::string name;   ///< Display name or title of the conversation.
    std::string client; ///< Identifier for the client application initiating the conversation.
};

/**
 * @struct ContextDoc
 * @brief Represents a single context document used in a conversation.
 */
struct ContextDoc {
    std::string id;     ///< Unique identifier for the context document.
    std::string type;   ///< Type or category of the document (e.g., KB, incident, etc.).
    std::string label;  ///< Human-readable label or title for the document.
    std::string url;    ///< URL linking to the full document or resource.
};

/**
 * @struct LastContext
 * @brief Holds the most recent context and referenced documents used in the conversation.
 */
struct LastContext {
    std::string text;   ///< User's last input or relevant textual content.
    std::string searchedContext; ///< Summary of the context used for search or inference.
    std::vector<ContextDoc> referencedDocs; ///< List of context documents referenced in the last response.
};

} // namespace smax
