
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
    virtual ~database(){}
    static std::string error_string(sqlite3* const db, const int error_code);
    virtual sqlite3* get()
    {
        return db_.get();
    }
private:
    typedef std::shared_ptr<sqlite3> db;
    db db_;
};

#endif // DATABASE_HPP_
