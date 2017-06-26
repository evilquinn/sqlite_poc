
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

int config::set_value_callback(void* void_value,
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
    if ( result == SQLITE_CONSTRAINT )
    {
       sqlite3_free(error);
       std::stringstream update_command;
       update_command << "UPDATE config_table SET value = '"
                      << value
                      << "' WHERE name = '"
                      << name
                      << "'";
       result = sqlite3_exec(database_,
                             update_command.str().c_str(),
                             NULL,
                             NULL,
                             &error);
    }
    if ( result != SQLITE_OK )
    {
        std::cout << error << std::endl;
        sqlite3_free(error);
        return false;
    }

    return true;
}

/*
 * Either backup to or recover from specified file
 *
 * If backup is true, save to file, else load from file
*/
int config::backup_or_recover(const std::string& backup_path, bool backup)
{
    int rc;                   /* Function return code */
    sqlite3* backup_file;     /* Database connection opened on backup_path */
    sqlite3_backup* backup_ctx;  /* Backup object used to copy data */
    sqlite3* sink_db;         /* DB to copy to (database_ or backup_file) */
    sqlite3* source_db;       /* DB to copy from (database_ or backup_file) */

    /* Open the database file identified by zFilename. Exit early if this fails
    ** for any reason. */
    rc = sqlite3_open(backup_path.c_str(), &backup_file);
    if( rc==SQLITE_OK )
    {
        // backing up or recovering?
        source_db = (backup ? database_   : backup_file);
        sink_db   = (backup ? backup_file : database_);

        // copy from source to sink
        backup_ctx = sqlite3_backup_init(sink_db, "main", source_db, "main");
        if( backup_ctx )
        {
            // in a single step, no paging
            (void)sqlite3_backup_step(backup_ctx, -1);
            (void)sqlite3_backup_finish(backup_ctx);
        }
        rc = sqlite3_errcode(sink_db);
    }
    (void)sqlite3_close(backup_file);
    return rc;
}

int config::print_config_callback(void*  void_os,
                                  int    columns,
                                  char** values,
                                  char** column_names)
{
    std::ostream* os = static_cast<std::ostream*>(void_os);
    for ( int i = 0; i < columns; i++ )
    {
        *os << column_names[i] << " = " << values[i] << std::endl;
    }
    *os << std::endl;
    return 0;
}


std::ostream& config::print_config(std::ostream& os) const
{
    std::stringstream command;
    command << "SELECT * FROM config_table";
    char* error = NULL;
    int result = sqlite3_exec(database_,
                              command.str().c_str(),
                              print_config_callback,
                              static_cast<void*>(&os), &error);
    if ( result != SQLITE_OK )
    {
        os << "\n" << error << std::endl;
        sqlite3_free(error);
    }

    return os;
}
