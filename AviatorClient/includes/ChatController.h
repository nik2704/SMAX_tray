#pragma once

#include <atomic>
#include <optional>
#include <vector>
#include <mutex>
#include <string>
#include <thread>
#include <functional>
#include <memory>
#include <mutex>

#include "ChatUI.h"
#include "MessageChat.h"
#include "Link.h"
#include "ConversationManager.h"
#include "StringJSONExtractor.h"

namespace smax {

// Forward declaration of the ChatUI class
class ChatUI;

/**
 * @class ChatController
 * @brief Manages the interaction between the Chat UI and backend conversation logic.
 *
 * Handles user input, sends messages, manages conversations, and updates the UI.
 */
class ChatController : public std::enable_shared_from_this<ChatController> {
public:
    /**
     * @brief Constructs a ChatController with all necessary configuration parameters.
     *
     * @param ui Reference to the Chat UI.
     * @param host The backend server address.
     * @param tenant_id Identifier for the tenant context.
     * @param client Client identifier string.
     * @param tag Optional tag for log filtering or tracking.
     * @param aviator_model The ID or name of the Aviator model used.
     * @param username The user’s login name.
     * @param password The user's password for authentication.
     * @param min_log_level Minimum log level for internal logging.
     */
    explicit ChatController(
        ChatUI& ui,
        const std::string& host,
        const std::string& tenant_id,
        const std::string& client,
        const std::string& tag,
        const std::string& aviator_model,
        const std::string& username,
        const std::string& password,
        const std::string& min_log_level
    );

    /**
     * @brief Loads an existing or new conversation from the ConversationManager.
     * 
     * Initializes conversation context and prepares chat state.
     */
    void loadConversation();

    /**
     * @brief Sends a user message to the backend and updates the UI.
     * 
     * @param message The message content entered by the user.
     */
    void sendChatMessage(const std::string & message);

    /**
     * @brief Resets the current conversation, clearing history and context.
     * 
     * Useful for starting a new topic or when resetting state is required.
     */
    void resetConversation();

private:
    ChatUI& chatUI_;                        ///< Reference to the user interface handler.
    std::string host_;                      ///< Hostname or URL of SMAX.
    std::string tenant_id_;                 ///< SMAX Tenant ID
    std::string client_;                    ///< Client name or ID (Aviator API).
    std::string tag_;                       ///< Tag (Aviator API)).
    std::string aviator_model_;             ///< Aviator AI model identifier.
    std::string user_name_;                 ///< Username for authentication.
    std::string password_;                  ///< Password for authentication.
    std::string min_log_level_;             ///< Minimum log level for internal logging.
    ConversationManager conversationManager_; ///< Handles chat session logic and state.

    std::string topic_id_;                  ///< Current conversation topic ID.
    std::string searchedContext_;           ///< Context from previous search or response.
    std::string last_message_;              ///< Stores the last sent message.
    char inputBuffer_[2048]{};              ///< Input buffer for composing user messages.

    std::mutex chatMutex_;                  ///< Ensures thread-safe access to chat data.
    std::atomic<bool> isChatSending_;

    /**
     * @brief Fills out contextual links in the UI from a list of references.
     *
     * @param references A vector of Link objects containing reference data.
     */
    void fillOutLinks(const std::vector<smax::Link>& references);

    /**
     * @brief Fills out contextual links in the UI from a context object.
     *
     * @param context A shared pointer to the last known context from the backend.
     */
    void fillOutLinks(const std::shared_ptr<smax::LastContext>& context);
};

} // namespace smax
