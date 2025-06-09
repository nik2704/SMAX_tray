#pragma once
#include <algorithm> // for std::transform
#include <cctype>
#include <sstream>
#include <string>
#include <stdexcept>
#include "../../libs/JSONhelper/json.h"
#include "../../libs/JSONhelper/json_builder.h"
#include "../../libs/JSONhelper/JSONhelper.h"
#include "Link.h"


namespace smax {

bool containsStringIgnoreCase(const std::string& originalString, const std::string& substringToFind);

bool TrimLine(std::string& line);

std::string ExtractDataFromBuffer(const std::string& buffer);

std::string getESSUrl(const std::string& host, const std::string& tenant_id, const std::string& doc_id, const std::string& url, const std::string& type);

// void parseContext(const json::Dict &root, const std::string& host, const std::string& tenant_id, 
//     const std::shared_ptr<json::Document>& doc, std::string& message,
//     std::string& searchedContext, std::string& text, std::vector<Link>& references, std::string& error_message);

// std::string extractFieldFromStringJSON(const std::string& json_string, const std::string& field_name);

// bool ExtractContentFromJson(const std::string& host, const std::string& tenant_id, 
//     const std::shared_ptr<json::Document>& doc, std::string& message,
//     std::string& searchedContext, std::string& text, std::vector<Link>& references, std::string& error_message);

// bool tryExtractMessageFromBuffer(
//     const std::string& host,
//     const std::string& tenant_id,
//     const std::string& buffer,
//     std::string& message,
//     std::string& searchedContext,
//     std::string& text,
//     std::vector<Link>& references,
//     std::string& error_message
// );

std::string generate_UUID_v4();

std::string GetCurrentTimestamp();

} // namespace smax