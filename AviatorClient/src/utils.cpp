#include <random>
#include <sstream>
#include <iomanip>

#include "utils.h"
#include "../../libs/Logger/Logger.h"

namespace smax {

bool containsStringIgnoreCase(const std::string& originalString, const std::string& substringToFind) {
    if (substringToFind.empty()) {
        return false;
    }

    std::string lowerType = originalString;
    std::string lowerTarget = substringToFind;

    std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    std::transform(lowerTarget.begin(), lowerTarget.end(), lowerTarget.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    return lowerType.find(lowerTarget) != std::string::npos;
}

bool TrimLine(std::string& line) {
    size_t start = line.find_first_not_of(" \t\r\n");
    size_t end = line.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos) {
        line.clear();
        return false;
    }
    line = line.substr(start, end - start + 1);
    return true;
}

std::string ExtractDataFromBuffer(const std::string& buffer) {
    std::istringstream iss(buffer);
    std::string extracted_data;
    std::string line;

    while (std::getline(iss, line)) {
        if (!TrimLine(line)) continue;
        if (line.rfind("data:", 0) == 0) {
            if (!extracted_data.empty()) {
                extracted_data += "\n";
            }
            extracted_data += line.substr(5);
        }
    }
    return extracted_data;
}

std::string getESSUrl(const std::string& host, const std::string& tenant_id, const std::string& doc_id, const std::string& url, const std::string& type) {
    if (url.empty() & !doc_id.empty()) {
        if (containsStringIgnoreCase(type, "remotearticle")) {
            return url;
        } else if (containsStringIgnoreCase(type, "article")) {
            return "https://" + host + "/saw/ess/viewResult/" + doc_id + "?TENANTID=" + tenant_id;
        } else if (containsStringIgnoreCase(type, "offering")) {
            return "https://" + host + "/saw/ess/offeringPage/" + doc_id + "?TENANTID=" + tenant_id;
        } else if (containsStringIgnoreCase(type, "news")) {
            return "https://" + host + "/saw/ess/viewResult/" + doc_id + "?TENANTID=" + tenant_id;
        }
    }

    return url;
}  

std::string generate_UUID_v4() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << dis(gen);
    ss << "-4";
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    ss << dis2(gen);
    for (int i = 0; i < 3; ++i) ss << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << dis(gen);

    return ss.str();
}

std::string GetCurrentTimestamp() {
    time_t now = time(nullptr);
    tm local;
    // Use localtime_s: &local is the output, now is input
    localtime_s(&local, &now);

    char buf[64];
    strftime(buf, sizeof(buf), "%H:%M", &local);
    return std::string(buf);
}

} // namespace smax