#pragma once

#include <atomic>
#include <optional>
#include <vector>
#include <mutex>
#include <string>
#include <imgui.h>

#include "MessageChat.h"
#include "Link.h"
#include "ChatController.h"

namespace smax {

class ChatController;

class ChatUI {
public:
    ChatUI(
        const std::string& host,
        const std::string& tenant_id,
        const std::string& client,
        const std::string& tag,
        const std::string & aviator_model,
        const std::string& username,
        const std::string& password,
        ImTextureID userAvatar,
        ImTextureID assistantAvatar,
        const std::string & min_log_level
    );

    void ImGuiSpacing(int count);
    void Render();

    void AddUserMessage(const std::string& text);
    void AddAssistantMessage(const std::string& text);
    void setLastMessageTextError(const std::string& text);

    void AppendAssistantMessageSlowly(const std::string& full_text);
    void updateLoadedMessages(const std::vector<MessageChat> & newLoadedMessages);
    void clearLinks();
    void addMandatoryLinks();
    void setLinks(const std::vector<smax::Link> &references);
    void addLink(const smax::Link & link);
    void switchOffLoadingFlag();

private:
    std::shared_ptr<ChatController> chat_controller_;
    std::vector<MessageChat> messages_;
    std::vector<Link> links_;
    Link smaxLink_;
    char inputBuffer_[2048]{};

    ImTextureID userAvatar_;
    ImTextureID assistantAvatar_;
    bool conversationHistoryLoaded_ = false;
    bool loading_ = false;
    std::vector<MessageChat> loadedMessages_;
    std::mutex messageMutex_;
    std::mutex topicMutex_;
    std::mutex contextMutex_;
    std::mutex linkMutex_;
    std::atomic<bool> needsUpdate_ {false};

    void SetupMainWindow();
    void StartBackgroundLoadingIfNeeded();
    void MergeLoadedMessages();
    void RenderMessagesRegion();
    void RenderMessage(const MessageChat& msg);
    void RenderChatPanel(float chatWidth);
    void RenderLinks(std::string sectionLabel, std::string linkType);
    void RenderLinkPanel(float linkPanelWidth);

    void RenderRestartButton();

    void RenderSendInput();
    void reset();
};

} // namespace smax