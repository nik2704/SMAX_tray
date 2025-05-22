/**
 * @file builder.h
 * @brief Определение класса json::Builder для построения JSON-объектов.
 */

#pragma once

#include <string>
#include <vector>
#include "json.h"

namespace json {

/**
 * @class Builder
 * @brief Класс для поэтапного построения JSON-объекта.
 */
class Builder {
private:
    class BaseContext;
    class DictValueContext;
    class DictItemContext;
    class ArrayItemContext;

public:
    /** @brief Конструктор по умолчанию. */
    Builder();
    
    /** @brief Завершает построение JSON и возвращает его корневой узел. */
    Node Build();
    
    /** @brief Добавляет ключ в объект JSON. */
    DictValueContext Key(std::string key);
    
    /** @brief Добавляет значение в текущий контейнер. */
    BaseContext Value(Node::Value value);
    
    /** @brief Начинает новый объект JSON. */
    DictItemContext StartDict();
    
    /** @brief Начинает новый массив JSON. */
    ArrayItemContext StartArray();
    
    /** @brief Завершает объект JSON. */
    BaseContext EndDict();
    
    /** @brief Завершает массив JSON. */
    BaseContext EndArray();

private:
    Node root_;  ///< Корневой узел JSON-объекта.
    std::vector<Node*> nodes_stack_; ///< Стек текущих узлов JSON.

    Node::Value& GetCurrentValue();
    const Node::Value& GetCurrentValue() const;
    
    void AssertNewObjectContext() const;
    void AddValue(Node::Value value);
    void AddNode(Node::Value value);
    
    /** @brief Базовый контекст для управления построением JSON. */
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
    
    /** @brief Контекст для значения внутри словаря. */
    class DictValueContext : public BaseContext {
    public:
        DictValueContext(BaseContext base);
        DictItemContext Value(Node::Value value);
        Node Build() = delete;
        DictValueContext Key(std::string key) = delete;
        BaseContext EndDict() = delete;
        BaseContext EndArray() = delete;
    };
    
    /** @brief Контекст внутри словаря, когда ожидается ключ. */
    class DictItemContext : public BaseContext {
    public:
        DictItemContext(BaseContext base);
        Node Build() = delete;
        BaseContext Value(Node::Value value) = delete;
        BaseContext EndArray() = delete;
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
    };
    
    /** @brief Контекст для элемента внутри массива. */
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