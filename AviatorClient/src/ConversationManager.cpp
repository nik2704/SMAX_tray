#include <iostream>
#include <sstream>
#include "ConversationManager.h"
#include "MessageAPI.h"
#include "../../libs/Logger/Logger.h"
#include "../../libs/NetworkClient/NetworkClient.h"

namespace smax {

ConversationManager::ConversationManager(
    const std::string& host,
    const std::string& tenant_id,
    const std::string& client,
    const std::string& tag,
    const std::string& aviator_model,
    const std::string& username,
    const std::string& password,
    const std::string& min_log_level
)
    : host_(host),
      tenant_id_(tenant_id),
      client_(client),
      tag_(tag),
      aviator_model_(aviator_model),
      user_name_(username),
      password_(password),
      min_log_level_(min_log_level),
      conversation_id_(""),
      topic_id_("")
{
    AppLogger::Logger::getInstance().setLogFile("smax_conversation.log");
    AppLogger::Logger::getInstance().setMinLogLevel(min_log_level_);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation manager started.");
}

ConversationManager::MessageContextPair ConversationManager::setUpConversation(std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "setUpConversation()");

    MessageContextPair conversation = {
        std::make_shared<std::vector<MessageAPI>>(),
        std::make_shared<LastContext>()
    };

    if (getConversation(conversation, error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation is found: " + conversation_id_);
        AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Context of the conversation: " + conversation.second->text);
        return conversation;
    }

    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation was not found");
    if (!createConversation(error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation was not created: " + error_message);
        return conversation;
    }

    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation created successfully");
    if (getConversation(conversation, error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Conversation is found after creation: " + conversation_id_);
        return conversation;
    }

    AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Conversation was not found after creation");
    return conversation;
}

bool ConversationManager::getConversation(MessageContextPair & conversation, std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversation()");
    auto conversation_result = NetworkClient::get(getConversationURL(), user_name_, password_);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversation() result code: " + std::to_string(conversation_result.code));
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversation() result: " + conversation_result.result);

    if (conversation_result.code == 200) {
        conversation = StringJSONExtractor::extractConversationFromJSON(conversation_result.result, conversation_id_, topic_id_);
        return true;
    } else {
        error_message = conversation_result.error_message;
    }

    conversation = { std::make_shared<std::vector<MessageAPI>>(), std::make_shared<LastContext>() };
    return false;
}

bool ConversationManager::createConversation(std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "createConversation()");
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "client_= " + client_);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "tag_= " + tag_);

    conversation_id_ = "";

    if (client_.empty() || tag_.empty()) {
        error_message = "Client or tag is empty.";
        return false;
    }

    std::string json_payload_string;
    if (!getConversationBody(json_payload_string, error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Conversation body was not created: " + error_message);
        return false;
    }

    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "json_payload_string: " + json_payload_string);

    auto conversation_result = NetworkClient::post(getCreateConversationURL(), user_name_, password_, json_payload_string);

    if (conversation_result.code != 201 && conversation_result.code != 200) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Conversation was not created: " + conversation_result.result);

        error_message = conversation_result.error_message;
        return false;
    }

    return true;
}

bool ConversationManager::startChat(const std::string& chat_message, const std::string& searchedContext, const std::string& text, std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "startChat()");
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "startChat() text: " + text);

    if (conversation_id_.empty()) {
        error_message = "Conversation ID is empty. Please create a conversation first.";
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    }
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "conversation_id_= " + conversation_id_);

    std::string json_body;
    if (!getChatBody(json_body, chat_message, searchedContext, text, "NewTopic", error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Chat body was not created: " + error_message);
        return false;
    }
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "chat Body= " + json_body);

    auto result = NetworkClient::post(getChatCreateURL(), user_name_, password_, json_body);
    if (result.code != 201 && result.code != 200) {
        error_message = result.error_message;
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Chat was not created: " + error_message);
        return false;
    }

    topic_id_ = StringJSONExtractor::getStringParameter(result.result, "topicId", error_message);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "topic_id_= " + topic_id_);

    if (topic_id_.empty()) {
        error_message = "Failed to extract topic ID from response. " + error_message;
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    }

    return true;
}

bool ConversationManager::chat(
    const std::string& chat_message,
    const std::string& searchedContext,
    const std::string& text,
    std::string& error_message,
    std::function<void(const std::string&)> onStreamChunk
) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "chat(): " + chat_message);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "topic_id_: " + topic_id_);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "searchedContext: " + searchedContext);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "chat() text: " + text);

    if (topic_id_.empty()) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Topic ID is empty. Starting a new chat.");
        if (!startChat(chat_message, searchedContext, text, error_message)) {
            AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "chat was not started: " + chat_message);
            return false;
        }
    }

    std::string json_body;
    if (!getChatBody(json_body, chat_message, searchedContext, text, "Next", error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "chat body was not created: " + error_message);
        return false;
    }

    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "json_body: " + json_body);

    // auto result = NetworkClient::postStream(getChatCompletionsURL(), user_name_, password_, json_body, [&](const std::string& chunk) {
    //     AppLogger::Logger::getInstance().log(AppLogger::INFO, "CHUNK: " + chunk);
    //     onStreamChunk(chunk);
    // }, tryExtractMessageFromBuffer);
    auto result = NetworkClient::postStream(getChatCompletionsURL(), user_name_, password_, json_body, onStreamChunk);

    if (result.code != 200 && result.code != 201) {
        error_message = result.error_message;
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "chat was not posted: " + error_message);
        return false;
    }

    return true;
}

