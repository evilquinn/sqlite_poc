
#ifndef DATABASE_STMT_HPP_
#define DATABASE_STMT_HPP_


#include <database.hpp>
#include <functional>

class database_stmt
{
public:
    typedef std::function<void (const database::column_value_pairs&)>
        row_callback;

    database_stmt(database& db, const std::string& statement);
    virtual ~database_stmt(){}

    virtual int execute(row_callback callback);
    virtual sqlite3_stmt* get()
    {
        return db_stmt_.get();
    }

private:
    typedef std::shared_ptr<sqlite3_stmt> db_stmt;
    db_stmt db_stmt_;
};


#endif // DATABASE_STMT_HPP_
