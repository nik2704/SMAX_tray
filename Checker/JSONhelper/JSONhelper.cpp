#include "JSONhelper.h"

#include <iostream>
#include <fstream>
#include <sstream>

bool JSONHelper::IsValidJSON(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return false;
    }

    try {
        json::Document doc = json::Load(input);
        return true;
    } catch (const json::ParsingError& e) {
        std::cerr << "Invalid JSON: " << e.what() << "\n";
        return false;
    }
}

bool JSONHelper::ConvertJSON(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream input(inputFile);
    if (!input) {
        std::cerr << "Error: Cannot open file " << inputFile << "\n";
        return false;
    }

    std::ofstream output(outputFile);
    if (!output) {
        std::cerr << "Error: Cannot create file " << outputFile << "\n";
        return false;
    }

    try {
        json::Document doc = json::Load(input);
        json::Print(doc, output);
        std::cout << "JSON successfully converted and saved to " << outputFile << "\n";
        return true;
    } catch (const json::ParsingError& e) {
        std::cerr << "Invalid JSON: " << e.what() << "\n";
        return false;
    }
}

bool JSONHelper::PrintJSONfromFile(const std::string& filename) {
    std::ifstream input(filename);
    if (!input) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return false;
    }

    try {
        json::Document doc = json::Load(input);
        json::Print(doc, std::cout);
        return true;
    } catch (const json::ParsingError& e) {
        std::cerr << "Invalid JSON: " << e.what() << "\n";
        return false;
    }
}

std::shared_ptr<json::Document> JSONHelper::ConvertCSVToJSON(const std::string& csvFile, OutputDestination outputDestination, const std::string& outputJsonFile) {
    auto doc = CreateJsonFromCSV(csvFile);

    PrintJSON(doc, outputDestination, outputJsonFile);

    return doc;
}

std::shared_ptr<json::Document> JSONHelper::ParseJSONFromString(const std::string& jsonString) {
    try {
        std::istringstream iss(jsonString);
        json::Document doc = json::Load(iss);
        return std::make_shared<json::Document>(std::move(doc));
    } catch (const json::ParsingError& e) {
        std::cerr << "Invalid JSON: " << e.what() << std::endl;
        return nullptr;
    }
}

std::vector<std::string> JSONHelper::SplitCSV(const std::string& line) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    
    while (std::getline(ss, item, ',')) {
        result.push_back(item);
    }
    return result;
}

std::shared_ptr<json::Document> JSONHelper::CreateJsonFromCSV(const std::string& csvFile) {
    std::ifstream input(csvFile);
    if (!input) {
        std::cerr << "Error: Cannot open file " << csvFile << "\n";
        return nullptr;
    }

    auto headers = ReadCSVHeaders(input);
    if (!headers) {
        return nullptr;
    }

    return CreateJsonFromRows(input, headers);
}

std::shared_ptr<std::vector<std::string>> JSONHelper::ReadCSVHeaders(std::ifstream& input) {
    std::string line;

    if (std::getline(input, line)) {
        auto headers = std::make_shared<std::vector<std::string>>(SplitCSV(line));
        return headers;
    } else {
        std::cerr << "Error: CSV file is empty or cannot read headers\n";
        return nullptr;
    }
}

std::shared_ptr<json::Document> JSONHelper::CreateJsonFromRows(std::ifstream& input, const std::shared_ptr<std::vector<std::string>>& headers) {
    json::Builder builder;
    builder.StartArray();

    std::string line;
    while (std::getline(input, line)) {
        std::vector<std::string> values = SplitCSV(line);

        if (values.size() != headers->size()) {
            std::cerr << "Error: Row has different number of columns than header\n";
            return nullptr;
        }

        builder.StartDict();
        for (size_t i = 0; i < headers->size(); ++i) {
            builder.Key((*headers)[i]).Value(values[i]);
        }

        builder.EndDict();
    }

    builder.EndArray();
    json::Node root = builder.Build();

    return std::make_shared<json::Document>(root);
}

void JSONHelper::PrintJSON(const std::shared_ptr<json::Document>& doc, const OutputDestination & outputDestination, const std::string& jsonFile) {
    if (doc) {
        switch (outputDestination) {
        case OutputDestination::Console:
            json::Print(*doc, std::cout);
            break;

        case OutputDestination::File:
            PrintJSONtoFile(doc, jsonFile);
            break;

        case OutputDestination::Both:
            json::Print(*doc, std::cout);
            PrintJSONtoFile(doc, jsonFile);
            break;

        default:
            break;
        }
    } else {
        std::cerr << "Error: Invalid JSON, cannot output\n";
    }
}    

void JSONHelper::PrintJSONtoFile(const std::shared_ptr<json::Document>& doc, const std::string& jsonFile) {
    if (doc) {
            std::ofstream output(jsonFile);
            if (!output) {
                std::cerr << "Error: Cannot create file " << jsonFile << "\n";
            }     
            
            json::Print(*doc, output);
    } else {
        std::cerr << "Error: Invalid JSON, cannot output\n";
    }
}    
