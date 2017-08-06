
#include <iostream>
#include <sstream>
#include <sqlite3.h>
#include <memory>
#include <thread>
#include <mutex>
#include <boost/range/irange.hpp>
#include <map>

typedef std::unique_ptr<sqlite3,
                        decltype(&sqlite3_close_v2)> database;
typedef std::unique_ptr<sqlite3_stmt,
                        decltype(&sqlite3_finalize)> database_stmt;

namespace db
{

std::string error_string(const database& db, const int error_code)
{
    std::stringstream errstr;
    errstr << "errcode: " << sqlite3_errcode(db.get()) << "\n"
           << "ext.errcode: " << sqlite3_extended_errcode(db.get()) << "\n"
           << "errmsg: " << sqlite3_errmsg(db.get()) << "\n"
           << "errstr: " << sqlite3_errstr(error_code) << "\n";
    return errstr.str();
}

database open(const std::string& filename)
{
    sqlite3* temp_handle;
    int open_result = sqlite3_open(filename.c_str(), &temp_handle);
    database db(temp_handle, sqlite3_close_v2);
    if ( open_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error opening: " << filename
               << "\n"
               << error_string(db, open_result);
        throw std::runtime_error(errstr.str());
    }
    return db;
}

database_stmt prepare(const database& db,
                      const std::string& statement)
{
    sqlite3_stmt* temp_handle;
    int prepare_result = sqlite3_prepare_v2(db.get(),
                                            statement.c_str(),
                                            statement.size()+1,
                                            &temp_handle,
                                            NULL);
    database_stmt stmt(temp_handle, sqlite3_finalize);
    if ( prepare_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error preparing statement:\n"
               << statement << "\n"
               << error_string(db, prepare_result);
        throw std::runtime_error(errstr.str());
    }
    return stmt;
}

typedef std::map<std::string, std::string> column_value_pairs;
typedef int (*row_callback)(void*, const column_value_pairs&);
int execute(const database_stmt& statement, row_callback callback, void* userdata)
{
    int step_result;
    do
    {
        step_result = sqlite3_step(statement.get());
        switch ( step_result )
        {
        case SQLITE_ROW :
        {
            column_value_pairs column_values;
            for ( int i : boost::irange(0, sqlite3_column_count(statement.get())) )
            {
                std::stringstream val;
                val << sqlite3_column_text(statement.get(), i);
                column_values[sqlite3_column_name(statement.get(), i)] = val.str();
            }
            callback(userdata, column_values);
            break;
        } // end SQLITE_ROW
        case SQLITE_DONE :
        {
            // no op
            break;
        } // end SQLITE_DONE
        default :
        {
            // some error
            std::stringstream errstr;
            errstr << "Error stepping statement:\n"
                   << sqlite3_sql(statement.get())
                   << "\n";
            throw std::runtime_error(errstr.str());
        } // end default
        } // end switch
    }
    while ( step_result == SQLITE_ROW );

    return step_result;
}

} // end namespace db

std::string get_value(const database& db,
                      const std::string& key)
{
    // one-time static init
    static std::string sql = "select value from keys where key = ?";
    static database_stmt stmt = db::prepare(db, sql.c_str());
    static std::mutex stmt_mutex;

    // get a lock so no one else blatters the prepared statement
    std::lock_guard<std::mutex> get_value_stmt_lock(stmt_mutex);

    sqlite3_bind_text(stmt.get(), 1, key.c_str(), key.size(), NULL);
    std::stringstream val;
    int step_result = sqlite3_step(stmt.get());
    switch ( step_result )
    {
    case SQLITE_ROW :
    {
        val << sqlite3_column_text(stmt.get(), 0);
        break;
    } // end SQLITE_ROW
    case SQLITE_DONE :
    {
        // no op
        break;
    } // end SQLITE_DONE
    default :
    {
        // some error
        std::stringstream errstr;
        errstr << "Error getting value for: " << key
               << "\n"
               << db::error_string(db, step_result);
        throw std::runtime_error(errstr.str());
    } // end default
    } // end switch

    sqlite3_reset(stmt.get());
    return val.str();
}

void do_get_values(const database& db,
                   const std::string& key)
{
    for ( int i : boost::irange(0, 100) )
    {
        std::cout << key << "(" << i << ") : "
                  << get_value(db, key) << std::endl;
    }
}

int my_callback(void*, const db::column_value_pairs& cvp)
{
    for ( const auto& e : cvp )
    {
        std::cout << e.first << " : " << e.second << std::endl;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    database super_db(db::open("keys.db"));

    const char* create_statement =
        "CREATE TABLE IF NOT EXISTS keys ("
        " key text PRIMARY KEY,"
        " value text NOT NULL );";
    const char* insert_statement =
        "INSERT OR REPLACE INTO keys ( key, value )"
        " VALUES"
        " ( 'firstname', 'Niall' ),"
        " ( 'lastname', 'Quinn' );";

    database_stmt stmt(db::prepare(super_db, create_statement));

    int create_result = sqlite3_step(stmt.get());
    printf("Create result: %s (%d)\n",
            sqlite3_errstr(create_result),
            create_result);

    stmt = db::prepare(super_db, insert_statement);
    int insert_result = sqlite3_step(stmt.get());
    printf("Insert result: %s (%d)\n",
            sqlite3_errstr(insert_result),
            insert_result);

    std::cout << "firstname: " << get_value(super_db, "firstname") << std::endl;
    std::cout << "lastname: " << get_value(super_db, "lastname") << std::endl;

    std::cout << "threads:" << std::endl;

    std::thread get_firstnames(do_get_values, std::ref(super_db), "firstname");
    std::thread get_lastnames(do_get_values, std::ref(super_db), "lastname");

    get_firstnames.join();
    get_lastnames.join();

    stmt = db::prepare(super_db, "select * from keys");
    db::execute(stmt, my_callback, NULL);

    return 0;
}
