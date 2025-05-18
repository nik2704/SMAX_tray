#include "Utils.h"
#include <windows.h>

std::string wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), (int)wstr.size(), &result[0], sizeNeeded, NULL, NULL);
    return result;
}

std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return {};
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), NULL, 0);
    std::wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &result[0], sizeNeeded);
    return result;
}

std::string encrypt(const std::string& input) {
    std::string result = input;
    char key = 0x5A;
    for (char& c : result) {
        c ^= key;
    }
    return result;
}

std::string toHex(const std::string& data) {
    static const char hex_digits[] = "0123456789ABCDEF";
    std::string hex;
    hex.reserve(data.size() * 2);
    for (unsigned char c : data) {
        hex.push_back(hex_digits[c >> 4]);
        hex.push_back(hex_digits[c & 0x0F]);
    }
    return hex;
}

std::string fromHex(const std::string& hex) {
    std::string data;
    data.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        unsigned char byte = (unsigned char)(std::stoi(hex.substr(i, 2), nullptr, 16));
        data.push_back(byte);
    }
    return data;
}

std::wstring getEncryptedString(std::wstring && strLine) {
    std::string tokenEncrypted = encrypt(wideToUtf8(strLine));
    std::string strHex = toHex(tokenEncrypted);

    std::wstring valueStr(strHex.begin(), strHex.end());

    return valueStr;
}

std::string getDecryptedString(const wchar_t* strHexW) {
    if (strHexW) {
        std::string strHex;
        size_t len = wcslen(strHexW);
        strHex.reserve(len);
        for (size_t i = 0; i < len; ++i) {
            strHex.push_back(static_cast<char>(strHexW[i]));
        }

        std::string strEncrypted = fromHex(strHex);
        auto result = encrypt(strEncrypted);

        return result;
    }

    return "";
}