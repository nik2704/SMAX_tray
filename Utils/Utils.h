#pragma once
#include <string>

std::string wideToUtf8(const std::wstring& wstr);
std::wstring utf8ToWide(const std::string& str);
std::string encrypt(const std::string& input);
std::string toHex(const std::string& data);
std::string fromHex(const std::string& hex);