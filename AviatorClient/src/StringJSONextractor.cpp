#include "StringJSONextractor.h"

namespace smax {


bool StringJSONExtractor::setStringSafe(const json::Dict& obj, const std::string& key, std::string& result) {
    auto it = obj.find(key);

    if (it != obj.end() && it->second.IsString()) {
        result = it->second.AsString();
        return true;
    }

    result = "";
    return false;
}

bool StringJSONExtractor::setStringSafeNotEmpty(const json::Dict& obj, const std::string& key, std::string& result) {
    std::string temp;
    
    if (setStringSafe(obj, key, temp) && !temp.empty()) {
        result = std::move(temp);
        return true;
    }

    return false;
}

std::string StringJSONExtractor::getStringParameter(const std::string& json_body,
                                                    const std::string& key,
                                                    std::string& error_message)
{
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "getParameterFromJSON()");
    auto doc = JSONHelper::ParseJSONFromString(json_body);
    if (!doc) {
        error_message = "Error: Failed to parse JSON string.";
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
        return "";
    }

    try {
        const json::Node& root_node = doc->GetRoot();
        if (!root_node.IsMap()) return "";

        const auto& root = root_node.AsMap();
        return root.at(key).AsString();
    } catch (const std::exception& ex) {
       error_message = "Error extracting parameter '" + key + "': " + ex.what();
       AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, error_message);
    }

    return "";
}

StringJSONExtractor::MessageContextPair StringJSONExtractor::extractConversationFromJSON(const std::string& json_str, std::string& conversation_id, std::string& topic_id) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "extractConversationFromJSON()");
    auto messages = std::make_shared<std::vector<MessageAPI>>();
    auto last_context = std::make_shared<LastContext>();

    auto doc = JSONHelper::ParseJSONFromString(json_str);
    conversation_id = "";
    // topic_id_ = "";

    if (!doc) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Error: Failed to parse JSON string.");
        return {messages, last_context};
    }

    try {
        const json::Node& root_node = doc->GetRoot();
        if (!root_node.IsMap()) return {messages, last_context};

        const auto& root = root_node.AsMap();
        // conversation_id = root.at("id").AsString();

        setStringSafeNotEmpty(root, "id", conversation_id);
        // topic_id_ = "";

        auto messages_it = root.find("messages");
        if (messages_it == root.end() || !messages_it->second.IsArray()) return {messages, last_context};

        const auto& json_messages = messages_it->second.AsArray();

        for (const auto& msg_node : json_messages) {
            if (!msg_node.IsMap()) continue;

            const auto& msg_map = msg_node.AsMap();
            MessageAPI msg;

            setStringSafe(msg_map, "id", msg.id);
            setStringSafeNotEmpty(msg_map, "topicId", topic_id);
            setStringSafe(msg_map, "role", msg.role);
            setStringSafe(msg_map, "content", msg.content);

            msg.topic_id = topic_id;
            messages->push_back(std::move(msg));
        }

        auto lastContext_it = root.find("lastContext");
        if (lastContext_it != root.end() && lastContext_it->second.IsMap()) {
            auto lastContext = root.at("lastContext").AsMap();
            last_context = StringJSONExtractor::extractLastContextFromJSON(lastContext);
        }

    } catch (const std::exception& ex) {
        AppLogger::Logger::getInstance().log(AppLogger::LOG_ERROR, "Error extracting messages: " + std::string(ex.what()));
    }

    return {messages, last_context};
}

std::shared_ptr<LastContext> StringJSONExtractor::extractLastContextFromJSON(const json::Dict & json_obj) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "extractLastContextFromJSON()");
    auto last_context = std::make_shared<LastContext>();

    setStringSafeNotEmpty(json_obj, "text", last_context->text);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "Context text: " + last_context->text);

    setStringSafeNotEmpty(json_obj, "searchedContext", last_context->searchedContext);
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "searchedContext: " + last_context->searchedContext);

    // The next code: it is IMPOSSIBLE to get URL and LABEL using current API (will be used in future)
    // auto referencedDocs_it = json_obj.find("referencedDocs");
    // if (referencedDocs_it == json_obj.end() || !referencedDocs_it->second.IsArray()) return last_context;
    // AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "referencedDocs are found");

    // const auto& referencedDocs = referencedDocs_it->second.AsArray();

    // for (const auto& doc_node : referencedDocs) {
    //     if (!doc_node.IsMap()) continue;

    //     const auto& doc_map = doc_node.AsMap();
    //     ContextDoc context_doc;

    //     setStringSafeNotEmpty(doc_map, "id", context_doc.id);
    //     setStringSafeNotEmpty(doc_map, "type", context_doc.type);
    //     setStringSafeNotEmpty(doc_map, "label", context_doc.label);
    //     setStringSafeNotEmpty(doc_map, "url", context_doc.url);

    //     AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "---> context_doc.id: " + context_doc.id);
    //     AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "---> context_doc.type: " + context_doc.type);
    //     AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "---> context_doc.label: " + context_doc.label);
    //     AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "---> context_doc.url: " + context_doc.url);
        
    //     last_context->referencedDocs.push_back(std::move(context_doc));
    // }

    return last_context;
}

