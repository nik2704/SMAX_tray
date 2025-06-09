#include "ChatUI.h"
#include <ctime>
#include <optional>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>         // BEFORE shellapi.h
#include <shellapi.h>        // for ShellExecuteA
#include <random>
#include <sstream>
#include <iomanip>
// #include "utils.h"

namespace smax {

ChatUI::ChatUI(
    const std::string& host,
    const std::string& tenant_id,
    const std::string& client,
    const std::string& tag,
    const std::string& aviator_model,
    const std::string& username,
    const std::string& password,
    ImTextureID userAvatar,
    ImTextureID assistantAvatar,
    const std::string & min_log_level
) : userAvatar_(userAvatar), 
    assistantAvatar_(assistantAvatar)
    {
        chat_controller_ = std::make_shared<smax::ChatController>(
            *this, host, tenant_id, client, tag, aviator_model, username, password, min_log_level
        );

        smaxLink_ = {"1", "SMAX", "Self service", "https://" + host + "/saw/ess?TENANTID=" + tenant_id};
        
        addMandatoryLinks();
    }

void ChatUI::AddUserMessage(const std::string& text) {
    messages_.push_back({ generate_UUID_v4(), text, true, GetCurrentTimestamp() });
}

void ChatUI::AddAssistantMessage(const std::string& text) {
    messages_.push_back({ generate_UUID_v4(), text, false, GetCurrentTimestamp() });
}

void ChatUI::RenderMessage(const MessageChat& msg) {
    ImGui::PushID(msg.id.c_str());
    ImGui::BeginGroup();

    ImGui::Image(msg.from_user ? userAvatar_ : assistantAvatar_, ImVec2(32, 32));
    ImGui::SameLine();

    // Bubble background
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    float maxWidth = ImGui::GetWindowWidth() * (msg.from_user ? 0.6f : 0.85f);
    ImVec2 textSize = ImGui::CalcTextSize(msg.text.c_str(), nullptr, false, maxWidth);

    ImVec2 bubbleStart = cursor;
    ImVec2 bubbleEnd = ImVec2(cursor.x + textSize.x + 20, cursor.y + textSize.y + 20);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImU32 bubbleColor = msg.from_user ? IM_COL32(70, 130, 180, 255) : IM_COL32(240, 240, 240, 255);
    ImU32 textColor   = msg.from_user ? IM_COL32(255, 255, 255, 255) : IM_COL32(20, 20, 20, 255);

    drawList->AddRectFilled(bubbleStart, bubbleEnd, bubbleColor, 10.0f);

    ImGui::Dummy(ImVec2(10, 10));
    ImGui::SameLine();

    ImGui::PushTextWrapPos(bubbleStart.x + maxWidth);

    // Use a unique label with ## to avoid visual duplication
    ImGui::TextColored(ImColor(textColor), "%s", msg.text.c_str());

    // Right-click on this text shows popup
    if (ImGui::BeginPopupContextItem("msg_popup")) {
        if (ImGui::MenuItem("Copy")) {
            ImGui::SetClipboardText(msg.text.c_str());
        }
        ImGui::EndPopup();
    }

    ImGui::PopTextWrapPos();
    ImGui::TextDisabled("%s", msg.timestamp.c_str());
    ImGui::EndGroup();
    ImGui::Spacing();
    ImGui::PopID();
}

void ChatUI::ImGuiSpacing(int count) {
    for (int i = 0; i < count; ++i) {
        ImGui::Spacing();
    }
}

void ChatUI::RenderChatPanel(float chatWidth) {
    ImGui::BeginChild("ChatPanel", ImVec2(chatWidth, 0), false);
    RenderMessagesRegion();
    RenderSendInput();
    RenderRestartButton();
    ImGui::EndChild();
}

void ChatUI::RenderLinks(std::string sectionLabel, std::string linkType) {
    if (!std::any_of(links_.begin(), links_.end(), [&](const Link& link) {
        return link.type == linkType;
    })) {
        return;
    }

    ImGui::Text(sectionLabel.c_str());
    ImGui::Separator();

    for (const auto& link : links_) {
        if (link.type == linkType) {
            std::string display = link.label.empty() ? link.url : link.label;

            if (ImGui::Selectable(display.c_str())) {
                ShellExecuteA(nullptr, "open", link.url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
            }

            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::Selectable("Copy link to clipboard")) {
                    ImGui::SetClipboardText(link.url.c_str());
                }

                ImGui::EndPopup();
            }
        }
    }

    ImGuiSpacing(8);
}

