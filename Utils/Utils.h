#pragma once
#include <string>

/**
 * @brief Converts a wide string (UTF-16/UTF-32) to a UTF-8 encoded string.
 * 
 * @param wstr The input wide string to convert.
 * @return std::string UTF-8 encoded string.
 */
std::string wideToUtf8(const std::wstring& wstr);

/**
 * @brief Converts a UTF-8 encoded string to a wide string (UTF-16/UTF-32).
 * 
 * @param str The input UTF-8 encoded string.
 * @return std::wstring Converted wide string.
 */
std::wstring utf8ToWide(const std::string& str);

/**
 * @brief Encrypts the given input string.
 * 
 * @param input The input string to encrypt.
 * @return std::string The encrypted string.
 */
std::string encrypt(const std::string& input);

/**
 * @brief Converts binary data to a hexadecimal string representation.
 * 
 * @param data The binary data string.
 * @return std::string Hexadecimal representation of the data.
 */
std::string toHex(const std::string& data);

/**
 * @brief Converts a hexadecimal string back to binary data.
 * 
 * @param hex The hexadecimal string.
 * @return std::string The original binary data.
 */
std::string fromHex(const std::string& hex);
