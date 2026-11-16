#pragma once
#include <sqlite3.h>
#include <string>
#include <iostream>

class Database
{
public:
    Database(const std::string &db_path);
    ~Database();

    // Creates the table if it does not exist
    void init();

    // Saves the entry and exit counters
    void insert_log(int in_count, int out_count);

private:
    sqlite3 *db;
    std::string db_path;
};