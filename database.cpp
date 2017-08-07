
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

database::database(const std::string& filename)
{
    sqlite3* temp_handle;
    int open_result = sqlite3_open(filename.c_str(), &temp_handle);
    db_ = db(temp_handle, sqlite3_close_v2);
    if ( open_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error opening: " << filename
               << "\n"
               << error_string(db_.get(), open_result);
        throw std::runtime_error(errstr.str());
    }
}