bool ConversationManager::deleteConversation(std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "deleteConversation(), conversation_id: " + conversation_id_);
    auto result = NetworkClient::del(getDeleteConversationURL(), user_name_, password_);

    if (result.code != 204 && result.code != 200) {
        error_message = result.error_message;
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "conversation was not deleted" + error_message);
        return false;
    }


    conversation_id_ = "";
    topic_id_ = "";
    messages_.clear();

    return true;
}

std::string ConversationManager::getBaseURL() const {
    return "https://" +  host_ + "/rest/" + tenant_id_ + "/aviator/";
}

std::string ConversationManager::getConversationURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversationURL(): " +  getBaseURL() + "conversations/" + client_ + "/" + tag_);

    return getBaseURL() + "conversations/" + client_ + "/" + tag_;
}

std::string ConversationManager::getConversationByIdURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversationByIdURL(): " + getBaseURL() + "conversations/" + conversation_id_);

    return getBaseURL() + "conversations/" + conversation_id_;
}

std::string ConversationManager::getCreateConversationURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getCreateConversationURL(): " + getBaseURL() + "conversations");

    return getBaseURL() + "conversations";   
}

std::string ConversationManager::getChatCreateURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getChatCreateURL(): " + getBaseURL() + "chats");

    return getBaseURL() + "chats";
}

std::string ConversationManager::getChatCompletionsURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getChatCompletionsURL(): " + getBaseURL() + "chat/completions");

    return getBaseURL() + "chat/completions";
}

std::string ConversationManager::getDeleteConversationURL() const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getDeleteConversationURL(): " + getConversationByIdURL());

    return getConversationByIdURL();    
}

bool ConversationManager::getJSONstring(const json::Node& node, std::string & json_body, std::string& error_message) const {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getJSONstring()");

    try {
        std::stringstream ss;
        json::Print(json::Document{node}, ss);
        json_body = ss.str();
    } catch (const std::exception& e) {
        error_message = "Error converting JSON Node to string for chat: " + std::string(e.what());
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    } catch (...) {
        error_message = "Unknown error converting JSON Node to string for chat.";
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    }

    return true;
}


bool ConversationManager::getConversationBody(std::string& json_body, std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getConversationBody()");
    json::Node conversation_payload_node;

    try {
        conversation_payload_node = json::Builder{}
            .StartDict()
                .Key("client").Value(client_)
                .Key("name").Value(tag_)
                .Key("referenceTag").Value(tag_)
            .EndDict()
            .Build();
    } catch (const std::exception& e) {
        error_message = "JSON conversation build error: " + std::string(e.what());
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    } catch (...) {
        error_message = "JSON conversation build error";
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    }

    if (!getJSONstring(conversation_payload_node, json_body, error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "getJSONstring() failed: " + error_message);
        return false;
    }

    return true;
}

bool ConversationManager::getChatBody(std::string& json_body, const std::string& message, const std::string& searchedContext, const std::string& text, std::string chat_action, std::string& error_message) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getChatBody()");
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "searchedContext=" + searchedContext);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "text=" + searchedContext);    
    std::string id_field_name = "topicId";
    std::string id_field_value = topic_id_;
    bool stream = true;

    if (chat_action == "NewTopic") {
        id_field_name = "conversation";
        id_field_value = conversation_id_;
        stream = false;
    }

    json::Node chat_payload_node;

    try {
        json::Builder builder;
        auto root = builder.StartDict();

        // Required fields
        root.Key(id_field_name).Value(id_field_value);
        root.Key("stream").Value(stream);
        root.Key("client").Value(client_);
        root.Key("action").Value(chat_action);

        // conf block
        root.Key("conf").StartDict()
            .Key("aviatorModel").Value(aviator_model_)
            .Key("creationSources").StartArray()
                .Value("opsb")
                .Value("smax")
            .EndArray()
        .EndDict();

        // message block
        root.Key("message").StartDict()
            .Key("type").Value("text")
            .Key("content").Value(message)
        .EndDict();

        // Conditionally add contextProvider
        if (chat_action != "NewTopic") {
            if (!text.empty()) {
                root.Key("contextProvider").StartDict()
                    .Key("text").Value(text)
                .EndDict();
            }
            //  else if (!searchedContext.empty()) {
            //     root.Key("contextProvider").StartDict()
            //         .Key("searchedContext").Value(searchedContext)
            //     .EndDict();
            // }   
        }

        chat_payload_node = root.EndDict().Build();

    } catch (const std::exception& e) {
        error_message = "JSON build error for chat message: " + std::string(e.what());
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return false;
    }

    if (!getJSONstring(chat_payload_node, json_body, error_message)) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "getJSONstring() error :" + error_message);
        return false;
    }

    return true;
}

} // namespace smax