void StringJSONExtractor::parseContext(
    const json::Dict &root,
    const std::string& host,
    const std::string& tenant_id,
    const std::shared_ptr<json::Document>& doc,
    std::string& message,
    std::string& searchedContext,
    std::string& text,
    std::vector<Link>& references,
    std::string& error_message    
) {
    AppLogger::Logger::getInstance().log(AppLogger::LOG_DEBUG, "parseContext()");

    try {
        std::string temp_context;
        std::string temp_text;

        setStringSafe(root, "searchedContext", temp_context);

        if (!temp_context.empty()) {
            temp_text = getStringParameter(temp_context, "text", error_message);
            if (!temp_text.empty()) {
                text = temp_text;
            }

            searchedContext = temp_context;
        }

        auto references_it = root.find("references"); // root, not msg_obj!
        if (references_it != root.end() && references_it->second.IsArray()) {
            const auto& references_docs = references_it->second.AsArray();
            for (const auto& doc_node : references_docs) {
                if (!doc_node.IsMap()) continue;

                const auto& doc_map = doc_node.AsMap();
                Link link;

                setStringSafe(doc_map, "DocId", link.id);
                setStringSafe(doc_map, "Title", link.label);
                setStringSafe(doc_map, "DocType", link.type);
                setStringSafe(doc_map, "Url", link.url);
                
                link.url = getESSUrl(
                    host,
                    tenant_id,
                    link.id,
                    link.url,
                    link.type
                );

                references.push_back(std::move(link));
            }
        }
    } catch (const std::exception& ex) {
        error_message = "Exception while parsing context: " + std::string(ex.what());
    }
}

bool StringJSONExtractor::ExtractContentFromJson(
    const std::string& host,
    const std::string& tenant_id,
    const std::shared_ptr<json::Document>& doc,
    std::string& message,
    std::string& searchedContext,
    std::string& text,
    std::vector<Link>& references,
    std::string& error_message
) {
    try {
        const json::Node& root_node = doc->GetRoot();
        if (!root_node.IsMap()) {
            error_message = "Root is not a JSON object.";
            return false;
        }

        const auto& root = root_node.AsMap();

        auto message_it = root.find("message");
        if (message_it == root.end() || !message_it->second.IsMap()) {
            error_message = "Missing or invalid 'message' object.";
            return false;
        }

        const auto& msg_obj = message_it->second.AsMap();

        auto message_found = setStringSafe(msg_obj, "content", message);
        if (!message_found) {
            error_message = "Missing or invalid 'content' string.";
            return false;
        }

        parseContext(
            root,
            host,
            tenant_id,
            doc,
            message,
            searchedContext,
            text,
            references,
            error_message
        );
    } catch (const std::exception& ex) {
        error_message = "Exception: " + std::string(ex.what());
        return false;
    }

    return true;
}

bool StringJSONExtractor::getMessageFromBuffer(
    const std::string& host,
    const std::string& tenant_id,
    const std::string& buffer,
    std::string& message,
    std::string& searchedContext,
    std::string& text,
    std::vector<Link>& references,
    std::string& error_message
) {
    std::string extracted_data = ExtractDataFromBuffer(buffer);
    auto doc = JSONHelper::ParseJSONFromString(extracted_data);
    if (!doc) {
        error_message = "Failed to parse JSON.";
        return false;
    }

    return ExtractContentFromJson(host, tenant_id, doc, message, searchedContext, text, references, error_message);
}

} // namespace smax