
#include <config.hpp>
#include <sstream>
#include <iostream>
#include <database_stmt.hpp>

config::config(std::string database_path):
    db_(std::make_shared<database>(database_path))
{
    init_table();
}

void config::init_table()
{
    static database_stmt stmt(*(db_.get()),
        "CREATE TABLE IF NOT EXISTS config_table ( "
        "name text PRIMARY KEY, value text NOT NULL );");
    stmt.execute();
}

std::string config::read(const std::string& name) const
{
    static database_stmt stmt(
        *(db_.get()),
        "select value from config_table where name = $name");

    database::column_value_pairs cvp =
    {
        { "$name", name }
    };

    std::string value;

    stmt.execute(cvp, [&](const database::column_value_pairs& cvp)
    {
        value = cvp.begin()->second;
    });
    return value;
}

void config::save(const std::string& name, const std::string& value)
{
    static database_stmt stmt(
        *(db_.get()),
        "INSERT OR REPLACE INTO config_table ( name, value ) "
        "VALUES ( $name, $value )");

    const database::column_value_pairs cvp =
    {
        { "$name",  name  },
        { "$value", value }
    };

    stmt.execute(cvp);
}

std::ostream& config::print_config(std::ostream& os) const
{
    static database_stmt stmt(
        *(db_.get()),
        "SELECT * FROM config_table");

    bool print_headers = true;
    stmt.execute([&](const database::column_value_pairs& cvp)
    {
        if ( print_headers )
        {
            print_headers = false;
            for ( const auto& e : cvp )
            {
                os << " | " << e.first;
            }
            os << " |\n";
        }

        for ( const auto& e : cvp )
        {
            os << " | " << e.second;
        }
        os << " |\n";
    });

    return os;
}