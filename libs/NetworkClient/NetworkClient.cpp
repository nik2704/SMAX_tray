#include "NetworkClient.h"
#include <wincrypt.h>
#include <sstream>

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

RequestResult NetworkClient::sendStandardRequest(HINTERNET hRequest) {
    RequestResult response;

    DWORD statusCode = 0;
    DWORD length = sizeof(statusCode);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL)) {
        response.error_message = "HttpQueryInfoA failed";
        return response;
    }

    response.code = static_cast<int>(statusCode);

    char buffer[4096];
    DWORD bytesRead = 0;
    std::ostringstream oss;

    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        oss.write(buffer, bytesRead);
    }

    response.result = oss.str();

    if (statusCode < 200 || statusCode >= 300) {
        std::ostringstream err;
        err << "HTTP Error: Status Code " << statusCode;
        response.error_message = err.str();
    }

    return response;
}

RequestResult NetworkClient::sendStreamingRequest(
    HINTERNET hRequest,
    std::function<void(const std::string&)> onChunk
) {

    RequestResult response;

    DWORD statusCode = 0;
    DWORD length = sizeof(statusCode);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL)) {
        response.error_message = "HttpQueryInfoA failed";
        return response;
    }

    response.code = static_cast<int>(statusCode);

    char buffer[4096];
    DWORD bytesRead = 0;
    std::string incomplete_data_buffer;
    
    while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        incomplete_data_buffer.append(buffer, bytesRead);

        size_t pos;

        while ((pos = incomplete_data_buffer.find("\n\n")) != std::string::npos ||
               (pos = incomplete_data_buffer.find("\r\n\r\n")) != std::string::npos) {

            size_t delimiter_length = (incomplete_data_buffer[pos + 1] == '\n' ? 2 : 4);

            std::string raw_message_block = incomplete_data_buffer.substr(0, pos);
            incomplete_data_buffer.erase(0, pos + delimiter_length);

            // std::string extracted_data;
            // extractMessageFromChunk(raw_message_block, extracted_data, response.error_message);

            // if (!extracted_data.empty() && onChunk) {
                onChunk(raw_message_block);
            // }
        }
    }

    if (statusCode < 200 || statusCode >= 300) {
        std::ostringstream err;
        err << "HTTP Error: Status Code " << statusCode;
        response.error_message = err.str();
    }

    return response;
}

RequestResult NetworkClient::sendRequest(const std::string& method,
                                         const std::string& url,
                                         const std::string& username,
                                         const std::string& password,
                                         const std::string& body,
                                         bool stream,
                                         std::function<void(const std::string&)> onChunk
) {
    RequestResult response;

    // Construct headers
    std::string headers = "Authorization: Basic " + base64Encode(username + ":" + password) + "\r\n"
                          "User-Agent: Apache-HttpClient/4.1\r\n"
                          "Content-Type: application/json\r\n"
                          "Accept: " + std::string(stream ? "text/event-stream" : "application/json") + "\r\n";

    if (stream) {
        headers += "Cache-Control: no-cache\r\nConnection: keep-alive\r\n";
    }

    // Parse URL
    URL_COMPONENTSA urlComp = {};
    char hostName[256], urlPath[1024];
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath);

    if (!InternetCrackUrlA(url.c_str(), 0, 0, &urlComp)) {
        response.error_message = "InternetCrackUrlA failed for URL: " + url;
        return response;
    }

    // Open Internet session
    HINTERNET hInternet = InternetOpenA("TrayApp", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        response.error_message = "InternetOpenA failed";
        return response;
    }

    auto closeInternetHandle = [](HINTERNET handle) {
        if (handle) InternetCloseHandle(handle);
    };

    HINTERNET hConnect = InternetConnectA(hInternet, urlComp.lpszHostName, urlComp.nPort,
                                          NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        closeInternetHandle(hInternet);
        response.error_message = "InternetConnectA failed to " + std::string(urlComp.lpszHostName);
        return response;
    }

    const char* acceptTypes[] = { stream ? "text/event-stream" : "application/json", NULL };
    DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_KEEP_CONNECTION;
    if (_strnicmp(url.c_str(), "https://", 8) == 0) {
        flags |= INTERNET_FLAG_SECURE;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, method.c_str(), urlComp.lpszUrlPath,
                                          NULL, NULL, acceptTypes, flags, 0);
    if (!hRequest) {
        closeInternetHandle(hConnect);
        closeInternetHandle(hInternet);
        response.error_message = "HttpOpenRequestA failed";
        return response;
    }

    DWORD timeoutMs = 60000;
    InternetSetOptionA(hRequest, INTERNET_OPTION_CONNECT_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
    InternetSetOptionA(hRequest, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeoutMs, sizeof(timeoutMs));

    // Send request
    LPCVOID bodyPtr = body.empty() ? NULL : static_cast<LPCVOID>(body.c_str());
    DWORD bodyLen = static_cast<DWORD>(body.size());

    if (!HttpSendRequestA(hRequest, headers.c_str(), static_cast<DWORD>(headers.length()), (LPVOID)bodyPtr, bodyLen)) {
        closeInternetHandle(hRequest);
        closeInternetHandle(hConnect);
        closeInternetHandle(hInternet);
        response.error_message = "HttpSendRequestA failed";
        return response;
    }

    // Get response
    RequestResult result = stream
        ? sendStreamingRequest(hRequest, onChunk)
        : sendStandardRequest(hRequest);

    closeInternetHandle(hRequest);
    closeInternetHandle(hConnect);
    closeInternetHandle(hInternet);

    return result;
}

RequestResult NetworkClient::get(const std::string& url, const std::string& username, const std::string& password) {
    return sendRequest("GET", url, username, password, "", false, {});
}

RequestResult NetworkClient::post(const std::string& url, const std::string& username,
                                  const std::string& password, const std::string& json_body) {
    return sendRequest("POST", url, username, password, json_body, false, {});
}

RequestResult NetworkClient::postStream(const std::string& url, const std::string& username,
                                        const std::string& password, const std::string& json_body,
                                        std::function<void(const std::string&)> onChunk
                                    ) {
    return sendRequest("POST", url, username, password, json_body, true, onChunk);
}

RequestResult NetworkClient::del(const std::string& url, const std::string& username, const std::string& password) {
    return sendRequest("DELETE", url, username, password, "", false, {});
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