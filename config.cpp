
#include <config.hpp>
#include <sstream>
#include <iostream>

config::config()
{
};

config::config(std::string database_path)
{
    int open_result = sqlite3_open(database_path.c_str(), &database_);
    if ( open_result != 0 )
    {
        std::stringstream error;
        error << "Failed to construct config: "
              << open_result
              << " : "
              << sqlite3_errmsg(database_)
              << " : "
              << database_path;

        sqlite3_close(database_);
        throw std::runtime_error(error.str());
    }
}

config::~config()
{
    sqlite3_close(database_);
}

static int set_value_callback(void* void_value,
                              int columns,
                              char** values,
                              char** column_names)
{
    std::string* value = static_cast<std::string*>(void_value);
    *value = values[0];
    return 0;
}

std::string config::read(const std::string& name) const
{
    std::stringstream command;
    command << "SELECT value FROM config_table WHERE name='"
            << name
            << "'";
    std::string value;
    char* error = NULL;
    int result = sqlite3_exec(database_,
                              command.str().c_str(),
                              set_value_callback,
                              static_cast<void*>(&value), &error);
    if ( result != SQLITE_OK )
    {
        std::cout << error << std::endl;
        sqlite3_free(error);
    }

    return value;
}

bool config::save(const std::string& name, const std::string& value)
{
    std::stringstream command;
    command << "INSERT INTO config_table (name, value) "
            << "VALUES ('" << name << "', '" << value << "')";
    char* error = NULL;
    int result = sqlite3_exec(database_,
                              command.str().c_str(),
                              NULL,
                              NULL,
                              &error);
    if ( result != SQLITE_OK )
    {
        std::cout << error << std::endl;
        sqlite3_free(error);
        return false;
    }

    return true;
}
