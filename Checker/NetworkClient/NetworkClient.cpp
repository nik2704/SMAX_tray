#include "NetworkClient.h"
#include <windows.h>
#include <wininet.h>
#include <wincrypt.h>
#include <sstream>
#include <iostream>
#include <fstream>

#pragma comment(lib, "wininet.lib")

namespace smax {

std::string base64Encode(const std::string& input) {
    DWORD outputLength = 0;

    if (!CryptBinaryToStringA(reinterpret_cast<const BYTE*>(input.data()),
                              static_cast<DWORD>(input.size()),
                              CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                              NULL,
                              &outputLength)) {
        return {};
    }

    std::string encoded(outputLength, '\0');
    if (!CryptBinaryToStringA(reinterpret_cast<const BYTE*>(input.data()),
                              static_cast<DWORD>(input.size()),
                              CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF,
                              &encoded[0],
                              &outputLength)) {
        return {};
    }

    if (!encoded.empty() && encoded.back() == '\0') {
        encoded.pop_back();
    }

    return encoded;
}

std::optional<std::string> NetworkClient::get(const std::string& url, const std::string& username, const std::string& password) {
    std::string credentials = username + ":" + password;
    std::string encodedAuth = base64Encode(credentials);
    std::string headers = "Authorization: Basic " + encodedAuth + "\r\n"
                          "Content-Type: application/json\r\n";

    URL_COMPONENTSA urlComp = {};
    char hostName[256], urlPath[1024];
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath);

    if (!InternetCrackUrlA(url.c_str(), 0, 0, &urlComp)) {
        std::cerr << "InternetCrackUrlA failed\n";
        return std::nullopt;
    }

    HINTERNET hInternet = InternetOpenA("TrayApp", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        std::cerr << "InternetOpenA failed\n";
        return std::nullopt;
    }

    HINTERNET hConnect = InternetConnectA(
        hInternet,
        urlComp.lpszHostName,
        urlComp.nPort,
        NULL, NULL,
        INTERNET_SERVICE_HTTP,
        0,
        0
    );
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        std::cerr << "InternetConnectA failed\n";
        return std::nullopt;
    }

    const char* acceptTypes[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequestA(
        hConnect,
        "GET",
        urlComp.lpszUrlPath,
        NULL,
        NULL,
        acceptTypes,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE,
        0
    );
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        std::cerr << "HttpOpenRequestA failed\n";
        return std::nullopt;
    }

    if (!HttpSendRequestA(hRequest, headers.c_str(), static_cast<DWORD>(headers.length()), NULL, 0)) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        std::cerr << "HttpSendRequestA failed\n";
        return std::nullopt;
    }

    // 🟢 Query status code
    DWORD statusCode = 0;
    DWORD length = sizeof(statusCode);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL)) {
        std::cerr << "HttpQueryInfoA failed\n";
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return std::nullopt;
    }

    // ❌ Not 200 — return no result
    if (statusCode != 200) {
        std::cerr << "HTTP Status: " << statusCode << "\n";
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return std::nullopt;
    }

    // ✅ Read response
    char buffer[4096];
    DWORD bytesRead;
    std::ostringstream oss;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
        oss.write(buffer, bytesRead);
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return oss.str();
}

std::vector<std::string> NetworkClient::extractIDsFromJSON(const std::string& json_str) {
    std::vector<std::string> ids;    

    auto doc = JSONHelper::ParseJSONFromString(json_str);

    if (!doc) {
        std::cerr << "Error: Failed to parse JSON string." << std::endl;
        return ids;
    }

    try {
        const json::Node& root_node = doc->GetRoot();
        if (!root_node.IsMap()) return ids;

        const auto& root = root_node.AsMap();
        auto entities_it = root.find("entities");

        if (entities_it == root.end() || !entities_it->second.IsArray()) return ids;

        const auto& entities = entities_it->second.AsArray();

        for (const auto& entity_val : entities) {
            if (!entity_val.IsMap()) continue;

            const auto& entity = entity_val.AsMap();
            auto props_it = entity.find("properties");
            if (props_it == entity.end() || !props_it->second.IsMap()) continue;

            const auto& props = props_it->second.AsMap();
            auto id_it = props.find("Id");
            if (id_it != props.end()) {
                const auto& id_val = id_it->second;
                if (id_val.IsString()) {
                    ids.push_back(id_val.AsString());
                } else if (id_val.IsInt() || id_val.IsDouble()) {
                    std::ostringstream oss;
                    ids.push_back(oss.str());
                }
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error extracting IDs: " << ex.what() << std::endl;
    }

    return ids;
}

} // namespace smax