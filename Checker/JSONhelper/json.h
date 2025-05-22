/**
 * @file json.h
 * @brief Определение классов для работы с JSON-структурами.
 *
 * Этот заголовочный файл содержит определения классов и функций, 
 * необходимых для представления и обработки JSON-данных в C++.
 */

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * @namespace json
 * @brief Пространство имен для работы с JSON-объектами.
 */
namespace json {

class Node;
using Dict = std::map<std::string, Node>; ///< Определение типа JSON-объекта.
using Array = std::vector<Node>; ///< Определение типа JSON-массива.

/**
 * @brief Контекст для печати, содержащий параметры форматирования.
 * 
 * Эта структура хранит информацию о потоке вывода, шаге отступа и текущем уровне отступа,
 * а также предоставляет методы для удобного форматирования вывода с отступами.
 */
struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    /**
     * @brief Печатает отступы в соответствии с текущим уровнем вложенности.
     * 
     * Метод выводит пробелы в соответствии с текущим значением поля `indent`, что позволяет
     * отформатировать вывод с нужным количеством пробелов перед текстом.
     */
    void PrintIndent() const;

    /**
     * @brief Создает новый контекст печати с увеличенным уровнем отступа.
     * 
     * Этот метод создает копию текущего контекста с увеличением уровня отступа на
     * значение `indent_step`, что позволяет использовать его для печати вложенных структур
     * с соответствующими отступами.
     * 
     * @return Новый контекст печати с увеличенным уровнем отступа.
     */
    PrintContext Indented() const;
};

/**
 * @class ParsingError
 * @brief Исключение, возникающее при ошибке парсинга JSON.
 */
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error; ///< Использует конструктор базового класса.
};

/**
 * @class Node
 * @brief Универсальный контейнер для представления JSON-значений.
 *
 * Поддерживает типы: null, массив, объект, строка, логическое значение, 
 * целые и дробные числа.
 */
class Node final : public std::variant<std::nullptr_t, Array, Dict, std::string, bool, int, double> {
public:
    using Value = std::variant<std::nullptr_t, Array, Dict, std::string, bool, int, double>;
    using Value::variant;

    /**
     * @brief Конструктор узла JSON.
     * @param value Значение JSON-узла.
     */
    Node(Value value);

    /// @name Методы проверки типа
    /// @{
    bool IsInt() const; ///< Проверяет, хранится ли в узле целое число.
    bool IsPureDouble() const; ///< Проверяет, является ли число только double (не int).
    bool IsDouble() const; ///< Проверяет, является ли число double (включая int).
    bool IsBool() const; ///< Проверяет, является ли значение логическим.
    bool IsNull() const; ///< Проверяет, является ли значение null.
    bool IsArray() const; ///< Проверяет, является ли значение массивом.
    bool IsString() const; ///< Проверяет, является ли значение строкой.
    bool IsMap() const; ///< Проверяет, является ли значение объектом.
    /// @}

    /// @name Методы получения значения
    /// @{
    int AsInt() const; ///< Возвращает значение как int.
    double AsDouble() const; ///< Возвращает значение как double.
    bool AsBool() const; ///< Возвращает значение как bool.
    const Array& AsArray() const; ///< Возвращает значение как массив.
    const std::string& AsString() const; ///< Возвращает значение как строку.
    const Dict& AsMap() const; ///< Возвращает значение как объект.
    /// @}

    /**
     * @brief Оператор сравнения.
     * @param rhs Другой узел JSON.
     * @return true, если узлы равны.
     */
    bool operator==(const Node& rhs) const;

    /**
     * @brief Доступ к значению узла.
     * @return Ссылка на значение узла.
     */
    Value& GetValue();
    const Value& GetValue() const;
};

/**
 * @brief Оператор неравенства для Node.
 * @param lhs Левый операнд.
 * @param rhs Правый операнд.
 * @return true, если узлы не равны.
 */
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

/**
 * @class Document
 * @brief Представляет JSON-документ, содержащий корневой узел.
 */
class Document {
public:
    /**
     * @brief Конструктор документа.
     * @param root Корневой узел документа.
     */
    explicit Document(Node root);

    /**
     * @brief Получает корневой узел документа.
     * @return Ссылка на корневой узел.
     */
    const Node& GetRoot() const;

private:
    Node root_; ///< Корневой узел документа.
};

/**
 * @brief Оператор сравнения JSON-документов.
 * @param lhs Левый документ.
 * @param rhs Правый документ.
 * @return true, если документы равны.
 */
inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

/**
 * @brief Оператор неравенства JSON-документов.
 * @param lhs Левый документ.
 * @param rhs Правый документ.
 * @return true, если документы не равны.
 */
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

/**
 * @brief Загружает JSON-документ из входного потока.
 * @param input Входной поток.
 * @return JSON-документ.
 */
Document Load(std::istream& input);

/**
 * @brief Выводит JSON-документ в поток.
 * @param doc JSON-документ.
 * @param output Выходной поток.
 */
void Print(const Document& doc, std::ostream& output);

}  // namespace json