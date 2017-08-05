
#include <iostream>
#include <sstream>
#include <sqlite3.h>
#include <memory>

typedef std::unique_ptr<sqlite3,
                        decltype(&sqlite3_close_v2)> database;
typedef std::unique_ptr<sqlite3_stmt,
                        decltype(&sqlite3_finalize)> database_stmt;

namespace db
{

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
               << "errcode: " << sqlite3_errcode(temp_handle) << "\n"
               << "ext.errcode: " << sqlite3_extended_errcode(temp_handle)
               << "\n"
               << "errmsg: " << sqlite3_errmsg(temp_handle) << "\n"
               << "errstr: " << sqlite3_errstr(open_result) << "\n";
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
               << "errcode: " << sqlite3_errcode(db.get()) << "\n"
               << "ext.errcode: " << sqlite3_extended_errcode(db.get()) << "\n"
               << "errmsg: " << sqlite3_errmsg(db.get()) << "\n"
               << "errstr: " << sqlite3_errstr(prepare_result) << "\n";
        throw std::runtime_error(errstr.str());
    }
    return stmt;
}

} // end namespace db

std::string get_value(const database& db,
                      const std::string& key)
{
    std::string sql = "select value from keys where key = ?";
    static database_stmt stmt = db::prepare(db, sql.c_str());
    sqlite3_reset(stmt.get());
    sqlite3_bind_text(stmt.get(), 1, key.c_str(), -1, NULL);
    if ( sqlite3_step(stmt.get()) == SQLITE_ROW )
    {
        std::stringstream val;
        val << sqlite3_column_text(stmt.get(), 0);
        return val.str();
    }
    return "";
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

    stmt = db::prepare(super_db, "select * from keys");

    printf("Got results:\n");
    int step_result = 0;
    for ( step_result = sqlite3_step(stmt.get());
          step_result == SQLITE_ROW;
          step_result = sqlite3_step(stmt.get()))
    {
        printf("Step result: %s (%d)\n",
                sqlite3_errstr(step_result),
                step_result);
        int num_cols = sqlite3_column_count(stmt.get());

        for (int i = 0; i < num_cols; i++)
        {
            switch (sqlite3_column_type(stmt.get(), i))
            {
            case (SQLITE3_TEXT):
                printf("%s, ", sqlite3_column_text(stmt.get(), i));
                break;
            case (SQLITE_INTEGER):
                printf("%d, ", sqlite3_column_int(stmt.get(), i));
                break;
            case (SQLITE_FLOAT):
                printf("%g, ", sqlite3_column_double(stmt.get(), i));
                break;
            default:
                break;
            }
        }
        printf("\n");

    }
    printf("Step result: %s (%d)\n",
            sqlite3_errstr(step_result),
            step_result);

    std::cout << "firstname: " << get_value(super_db, "firstname") << std::endl;
    std::cout << "lastname: " << get_value(super_db, "lastname") << std::endl;

    return 0;
}
