#pragma once
#include <sqlite3.h>
#include <string>
#include <iostream>

class Database
{
public:
    Database(const std::string &db_path);
    ~Database();

    // Создает таблицу, если её нет
    void init();

    // Сохраняет текущее значение счетчика
    void insert_log(int count);

private:
    sqlite3 *db;
    std::string db_path;
};