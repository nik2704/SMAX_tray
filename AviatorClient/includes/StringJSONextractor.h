/**
 * @file StringJSONExtractor.h
 * @brief Provides static utility functions to extract and parse conversation-related data from JSON.
 */

#pragma once

#include <memory>
#include <string>
#include "../../libs/JSONhelper/json.h"
#include "../../libs/JSONhelper/json_builder.h"
#include "../../libs/JSONhelper/JSONhelper.h"
#include "ConversationStructures.h"
#include "../../libs/Logger/Logger.h"
#include "MessageAPI.h"
#include "Link.h"
#include "utils.h"

namespace smax {

/**
 * @class StringJSONExtractor
 * @brief A collection of static methods to parse conversation and context information from JSON strings and objects.
 *
 * This class facilitates extracting messages, context, conversation IDs, topics, and references
 * from JSON payloads typically returned by the chat or conversation API.
 */
class StringJSONExtractor {
public:
    /// Pair consisting of a shared pointer to a vector of messages and a shared pointer to the last context.
    using MessageContextPair = std::pair<std::shared_ptr<std::vector<MessageAPI>>, std::shared_ptr<LastContext>>;

    /**
     * @brief Extracts a string parameter value by key from a JSON string.
     * 
     * @param json_body JSON string to parse.
     * @param key The key whose value is to be extracted.
     * @param error_message Reference to a string to receive any error messages.
     * @return Extracted string value if successful, empty string otherwise.
     */
    static std::string getStringParameter(const std::string& json_body,
                                          const std::string& key,
                                          std::string& error_message);

    /**
     * @brief Extracts conversation messages and last context from a JSON string.
     * 
     * @param json_str JSON string containing conversation data.
     * @param conversation_id Reference to string to receive the extracted conversation ID.
     * @param topic_id Reference to string to receive the extracted topic ID.
     * @return A MessageContextPair with extracted messages and context.
     */
    static MessageContextPair extractConversationFromJSON(const std::string& json_str,
                                                          std::string& conversation_id, 
                                                          std::string& topic_id);

    /**
     * @brief Extracts the last context information from a JSON dictionary object.
     * 
     * @param json_obj JSON dictionary containing the last context data.
     * @return Shared pointer to a LastContext structure representing the last conversation context.
     */
    static std::shared_ptr<LastContext> extractLastContextFromJSON(const json::Dict & json_obj);

    /**
     * @brief Parses context details from a JSON root dictionary.
     * 
     * Extracts message text, searched context, additional text, references, and error messages.
     * 
     * @param root JSON root dictionary object.
     * @param host Host address string.
     * @param tenant_id Tenant identifier string.
     * @param doc Shared pointer to the JSON document.
     * @param message Output parameter for the extracted message text.
     * @param searchedContext Output parameter for the searched context text.
     * @param text Output parameter for additional text.
     * @param references Output vector of extracted Link references.
     * @param error_message Output string for any error messages.
     */
    static void parseContext(
        const json::Dict &root,
        const std::string& host,
        const std::string& tenant_id,
        const std::shared_ptr<json::Document>& doc,
        std::string& message,
        std::string& searchedContext,
        std::string& text,
        std::vector<Link>& references,
        std::string& error_message    
    );

    /**
     * @brief Extracts chat content and context data from a JSON document.
     * 
     * @param host SMAX Host fqdn.
     * @param tenant_id SMAX Tenant ID.
     * @param doc Shared pointer to the JSON document.
     * @param message Output string for extracted message content.
     * @param searchedContext Output string for searched context.
     * @param text Output string for additional text.
     * @param references Output vector of extracted Link references.
     * @param error_message Output string for error details.
     * @return True if extraction was successful, false otherwise.
     */
    static bool ExtractContentFromJson(
        const std::string& host,
        const std::string& tenant_id,
        const std::shared_ptr<json::Document>& doc,
        std::string& message,
        std::string& searchedContext,
        std::string& text,
        std::vector<Link>& references,
        std::string& error_message
    );

    /**
     * @brief Extracts message and context information from a raw JSON buffer string.
     * 
     * @param host SMAX Host fqdn.
     * @param tenant_id SMAX Tenant ID.
     * @param buffer Raw JSON string buffer.
     * @param message Output string for extracted message content.
     * @param searchedContext Output string for searched context.
     * @param text Output string for additional text.
     * @param references Output vector of extracted Link references.
     * @param error_message Output string for any error messages.
     * @return True if extraction was successful, false otherwise.
     */
    static bool getMessageFromBuffer(
        const std::string& host,
        const std::string& tenant_id,
        const std::string& buffer,
        std::string& message,
        std::string& searchedContext,
        std::string& text,
        std::vector<Link>& references,
        std::string& error_message
    );    

private:
    /**
     * @brief Safely extracts a string value by key from a JSON dictionary.
     * 
     * @param obj JSON dictionary object.
     * @param key Key to look up in the dictionary.
     * @param result Output string where extracted value is stored.
     * @return True if key exists and value is successfully extracted, false otherwise.
     */
    static bool setStringSafe(const json::Dict& obj, const std::string& key, std::string& result);

    /**
     * @brief Safely extracts a non-empty string value by key from a JSON dictionary.
     * 
     * @param obj JSON dictionary object.
     * @param key Key to look up in the dictionary.
     * @param result Output string where extracted value is stored.
     * @return True if key exists and non-empty value is successfully extracted, false otherwise.
     */
    static bool setStringSafeNotEmpty(const json::Dict& obj, const std::string& key, std::string& result);
};

} // namespace smax
