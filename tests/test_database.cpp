#include <gtest/gtest.h>
#include <sqlite3.h>
#include <filesystem>
#include "database.h"

namespace fs = std::filesystem;

namespace
{
// Runs each test in a fresh temporary directory so relative paths can be tested.
class DatabaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        old_cwd_ = fs::current_path();
        tmp_ = fs::temp_directory_path() / fs::path("smartcounter_db_test_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()) + "_" +
                                                    ::testing::UnitTest::GetInstance()->current_test_info()->name());
        fs::remove_all(tmp_);
        fs::create_directories(tmp_);
        fs::current_path(tmp_);
    }
    void TearDown() override
    {
        fs::current_path(old_cwd_);
        fs::remove_all(tmp_);
    }

    static int row_count(const std::string &path)
    {
        sqlite3 *db = nullptr;
        EXPECT_EQ(sqlite3_open(path.c_str(), &db), SQLITE_OK);
        sqlite3_stmt *stmt = nullptr;
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM people_count", -1, &stmt, nullptr);
        int rows = (sqlite3_step(stmt) == SQLITE_ROW) ? sqlite3_column_int(stmt, 0) : -1;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return rows;
    }

    fs::path old_cwd_;
    fs::path tmp_;
};
} // namespace

TEST_F(DatabaseTest, BareFileNameCreatesAFileNotADirectory)
{
    {
        Database db("analytics.db");
        db.init();
        db.insert_log(3, 1);
    }
    EXPECT_TRUE(fs::is_regular_file("analytics.db"));
    EXPECT_EQ(row_count("analytics.db"), 1);
}

TEST_F(DatabaseTest, MissingNestedParentDirectoriesAreCreated)
{
    {
        Database db("logs/nested/analytics.db");
        db.init();
        db.insert_log(1, 0);
        db.insert_log(2, 0);
    }
    EXPECT_TRUE(fs::is_regular_file("logs/nested/analytics.db"));
    EXPECT_EQ(row_count("logs/nested/analytics.db"), 2);
}
