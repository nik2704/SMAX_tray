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

// Forward declaration of ChatController to avoid circular dependency
class ChatController;

/**
 * @class ChatUI
 * @brief Handles all ImGui-based rendering and user interaction for the chat interface.
 *
 * Responsible for rendering the chat panel, processing input, displaying messages,
 * managing links, and integrating with ChatController for backend communication.
 */
class ChatUI {
public:
    /**
     * @brief Constructs the Chat UI with configuration and resource references.
     *
     * @param host SMAX host address.
     * @param tenant_id SMAX Tenant ID.
     * @param client Client (Aviator API)
     * @param tag Tag (Aviator API).
     * @param aviator_model ID of the Aviator model to use.
     * @param username User's login name.
     * @param password User's password.
     * @param userAvatar Avatar image used for user messages.
     * @param assistantAvatar Avatar image used for assistant messages.
     * @param min_log_level Minimum log level for diagnostics/log filtering.
     */
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

    /**
     * @brief Inserts vertical spacing between ImGui elements.
     *
     * @param count Number of vertical spacings to insert.
     */
    void ImGuiSpacing(int count);

    /**
     * @brief Renders the entire chat interface.
     *
     * Should be called on each frame inside the main application loop.
     */
    void Render();

    /**
     * @brief Adds a user message to the chat.
     *
     * @param text Message text.
     */
    void AddUserMessage(const std::string& text);

    /**
     * @brief Adds an assistant message to the chat.
     *
     * @param text Message text.
     */
    void AddAssistantMessage(const std::string& text);

    /**
     * @brief Displays an error message related to the last sent user input.
     *
     * @param text Error message text.
     */
    void setLastMessageTextError(const std::string& text);

    /**
     * @brief Appends assistant text with simulated typing effect.
     *
     * @param full_text The full message to be revealed incrementally.
     */
    void AppendAssistantMessageSlowly(const std::string& full_text);

    /**
     * @brief Replaces the current message list with previously loaded messages.
     *
     * @param newLoadedMessages List of messages loaded from storage or backend.
     */
    void updateLoadedMessages(const std::vector<MessageChat>& newLoadedMessages);

    /**
     * @brief Clears all current contextual links.
     */
    void clearLinks();

    /**
     * @brief Adds default/mandatory links to the link panel.
     */
    void addMandatoryLinks();

    /**
     * @brief Replaces the current list of links.
     *
     * @param references List of new reference links.
     */
    void setLinks(const std::vector<smax::Link>& references);

    /**
     * @brief Adds a single link to the current list.
     *
     * @param link Link object to be added.
     */
    void addLink(const smax::Link& link);

    /**
     * @brief Signals that background loading is complete.
     *
     * Prevents the loading spinner from being shown further.
     */
    void switchOffLoadingFlag();

private:
    std::shared_ptr<ChatController> chat_controller_;  ///< Controller to manage backend logic.
    std::vector<MessageChat> messages_;                ///< Current chat message history.
    std::vector<Link> links_;                          ///< List of contextual resource links.
    Link smaxLink_;                                    ///< A default SMAX reference link.
    char inputBuffer_[2048]{};                         ///< Input buffer for typing user messages.

    ImTextureID userAvatar_;                           ///< Avatar texture for user.
    ImTextureID assistantAvatar_;                      ///< Avatar texture for assistant.
    bool conversationHistoryLoaded_ = false;           ///< Indicates if history was already loaded.
    bool loading_ = false;                             ///< Flag for UI loading state.
    std::vector<MessageChat> loadedMessages_;          ///< Temporarily loaded messages for merging.

    std::mutex messageMutex_;                          ///< Mutex for safe message operations.
    std::mutex topicMutex_;                            ///< Mutex for topic-specific operations.
    std::mutex contextMutex_;                          ///< Mutex for backend context operations.
    std::mutex linkMutex_;                             ///< Mutex for concurrent link updates.

    std::atomic<bool> needsUpdate_ {false};            ///< Indicates if the UI needs re-rendering.

    /**
     * @brief Initializes the layout and state of the main window.
     */
    void SetupMainWindow();

    /**
     * @brief Starts a background thread to load messages, if not already loaded.
     */
    void StartBackgroundLoadingIfNeeded();

    /**
     * @brief Merges loaded messages into the main message list.
     */
    void MergeLoadedMessages();

    /**
     * @brief Renders the message display area of the chat.
     */
    void RenderMessagesRegion();

    /**
     * @brief Renders a single message.
     *
     * @param msg The message to be rendered.
     */
    void RenderMessage(const MessageChat& msg);

    /**
     * @brief Renders the main chat panel (text + buttons).
     *
     * @param chatWidth Width allocated for chat UI.
     */
    void RenderChatPanel(float chatWidth);

    /**
     * @brief Renders a group of links for a specific section/type.
     *
     * @param sectionLabel Label for the section.
     * @param linkType Link category (used for filtering or styling).
     */
    void RenderLinks(std::string sectionLabel, std::string linkType);

    /**
     * @brief Renders the side panel with resource/reference links.
     *
     * @param linkPanelWidth Width allocated for link panel.
     */
    void RenderLinkPanel(float linkPanelWidth);

    /**
     * @brief Renders the restart conversation button.
     */
    void RenderRestartButton();

    /**
     * @brief Renders the input field and send button for sending messages.
     */
    void RenderSendInput();

    /**
     * @brief Clears the current chat session, messages, and links.
     */
    void reset();
};

} // namespace smax
