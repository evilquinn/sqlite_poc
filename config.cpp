
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

/*
** This function is used to load the contents of a database file on disk 
** into the "main" database of open database connection pInMemory, or
** to save the current contents of the database opened by pInMemory into
** a database file on disk. pInMemory is probably an in-memory database, 
** but this function will also work fine if it is not.
**
** Parameter zFilename points to a nul-terminated string containing the
** name of the database file on disk to load from or save to. If parameter
** isSave is non-zero, then the contents of the file zFilename are 
** overwritten with the contents of the database opened by pInMemory. If
** parameter isSave is zero, then the contents of the database opened by
** pInMemory are replaced by data loaded from the file zFilename.
**
** If the operation is successful, SQLITE_OK is returned. Otherwise, if
** an error occurs, an SQLite error code is returned.
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
    if( rc==SQLITE_OK ){

        /* If this is a 'load' operation (isSave==0), then data is copied
        ** from the database file just opened to database pInMemory. 
        ** Otherwise, if this is a 'save' operation (isSave==1), then data
        ** is copied from pInMemory to pFile.  Set the variables pFrom and
        ** pTo accordingly. */
        source_db = (backup ? backup_file : database_);
        sink_db   = (backup ? database_   : backup_file);

        /* Set up the backup procedure to copy from the "main" database of 
        ** connection pFile to the main database of connection pInMemory.
        ** If something goes wrong, pBackup will be set to NULL and an error
        ** code and message left in connection pTo.
        **
        ** If the backup object is successfully created, call backup_step()
        ** to copy data from pFile to pInMemory. Then call backup_finish()
        ** to release resources associated with the pBackup object.  If an
        ** error occurred, then an error code and message will be left in
        ** connection pTo. If no error occurred, then the error code belonging
        ** to pTo is set to SQLITE_OK.
        */
        backup_ctx = sqlite3_backup_init(sink_db, "main", source_db, "main");
        if( backup_ctx )
        {
            (void)sqlite3_backup_step(backup_ctx, -1);
            (void)sqlite3_backup_finish(backup_ctx);
        }
        rc = sqlite3_errcode(sink_db);
    }

    /* Close the database connection opened on database file zFilename
    ** and return the result of this function. */
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
