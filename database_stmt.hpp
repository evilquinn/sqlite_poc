
#ifndef DATABASE_STMT_HPP_
#define DATABASE_STMT_HPP_

#include <database.hpp>
#include <functional>
#include <mutex>

class database_stmt
{
public:
    typedef std::function<void (const database::column_value_pairs&)>
        row_callback;

    database_stmt(database& db, const std::string& statement);

    virtual void execute(row_callback callback = nullptr);

    virtual sqlite3_stmt* handle()
    {
        return db_stmt_.get();
    }

private:
    typedef std::unique_ptr<sqlite3_stmt,
                            decltype(&sqlite3_finalize)> db_stmt;
    db_stmt db_stmt_;
};

#endif // DATABASE_STMT_HPP_