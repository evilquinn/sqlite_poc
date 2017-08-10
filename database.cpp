
#include <sstream>
#include <database.hpp>

std::string database::error_string(sqlite3* const db, const int error_code)
{
    std::stringstream errstr;
    errstr << "errcode: " << sqlite3_errcode(db) << "\n"
           << "ext.errcode: " << sqlite3_extended_errcode(db) << "\n"
           << "errmsg: " << sqlite3_errmsg(db) << "\n"
           << "errstr: " << sqlite3_errstr(error_code) << "\n";
    return errstr.str();
}

database::database(const std::string& filename):
    db_(NULL, sqlite3_close_v2)
{
    sqlite3* temp_handle;
    int open_result = sqlite3_open(filename.c_str(), &temp_handle);
    db_ = std::move(db(temp_handle, sqlite3_close_v2));
    if ( open_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error opening: " << filename
               << "\n"
               << error_string(db_.get(), open_result);
        throw std::runtime_error(errstr.str());
    }
}

void database::backup_to(const std::string& backup_path)
{
    return backup_or_recover(backup_path, backup);
}

void database::restore_from(const std::string& restore_from_path)
{
    return backup_or_recover(restore_from_path, restore);
}

void database::backup_or_recover(const std::string& path, sync_direction d)
{
    typedef std::unique_ptr<sqlite3_backup,
                            decltype(&sqlite3_backup_finish)> backup_ctx;

    sqlite3* sink_db;         // DB to copy to (database_ or backup_file)
    sqlite3* source_db;       // DB to copy from (database_ or backup_file)

    // Open the database file identified by zFilename. Exit early if this fails
    // for any reason.
    database other_db(path);

    // backing up or recovering?
    source_db = (d == backup ? db_.get()  : other_db.handle());
    sink_db   = (d == backup ? other_db.handle() : db_.get());

    // copy from source to sink
    backup_ctx ctx(sqlite3_backup_init(sink_db,
                                       "main",
                                       source_db,
                                       "main"),
                   sqlite3_backup_finish);
    if ( ctx )
    {
        // in a single step, no paging
        sqlite3_backup_step(ctx.get(), -1);
    }
    else
    {
        std::stringstream errstr;
        errstr << "Error creating backup_ctx: \n"
               << error_string(db_.get(), -1);
        throw std::runtime_error(errstr.str());
    }

    int sink_status = sqlite3_errcode(sink_db);
    if ( sink_status != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error executing "
               << ( d == backup ? "backup" : "restore")
               << ":\n"
               << error_string(db_.get(), sink_status);
        throw std::runtime_error(errstr.str());
    }
}