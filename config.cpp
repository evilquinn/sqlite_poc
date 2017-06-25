
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

bool config::backup(const std::string& backup_path)
{
    int rc;                     /* Function return code */
    sqlite3* backup_handle;     /* Database connection opened on zFilename */
    sqlite3_backup* backup_db;  /* Backup handle used to copy data */

    /* Open the database file identified by zFilename. */
    rc = sqlite3_open(backup_path.c_str(), &backup_handle);
    if( rc == SQLITE_OK )
    {
        /* Open the sqlite3_backup object used to accomplish the transfer */
        backup_db = sqlite3_backup_init(backup_handle, "main", database_, "main");
        if( backup_db )
        {

          /* Each iteration of this loop copies 5 database pages from database
          ** pDb to the backup database. If the return value of backup_step()
          ** indicates that there are still further pages to copy, sleep for
          ** 250 ms before repeating. */
          do
          {
              rc = sqlite3_backup_step(backup_db, 5);
              if( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED ){
                  sqlite3_sleep(250);
              }
          }
          while( rc==SQLITE_OK || rc==SQLITE_BUSY || rc==SQLITE_LOCKED );

          /* Release resources allocated by backup_init(). */
          sqlite3_backup_finish(backup_db);
        }
        rc = sqlite3_errcode(backup_handle);
        return true;
    }
    return false;
}
