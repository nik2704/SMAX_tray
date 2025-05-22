/**
 * @file JSONHelper.h
 * @brief Helper class for handling JSON and CSV files.
 */

#pragma once

#include <memory>
#include <vector>
#include "json.h"
#include "json_builder.h"

/**
 * @enum OutputDestination
 * @brief Enumeration of possible output destinations.
 */
enum class OutputDestination {
    Console,         ///< Output to console
    File,            ///< Output to file
    Both             ///< Output to both console and file
};

/**
 * @class JSONHelper
 * @brief Class providing utilities for processing and converting JSON and CSV files.
 *
 * Includes methods for validating JSON, converting between CSV and JSON,
 * as well as outputting data to console and files.
 */
class JSONHelper {
public:
    /**
     * @brief Checks whether a file contains valid JSON.
     * @param filename Path to the file.
     * @return Returns true if the file contains valid JSON, otherwise false.
     */
    static bool IsValidJSON(const std::string& filename);

    /**
     * @brief Converts JSON string data from one file to another JSON file.
     * @param inputFile Path to the source JSON file.
     * @param outputFile Path to the output JSON file.
     * @return Returns true if conversion succeeded, otherwise false.
     */
    static bool ConvertJSON(const std::string& inputFile, const std::string& outputFile);

    /**
     * @brief Prints the contents of a JSON file to the console.
     * @param filename Path to the JSON file.
     * @return Returns true if the file was successfully read and printed, otherwise false.
     */
    static bool PrintJSONfromFile(const std::string& filename);

    /**
     * @brief Converts a CSV file to JSON.
     * @param csvFile Path to the CSV file.
     * @param outputDestination Output destination (default is console).
     * @param outputJsonFile Path to the file to save JSON output (default is empty, meaning no file saving).
     * @return Returns a pointer to the JSON document.
     */
    static std::shared_ptr<json::Document> ConvertCSVToJSON(
        const std::string& csvFile,
        OutputDestination outputDestination = OutputDestination::Console,
        const std::string& outputJsonFile = ""
    );

    /**
     * @brief Parses a JSON string into a JSON document.
     * @param jsonString String containing JSON.
     * @return Pointer to the JSON document if parsing succeeds, otherwise nullptr.
     */
    static std::shared_ptr<json::Document> ParseJSONFromString(const std::string& jsonString);

private:
    /**
     * @brief Splits a CSV line into individual elements.
     * @param line CSV line.
     * @return Vector of strings representing individual elements.
     */
    static std::vector<std::string> SplitCSV(const std::string& line);

    /**
     * @brief Converts a CSV file into JSON.
     * @param csvFile Path to the CSV file.
     * @return Pointer to the created JSON document.
     */
    static std::shared_ptr<json::Document> CreateJsonFromCSV(const std::string& csvFile);

    /**
     * @brief Reads headers from a CSV file.
     * @param input Input stream.
     * @return Vector of strings representing the headers.
     */
    static std::shared_ptr<std::vector<std::string>> ReadCSVHeaders(std::ifstream& input);

    /**
     * @brief Creates JSON based on CSV data rows.
     * @param input Input stream.
     * @param headers Vector of CSV headers.
     * @return Pointer to the created JSON document.
     */
    static std::shared_ptr<json::Document> CreateJsonFromRows(std::ifstream& input, const std::shared_ptr<std::vector<std::string>>& headers);

    /**
     * @brief Prints a JSON document to console or file.
     * @param doc Pointer to the JSON document.
     * @param outputDestination Output destination (console, file, or both).
     * @param jsonFile Path to the JSON file for output.
     */
    static void PrintJSON(const std::shared_ptr<json::Document>& doc, const OutputDestination & outputDestination, const std::string& jsonFile);

    /**
     * @brief Prints a JSON document to a file.
     * @param doc Pointer to the JSON document.
     * @param jsonFile Path to the JSON file for output.
     */
    static void PrintJSONtoFile(const std::shared_ptr<json::Document>& doc, const std::string& jsonFile);
};
