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

class ChatUI;

class ChatController {
public:
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

    void ChatController::loadConversation();
    void sendChatMessage(const std::string & message);
    void resetConversation();

private:
    ChatUI& chatUI_;
    std::string host_;
    std::string tenant_id_;
    std::string client_;
    std::string tag_;
    std::string aviator_model_;
    std::string user_name_;
    std::string password_;
    std::string min_log_level_;
    ConversationManager conversationManager_;

    std::string topic_id_;
    std::string searchedContext_;
    std::string last_message_;
    char inputBuffer_[2048]{};

    std::mutex chatMutex_;

    void fillOutLinks(const std::vector<smax::Link> & references);
    void fillOutLinks(const std::shared_ptr<smax::LastContext> &context);
};

} // namespace smax