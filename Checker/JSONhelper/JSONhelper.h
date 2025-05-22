/**
 * @file JSONHelper.h
 * @brief Класс-помощник для обработки JSON и CSV файлов.
 */

#pragma once

#include <memory>
#include <vector>
#include "json.h"
#include "json_builder.h"

/**
 * @enum OutputDestination
 * @brief Перечисление возможных вариантов вывода данных.
 */
enum class OutputDestination {
    Console,         ///< Вывод в консоль
    File,            ///< Вывод в файл
    Both             ///< Вывод и в консоль, и в файл
};

/**
 * @class JSONHelper
 * @brief Класс, предоставляющий утилиты для обработки и конвертации JSON и CSV файлов.
 *
 * Включает методы для валидации JSON, конвертации между CSV и JSON, а также для вывода данных в консоль и файлы.
 */
class JSONHelper {
public:
    /**
     * @brief Проверяет, является ли файл валидным JSON.
     * @param filename Путь к файлу.
     * @return Возвращает true, если файл является валидным JSON, иначе false.
     */
    static bool IsValidJSON(const std::string& filename);

    /**
     * @brief Конвертирует строковые данные JSON файл в другой JSON файл.
     * @param inputFile Путь к исходному JSON файлу.
     * @param outputFile Путь к выходному JSON файлу.
     * @return Возвращает true, если конвертация прошла успешно, иначе false.
     */
    static bool ConvertJSON(const std::string& inputFile, const std::string& outputFile);

    /**
     * @brief Выводит содержимое JSON файла в консоль.
     * @param filename Путь к JSON файлу.
     * @return Возвращает true, если файл был успешно прочитан и выведен, иначе false.
     */
    static bool PrintJSONfromFile(const std::string& filename);

    /**
     * @brief Конвертирует CSV файл в JSON.
     * @param csvFile Путь к CSV файлу.
     * @param outputDestination Вывод данных (по умолчанию - консоль).
     * @param outputJsonFile Путь к файлу для записи JSON (по умолчанию - пустая строка, значит не сохраняется в файл).
     * @return Возвращает указатель на JSON документ.
     */
    static std::shared_ptr<json::Document> ConvertCSVToJSON(
        const std::string& csvFile,
        OutputDestination outputDestination = OutputDestination::Console,
        const std::string& outputJsonFile = ""
    );

    /**
     * @brief Преобразует JSON-строку в JSON-документ.
     * @param jsonString Строка, содержащая JSON.
     * @return Указатель на JSON-документ, если разбор успешен, иначе nullptr.
     */
    static std::shared_ptr<json::Document> ParseJSONFromString(const std::string& jsonString);

private:
    /**
     * @brief Разделяет строку CSV на отдельные элементы.
     * @param line Строка CSV.
     * @return Вектор строк, представляющих отдельные элементы.
     */
    static std::vector<std::string> SplitCSV(const std::string& line);

    /**
     * @brief Преобразует CSV файл в JSON.
     * @param csvFile Путь к CSV файлу.
     * @return Указатель на созданный JSON документ.
     */
    static std::shared_ptr<json::Document> CreateJsonFromCSV(const std::string& csvFile);

    /**
     * @brief Читает заголовки из CSV файла.
     * @param input Поток ввода.
     * @return Вектор строк, представляющих заголовки.
     */
    static std::shared_ptr<std::vector<std::string>> ReadCSVHeaders(std::ifstream& input);

    /**
     * @brief Создает JSON на основе данных CSV.
     * @param input Поток ввода.
     * @param headers Вектор заголовков CSV.
     * @return Указатель на созданный JSON документ.
     */
    static std::shared_ptr<json::Document> CreateJsonFromRows(std::ifstream& input, const std::shared_ptr<std::vector<std::string>>& headers);

    /**
     * @brief Печатает JSON документ в консоль или в файл.
     * @param doc Указатель на JSON документ.
     * @param outputDestination Место вывода (консоль, файл или оба).
     * @param jsonFile Путь к файлу для записи JSON.
     */
    static void PrintJSON(const std::shared_ptr<json::Document>& doc, const OutputDestination & outputDestination, const std::string& jsonFile);

    /**
     * @brief Печатает JSON документ в файл.
     * @param doc Указатель на JSON документ.
     * @param jsonFile Путь к файлу для записи JSON.
     */
    static void PrintJSONtoFile(const std::shared_ptr<json::Document>& doc, const std::string& jsonFile);
};
