/**
 * @file json.h
 * @brief Definition of classes for working with JSON structures.
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * @namespace json
 * @brief Namespace for working with JSON objects.
 */
namespace json {

class Node;
using Dict = std::map<std::string, Node>; ///< Definition of a JSON object type.
using Array = std::vector<Node>;           ///< Definition of a JSON array type.

/**
 * @brief Print context containing formatting parameters.
 * 
 * This structure holds information about the output stream, the indent step, and the current indent level,
 * and provides methods for convenient formatted output with indentation.
 */
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    /**
     * @brief Prints indentation according to the current nesting level.
     * 
     * Outputs spaces matching the current value of the `indent` field, 
     * allowing formatted output with the correct number of spaces before the text.
     */
    void PrintIndent() const;

    /**
     * @brief Creates a new print context with increased indentation level.
     * 
     * This method creates a copy of the current context with the indent level increased by
     * `indent_step`, enabling printing of nested structures with appropriate indentation.
     * 
     * @return New print context with increased indentation level.
     */
    PrintContext Indented() const;
};

/**
 * @class ParsingError
 * @brief Exception thrown on JSON parsing errors.
 */
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error; ///< Inherit constructors from std::runtime_error.
};

/**
 * @class Node
 * @brief Universal container representing JSON values.
 *
 * Supports types: null, array, object, string, boolean,
 * integer and floating point numbers.
 */
class Node final : public std::variant<std::nullptr_t, Array, Dict, std::string, bool, int, double> {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, std::string, bool, int, double>;
    using Value::variant;

    /**
     * @brief Constructs a JSON node.
     * @param value JSON node value.
     */
    Node(Value value);

    /// @name Type-checking methods
    /// @{
    bool IsInt() const;       ///< Checks if the node contains an integer.
    bool IsPureDouble() const;///< Checks if the node contains only a double (not int).
    bool IsDouble() const;    ///< Checks if the node contains a double (including int).
    bool IsBool() const;      ///< Checks if the value is boolean.
    bool IsNull() const;      ///< Checks if the value is null.
    bool IsArray() const;     ///< Checks if the value is an array.
    bool IsString() const;    ///< Checks if the value is a string.
    bool IsMap() const;       ///< Checks if the value is an object.
    /// @}

    /// @name Value access methods
    /// @{
    int AsInt() const;              ///< Returns the value as int.
    double AsDouble() const;        ///< Returns the value as double.
    bool AsBool() const;            ///< Returns the value as bool.
    const Array& AsArray() const;   ///< Returns the value as array.
    const std::string& AsString() const; ///< Returns the value as string.
    const Dict& AsMap() const;      ///< Returns the value as object.
    /// @}

    /**
     * @brief Equality comparison operator.
     * @param rhs Another JSON node.
     * @return true if nodes are equal.
     */
    bool operator==(const Node& rhs) const;

    /**
     * @brief Access to the node's value.
     * @return Reference to the node's value.
     */
    Value& GetValue();
    const Value& GetValue() const;
};

/**
 * @brief Inequality operator for Node.
 * @param lhs Left operand.
 * @param rhs Right operand.
 * @return true if nodes are not equal.
 */
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

/**
 * @class Document
 * @brief Represents a JSON document containing a root node.
 */
class Document {
public:
    /**
     * @brief Constructs a document.
     * @param root Root node of the document.
     */
    explicit Document(Node root);

    /**
     * @brief Gets the root node of the document.
     * @return Reference to the root node.
     */
    const Node& GetRoot() const;

private:
    Node root_; ///< Root node of the document.
};

/**
 * @brief Equality operator for JSON documents.
 * @param lhs Left document.
 * @param rhs Right document.
 * @return true if documents are equal.
 */
inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

/**
 * @brief Inequality operator for JSON documents.
 * @param lhs Left document.
 * @param rhs Right document.
 * @return true if documents are not equal.
 */
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Loads a JSON document from an input stream.
 * @param input Input stream.
 * @return JSON document.
 */
Document Load(std::istream& input);

/**
 * @brief Prints a JSON document to an output stream.
 * @param doc JSON document.
 * @param output Output stream.
 */
void Print(const Document& doc, std::ostream& output);

}  // namespace json
