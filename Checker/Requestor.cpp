#include "Requestor.h"
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
    // First call to get the required buffer size
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

    // Trim the trailing null character if any
    if (!encoded.empty() && encoded.back() == '\0') {
        encoded.pop_back();
    }

    return encoded;
}

std::optional<std::string> Requestor::get(const std::string& url, const std::string& username, const std::string& password) {
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

    if (!HttpSendRequestA(hRequest, headers.c_str(), headers.length(), NULL, 0)) {
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

} // namespace smax