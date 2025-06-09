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

class StringJSONExtractor {
public:
    using MessageContextPair = std::pair<std::shared_ptr<std::vector<MessageAPI>>, std::shared_ptr<LastContext>>;

    static std::string getStringParameter(const std::string& json_body,
                                          const std::string& key,
                                          std::string& error_message);

    static MessageContextPair extractConversationFromJSON(const std::string& json_str,
                                                   std::string& conversation_id, 
                                                   std::string& topic_id);

    static std::shared_ptr<LastContext> extractLastContextFromJSON(const json::Dict & json_obj);

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
    static bool setStringSafe(const json::Dict& obj, const std::string& key, std::string& result);
    static bool setStringSafeNotEmpty(const json::Dict& obj, const std::string& key, std::string& result);

};

} // namespace smax