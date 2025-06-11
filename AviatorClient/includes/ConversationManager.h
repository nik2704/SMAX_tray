/**
 * @file ConversationManager.h
 * @brief Manages SMAX Aviator conversations including creation, chat, and deletion.
 */

#pragma once

#include <utility>
#include <functional>
#include <string>
#include <memory>
#include "MessageAPI.h"
#include "../../libs/JSONhelper/json.h"
#include "../../libs/JSONhelper/json_builder.h"
#include "../../libs/JSONhelper/JSONhelper.h"
#include "StringJSONExtractor.h"
#include "ConversationStructures.h"

namespace smax {

/**
 * @class ConversationManager
 * @brief Handles conversation lifecycle and chat communication with SMAX Aviator.
 */
class ConversationManager {
public:
    /**
     * @brief Represents a pair of message history and the last context.
     */
    using MessageContextPair = std::pair<std::shared_ptr<std::vector<MessageAPI>>, std::shared_ptr<LastContext>>;

    /**
     * @brief Constructs a ConversationManager instance with required configuration.
     * 
     * @param host ITOM Aviator host URL.
     * @param tenant_id SMAX Tenant ID.
     * @param client Client ID (Aviator API).
     * @param tag Tag (Aviator API).
     * @param aviator_model Model name used by Aviator.
     * @param username Username for authentication.
     * @param password Password for authentication.
     * @param min_log_level Minimum log level for debugging and tracing.
     */
    ConversationManager(
        const std::string& host,
        const std::string& tenant_id,
        const std::string& client,
        const std::string& tag,
        const std::string& aviator_model,
        const std::string& username,
        const std::string& password,
        const std::string& min_log_level
    );

    /// Deleted copy constructor.
    ConversationManager(const ConversationManager&) = delete;

    /// Deleted assignment operator.
    ConversationManager& operator=(const ConversationManager&) = delete;

    /**
     * @brief Sets up a new conversation or retrieves an existing one.
     * 
     * @param error_message Output parameter to hold error messages if any.
     * @return MessageContextPair containing the message history and last context.
     */
    MessageContextPair setUpConversation(std::string& error_message);

    /**
     * @brief Sends a message to Aviator and handles streaming of response chunks.
     * 
     * @param chat_message The message to send.
     * @param searchedContext Context information for response generation.
     * @param text Additional textual content for the message.
     * @param error_message Output parameter to hold error messages if any.
     * @param onStreamChunk Callback to handle streamed response chunks.
     * @return True on success, false on failure.
     */
    bool chat(
        const std::string& chat_message,
        const std::string& searchedContext,
        const std::string& text,
        std::string& error_message,
        std::function<void(const std::string&)> onStreamChunk);

    /**
     * @brief Deletes the current conversation.
     * 
     * @param error_message Output parameter to hold error messages if any.
     * @return True on success, false on failure.
     */
    bool deleteConversation(std::string& error_message);

private:
    std::string host_;
    std::string tenant_id_;
    std::string client_;
    std::string tag_;
    std::string aviator_model_;
    std::string user_name_;
    std::string password_;
    std::string min_log_level_;

    std::string conversation_id_;
    std::string topic_id_;
    std::vector<MessageAPI> messages_;

    /**
     * @brief Sends chat input and starts a conversation session if necessary.
     */
    bool startChat(const std::string& chat_message, const std::string& searchedContext, const std::string& text, std::string& error_message);

    /// @name Internal URL builders
    /// @{
    std::string getBaseURL() const;
    std::string getConversationURL() const;
    std::string getConversationByIdURL() const;
    std::string getCreateConversationURL() const;
    std::string getChatCreateURL() const;
    std::string getChatCompletionsURL() const;
    std::string getDeleteConversationURL() const;
    /// @}

    /**
     * @brief Retrieves the current or previously saved conversation.
     */
    bool getConversation(MessageContextPair & conversation, std::string& error_message);

    /**
     * @brief Creates a new conversation on the Aviator backend.
     */
    bool createConversation(std::string& error_message);

    /**
     * @brief Builds JSON request body for a chat request.
     */
    bool getChatBody(std::string& json_body, const std::string& message, const std::string& searchedContext, const std::string& text, std::string chat_action, std::string& error_message);

    /**
     * @brief Builds JSON request body for conversation creation.
     */
    bool getConversationBody(std::string& json_body, std::string& error_message);

    /**
     * @brief Serializes a JSON node to a string.
     */
    bool getJSONstring(const json::Node& node, std::string & json_body, std::string& error_message) const;
};

} // namespace smax
