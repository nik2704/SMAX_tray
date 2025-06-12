#include "ChatController.h"
#include <ctime>
#include <optional>
#include "utils.h"

namespace smax {

ChatController::ChatController(
    ChatUI& ui,
    const std::string& host,
    const std::string& tenant_id,
    const std::string& client,
    const std::string& tag,
    const std::string& aviator_model,
    const std::string& username,
    const std::string& password,
    const std::string& min_log_level
)
    : chatUI_(ui)
    , host_(host)
    , tenant_id_(tenant_id)
    , client_(client)
    , tag_(tag)
    , aviator_model_(aviator_model)
    , user_name_(username)
    , password_(password)
    , min_log_level_(min_log_level),
    conversationManager_(
        host_,
        tenant_id_,
        client_,
        tag_,
        aviator_model_,
        user_name_,
        password_,
        min_log_level_
    )
    , isChatSending_(false)
{ }

void ChatController::loadConversation() {
    std::string error_message;
    auto [messages, context] = conversationManager_.setUpConversation(error_message);

    fillOutLinks(context);

    if (context) {
        searchedContext_ = context->searchedContext;
        last_message_ = context->text;
    }

    std::vector<MessageChat> temp;
    if (messages->rbegin() != messages->rend()) {
        if (messages) {
            for (auto it = messages->rbegin(); it != messages->rend(); ++it) {
                const auto& msg = *it;
                topic_id_ = msg.topic_id;

                temp.push_back({
                    msg.id.empty() ? generate_UUID_v4() : msg.id,
                    msg.content,
                    msg.role == "User",
                    GetCurrentTimestamp()
                });
            }
        } else {
            temp.push_back({ generate_UUID_v4(), "Failed to parse conversation: " + error_message, false, GetCurrentTimestamp() });
        }
    }

    chatUI_.updateLoadedMessages(temp);
}

void ChatController::resetConversation() {
    inputBuffer_[0] = '\0';

    {
        std::lock_guard<std::mutex> lock(chatMutex_);

        std::string error;
        conversationManager_.deleteConversation(error);

        searchedContext_.clear();
        last_message_.clear();
    }

    topic_id_ = "";
}

void ChatController::sendChatMessage(const std::string& message) {
    bool expected = false;
    if (isChatSending_.compare_exchange_strong(expected, true)) {
        // CRITICAL CHANGE: Capture a shared_ptr to 'this' to manage lifetime
        std::shared_ptr<ChatController> self = shared_from_this();

        std::thread([self, message]() {
            std::string char_buffer;
            std::string error;
            std::vector<Link> references;

            // All accesses to ChatController members MUST now use 'self->'
            bool success = self->conversationManager_.chat(
                message,
                self->searchedContext_,
                self->last_message_,
                error,
                [&](const std::string& raw_data_block) {
                    // This inner lambda also implicitly captures 'self' because it uses
                    // self->chatMutex_ and self->chatUI_. This is safe.
                    std::lock_guard<std::mutex> lock(self->chatMutex_);
                    std::string chunk;
                    std::string error_message;

                    if (self->chatUI_.isShuttingDown()) return; // Use self->

                    auto data_extracted = StringJSONExtractor::getMessageFromBuffer(
                        self->host_,           // Use self->
                        self->tenant_id_,      // Use self->
                        raw_data_block,
                        chunk,
                        self->searchedContext_, // Use self->
                        self->last_message_,    // Use self->
                        references,
                        error_message
                    );

                    if (data_extracted) {
                        char_buffer += chunk;
                    }
                }
            );

            if (self->chatUI_.isShuttingDown()) return; // Use self->

            if (!char_buffer.empty()) {
                self->chatUI_.AppendAssistantMessageSlowly(char_buffer); // Use self->

                self->fillOutLinks(references); // Use self->
            } else {
                self->chatUI_.switchOffLoadingFlag(); // Use self->
            }

            if (!success) {
                self->chatUI_.setLastMessageTextError(error); // Use self->
            }

            self->isChatSending_.store(false);
        }).detach();
    }
}

void ChatController::fillOutLinks(const std::vector<smax::Link> & references) {
    chatUI_.clearLinks();

    if (!references.empty()) {
        chatUI_.setLinks(references);
    }

    chatUI_.addMandatoryLinks();
}

void ChatController::fillOutLinks(const std::shared_ptr<smax::LastContext> &context) {
    chatUI_.clearLinks();

    if (!context->referencedDocs.empty()) {
        for (const auto doc : context->referencedDocs) {
            Link link = { doc.id, doc.type, doc.label, doc.url };
            chatUI_.addLink(link);
        }
    }

    chatUI_.addMandatoryLinks();
}

} // namespace smax
