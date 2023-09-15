#include <iostream>
#include "sqlite3.h"

int main() {
    sqlite3* db;
    char* errMsg = 0;

    // Открываем или создаем базу данных
    int rc = sqlite3_open("example.db", &db);

    if (rc) {
        std::cerr << "Не удалось открыть/создать базу данных: " << sqlite3_errmsg(db) << std::endl;
        return rc;
    } else {
        std::cout << "База данных успешно открыта/создана." << std::endl;
    }

    // Создаем таблицу, если она не существует
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS People ("
                                  "ID INT PRIMARY KEY NOT NULL,"
                                  "Name TEXT NOT NULL,"
                                  "Age INT NOT NULL);";

    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL ошибка: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Таблица 'People' успешно создана или уже существует." << std::endl;
    }

    // Вставляем данные
    const char* insertDataSQL = "INSERT INTO People (ID, Name, Age) VALUES (1, 'Иван', 30);"
                                "INSERT INTO People (ID, Name, Age) VALUES (2, 'Мария', 25);"
                                "INSERT INTO People (ID, Name, Age) VALUES (3, 'Петр', 35);";

    rc = sqlite3_exec(db, insertDataSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL ошибка: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << "Данные успешно добавлены." << std::endl;
    }

    // Чтение данных
    const char* selectDataSQL = "SELECT * FROM People;";
    sqlite3_stmt* stmt;

    rc = sqlite3_prepare_v2(db, selectDataSQL, -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            int age = sqlite3_column_int(stmt, 2);

            std::cout << "ID: " << id << ", Имя: " << name << ", Возраст: " << age << std::endl;
        }
    } else {
        std::cerr << "SQL ошибка: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);

    // Закрываем базу данных
    sqlite3_close(db);

    return 0;
}