void ChatUI::RenderLinkPanel(float linkPanelWidth) {
    if (!links_.empty()) {
        ImGui::SameLine();

        ImGui::BeginChild("LinkPanel", ImVec2(linkPanelWidth, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

        RenderLinks("SMAX:", "SMAX");
        RenderLinks("Internal Links:", "Article");
        RenderLinks("External Links:", "RemoteArticle");
        RenderLinks("News:", "News");
        RenderLinks("ServiceOfferings:", "ServiceOffering");
        RenderLinks("SupportOfferings:", "SupportOffering");

        ImGui::EndChild();
    }

}

void ChatUI::Render() {
    SetupMainWindow();

    StartBackgroundLoadingIfNeeded();
    MergeLoadedMessages();

    if (needsUpdate_) {
        ImGui::SetScrollHereY(1.0f);
        needsUpdate_ = false;
    }

    ImVec2 contentSize = ImGui::GetContentRegionAvail();
    float linkPanelWidth = links_.empty() ? 0.0f : 300.0f;
    float chatWidth = contentSize.x - linkPanelWidth - ImGui::GetStyle().ItemSpacing.x;

    RenderChatPanel(chatWidth);
    RenderLinkPanel(linkPanelWidth);

    ImGui::End();
}

void ChatUI::SetupMainWindow() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("Chat with Aviator", nullptr,
                 ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse);
}

void ChatUI::StartBackgroundLoadingIfNeeded() {
    if (!conversationHistoryLoaded_ && !loading_) {
        loading_ = true;
        std::thread([this]() {

            chat_controller_->loadConversation();

            {
                std::lock_guard<std::mutex> lock(messageMutex_);
                conversationHistoryLoaded_ = true;
            }

            switchOffLoadingFlag();

        }).detach();
    }
}

void ChatUI::MergeLoadedMessages() {
    if (conversationHistoryLoaded_) {
        std::lock_guard<std::mutex> lock(messageMutex_);
        for (const auto& msg : loadedMessages_) {
            messages_.push_back(msg);
        }
        loadedMessages_.clear();
    }
}

void ChatUI::RenderMessagesRegion() {
    float inputHeight = 100.0f + ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeight();
    ImVec2 available = ImGui::GetContentRegionAvail();
    ImVec2 scrollSize = ImVec2(available.x, available.y - inputHeight - 10);

    ImGui::BeginChild("ScrollRegion", scrollSize, true);

    {
        std::lock_guard<std::mutex> lock(messageMutex_);
        for (const auto& msg : messages_) {
            RenderMessage(msg);
        }
    }

    if (loading_) {
        ImGui::TextColored(ImColor(ImU32(IM_COL32(150, 150, 150, 255))), "loading...");
    }

    ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();
}

void ChatUI::AppendAssistantMessageSlowly(const std::string& full_text) {
    std::thread([this, full_text]() {
        std::string current_text;

        for (char c : full_text) {
            {
                std::lock_guard<std::mutex> lock(messageMutex_);
                current_text += c;
                if (!messages_.empty()) {
                    messages_.back().text = current_text;
                    needsUpdate_ = true;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        loading_ = false;
    }).detach();

}

void ChatUI::RenderSendInput() {
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;
    ImGui::InputTextMultiline("##chat_input", inputBuffer_, sizeof(inputBuffer_), ImVec2(-1, 100), flags);

    if (loading_) {
        ImGui::BeginDisabled();
    }

    if ((ImGui::Button("Send") || (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))) 
        && strlen(inputBuffer_) > 0 && !loading_) {

        std::string user_input = inputBuffer_;

        {
            std::lock_guard<std::mutex> lock(messageMutex_);
            AddUserMessage(user_input);
            messages_.push_back({ generate_UUID_v4(), "", false, GetCurrentTimestamp() });
            loading_ = true;
        }

        inputBuffer_[0] = '\0';

        chat_controller_->sendChatMessage(user_input);
    }

    if (loading_) {
        ImGui::EndDisabled();
    }
}

void ChatUI::RenderRestartButton() {
    ImGui::SameLine();

    if (ImGui::Button("Restart conversation")) {
        ImGui::OpenPopup("ConfirmRestart");
    }

    if (ImGui::BeginPopupModal("ConfirmRestart", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to restart the conversation?");
        ImGui::Separator();

        if (ImGui::Button("Yes", ImVec2(120, 0))) {
            reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("No", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ChatUI::reset() {
    messages_.clear();
    inputBuffer_[0] = '\0';

    chat_controller_->resetConversation();

    {
        std::lock_guard<std::mutex> lock(messageMutex_);
        loadedMessages_.clear();
        conversationHistoryLoaded_ = false;

        links_.clear();
        links_.push_back(smaxLink_);

    }

    loading_ = false;
}

void ChatUI::setLastMessageTextError(const std::string& text) {
    {
        std::lock_guard<std::mutex> lock(messageMutex_);

        if (messages_.empty()) {
            messages_.back().text = "Error: " + text;
        }
    }
}

void ChatUI::switchOffLoadingFlag() {
    loading_ = false;
}

void ChatUI::updateLoadedMessages(const std::vector<MessageChat> & newLoadedMessages) {
    std::lock_guard<std::mutex> lock(messageMutex_);
    loadedMessages_ = newLoadedMessages;
    conversationHistoryLoaded_ = true;
}

void ChatUI::clearLinks() {
    std::lock_guard<std::mutex> lock(linkMutex_);
    links_.clear();
}

void ChatUI::addMandatoryLinks(){
    links_.push_back(smaxLink_);
}

void ChatUI::setLinks(const std::vector<smax::Link> &references) {
    std::lock_guard<std::mutex> lock(linkMutex_);
    links_ = references;
}

void ChatUI::addLink(const smax::Link & link) {
    std::lock_guard<std::mutex> lock(linkMutex_);
    links_.push_back(link);
}

} // namespace smax