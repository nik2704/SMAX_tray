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

class ConversationManager {
public:
    using MessageContextPair = std::pair<std::shared_ptr<std::vector<MessageAPI>>, std::shared_ptr<LastContext>>;

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

    ConversationManager(const ConversationManager&) = delete;
    ConversationManager& operator=(const ConversationManager&) = delete;

    MessageContextPair setUpConversation(std::string& error_message);

    bool chat(
        const std::string& chat_message,
        const std::string& searchedContext,
        const std::string& text,
        std::string& error_message,
        std::function<void(const std::string&)> onStreamChunk);
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

    bool startChat(const std::string& chat_message, const std::string& searchedContext, const std::string& text, std::string& error_message);
    std::string getBaseURL() const;
    std::string getConversationURL() const;
    std::string getConversationByIdURL() const;
    std::string getCreateConversationURL() const;
    std::string getChatCreateURL() const;
    std::string getChatCompletionsURL() const;
    std::string getDeleteConversationURL() const;

    bool getConversation(MessageContextPair & conversation, std::string& error_message);
    bool createConversation(std::string& error_message);
    bool getChatBody(std::string& json_body, const std::string& message, const std::string& searchedContext, const std::string& text, std::string chat_action, std::string& error_message);
    bool getConversationBody(std::string& json_body, std::string& error_message);
    bool getJSONstring(const json::Node& node, std::string & json_body, std::string& error_message) const;
};

} // namespace smax