
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
    typedef std::unique_lock<std::mutex> lock_type;

    database_stmt(database& db, const std::string& statement);

    virtual void execute(row_callback callback = nullptr);
    virtual void execute(const database::column_value_pairs& cvp,
                         row_callback callback = nullptr);
    virtual lock_type get_lock();

    virtual sqlite3_stmt* handle()
    {
        return db_stmt_.get();
    }

private:
    virtual void execute_no_locking(row_callback callback = nullptr);
    typedef std::unique_ptr<sqlite3_stmt,
                            decltype(&sqlite3_finalize)> db_stmt;
    db_stmt db_stmt_;
    std::unique_ptr<std::mutex> db_stmt_mutex_;
};

#endif // DATABASE_STMT_HPP_