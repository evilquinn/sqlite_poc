
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
    void backup_to(const std::string& backup_path);
    void restore_from(const std::string& restore_from_path);

    virtual sqlite3* handle()
    {
        return db_.get();
    }
private:
    enum sync_direction
    {
        backup,
        restore
    };

    void backup_or_recover(const std::string& path, sync_direction d);
    typedef std::unique_ptr<sqlite3,
                            decltype(&sqlite3_close_v2)> db;
    db db_;
};

#endif // DATABASE_HPP_
