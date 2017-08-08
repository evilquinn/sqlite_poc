
#ifndef DATABASE_HPP_
#define DATABASE_HPP_

#include <sqlite3.h>
#include <memory>
#include <map>

class database
{
public:
    typedef std::map<std::string, std::string> column_value_pairs;
    database(const std::string& filename);
    static std::string error_string(sqlite3* const db, const int error_code);
    virtual sqlite3* get()
    {
        return db_.get();
    }
private:
    typedef std::unique_ptr<sqlite3,
                            decltype(&sqlite3_close_v2)> db;
    db db_;
};

#endif // DATABASE_HPP_
