/**
 * @file builder.h
 * @brief Definition of the json::Builder class for constructing JSON objects.
 */

#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

/**
 * @class Builder
 * @brief Class for step-by-step construction of a JSON object.
 */
class Builder {
private:
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;

public:
    /** @brief Default constructor. */
    Builder();
    
    /** @brief Finalizes the JSON construction and returns its root node. */
    Node Build();
    
    /** @brief Adds a key to the JSON object. */
    DictValueContext Key(std::string key);
    
    /** @brief Adds a value to the current container. */
    BaseContext Value(Node::Value value);
    
    /** @brief Starts a new JSON object. */
    DictItemContext StartDict();
    
    /** @brief Starts a new JSON array. */
    ArrayItemContext StartArray();
    
    /** @brief Ends the current JSON object. */
    BaseContext EndDict();
    
    /** @brief Ends the current JSON array. */
    BaseContext EndArray();

private:
    Node root_;  ///< Root node of the JSON object.
    std::vector<Node*> nodes_stack_; ///< Stack of current JSON nodes.

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    
    void AssertNewObjectContext() const;
    void AddValue(Node::Value value);
    void AddNode(Node::Value value);
    
    /** @brief Base context for managing JSON construction. */
    class BaseContext {
    public:
        BaseContext(Builder& builder);
        Node Build();
        DictValueContext Key(std::string key);
        BaseContext Value(Node::Value value);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        BaseContext EndDict();
        BaseContext EndArray();
    private:
        Builder& builder_;
    };
    
    /** @brief Context for a value inside a dictionary. */
    class DictValueContext : public BaseContext {
    public:
        DictValueContext(BaseContext base);
        DictItemContext Value(Node::Value value);
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
    };
    
    /** @brief Context inside a dictionary when a key is expected. */
    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base);
        Node Build() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };
    
    /** @brief Context for an element inside an array. */
    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(BaseContext base);
        ArrayItemContext Value(Node::Value value);
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
    };
};

}  // namespace json
