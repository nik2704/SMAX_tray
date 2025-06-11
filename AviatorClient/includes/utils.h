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

std::string generate_UUID_v4();

std::string GetCurrentTimestamp();

} // namespace smax