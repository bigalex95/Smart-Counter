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

    // Сохраняет счетчики входа и выхода
    void insert_log(int in_count, int out_count);

private:
    sqlite3 *db;
    std::string db_path;
};