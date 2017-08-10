
#include <iostream>
#include <sstream>
#include <sqlite3.h>
#include <memory>
#include <thread>
#include <mutex>
#include <boost/range/irange.hpp>
#include <boost/bind.hpp>
#include <map>
#include <functional>
#include <database.hpp>
#include <database_stmt.hpp>
#include <config.hpp>


void my_callback(std::string& value, const database::column_value_pairs& cvp)
{
    value = cvp.begin()->second;
}

std::string get_value(database& db,
                      const std::string& key)
{
    // one-time static init
    static std::string sql = "select value from keys where key = $key";
    static database_stmt stmt(db, sql.c_str());
    static std::mutex stmt_mutex;

    // get a lock so no one else blatters the shared prepared statement
    std::lock_guard<std::mutex> stmt_lock(stmt_mutex);

    database::column_value_pairs cvp =
    {
        { "$key", key }
    };
    std::string value;
    stmt.execute(cvp, boost::bind(::my_callback, std::ref(value), _1));
//    db::execute(stmt, [&](const db::column_value_pairs& cvp)
//    {
//        value = cvp.begin()->second;
//    });

    return value;
}

void do_get_values(database& db,
                   const std::string& key)
{
    for ( int i : boost::irange(0, 5) )
    {
        std::cout << key << "(" << i << ") : "
                  << get_value(db, key) << std::endl;
    }
}


int main(int argc, char* argv[])
{
    config c("./config.db");

    c.save("one", "one");
    c.save("two", "two");

    std::cout << c << std::endl;


    database super_db("keys.db");

    const char* create_statement =
        "CREATE TABLE IF NOT EXISTS keys ("
        " key text PRIMARY KEY,"
        " value text NOT NULL );";
    const char* insert_statement =
        "INSERT OR REPLACE INTO keys ( key, value )"
        " VALUES"
        " ( 'firstname', 'Niall' ),"
        " ( 'lastname', 'Quinn' );";

    database_stmt stmt(super_db, create_statement);
    stmt.execute(nullptr);

    stmt = database_stmt(super_db, insert_statement);
    stmt.execute(NULL);

    std::cout << "firstname: " << get_value(super_db, "firstname") << std::endl;
    std::cout << "lastname: " << get_value(super_db, "lastname") << std::endl;

    std::cout << "threads:" << std::endl;

    std::thread get_firstnames(do_get_values, std::ref(super_db), "firstname");
    std::thread get_lastnames(do_get_values, std::ref(super_db), "lastname");

    get_firstnames.join();
    get_lastnames.join();

    stmt = database_stmt(super_db, "select * from keys");
    stmt.execute([](const database::column_value_pairs& cvp)
    {
        for ( const auto& e : cvp )
        {
            std::cout << e.second << ", ";
        }
        std::cout << std::endl;
    });

    return 0;
}
