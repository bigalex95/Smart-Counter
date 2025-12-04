#include "database.h"
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <iostream>

Database::Database(const std::string &path) : db_path(path), db(nullptr)
{
    // Ensure directory exists
    std::string dir_path = path.substr(0, path.find_last_of("/"));
    mkdir(dir_path.c_str(), 0755);

    int rc = sqlite3_open(path.c_str(), &db);
    if (rc)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        std::cout << "Opened database successfully" << std::endl;
    }
}

Database::~Database()
{
    if (db)
        sqlite3_close(db);
}

void Database::init()
{
    // Создаем таблицу с двумя счетчиками: вход и выход
    const char *sql = "CREATE TABLE IF NOT EXISTS people_count ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
                      "in_count INTEGER NOT NULL,"
                      "out_count INTEGER NOT NULL);";

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    else
    {
        std::cout << "Table initialised successfully" << std::endl;
    }
}

void Database::insert_log(int in_count, int out_count)
{
    // В реальном коде лучше использовать Prepared Statements, чтобы избежать инъекций,
    // но для Int это безопасно.
    std::string sql = "INSERT INTO people_count (in_count, out_count) VALUES (" +
                      std::to_string(in_count) + ", " + std::to_string(out_count) + ");";

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);

    if (rc != SQLITE_OK)
    {
        std::cerr << "Insert error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}