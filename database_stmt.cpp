
#include <sstream>
#include <boost/range/irange.hpp>
#include <map>
#include <database_stmt.hpp>

database_stmt::database_stmt(database& db,
                             const std::string& statement):
    db_stmt_(NULL, sqlite3_finalize)
{
    sqlite3_stmt* temp_handle;
    int prepare_result = sqlite3_prepare_v2(db.handle(),
                                            statement.c_str(),
                                            statement.size()+1,
                                            &temp_handle,
                                            NULL);
    db_stmt_ = std::move(db_stmt(temp_handle, sqlite3_finalize));
    if ( prepare_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Error preparing statement:\n"
               << statement << "\n"
               << database::error_string(db.handle(), prepare_result);
        throw std::runtime_error(errstr.str());
    }
}

void database_stmt::execute(row_callback callback)
{
    int reset_result = sqlite3_reset(handle());
    if ( reset_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Failed to reset database_stmt:\n"
               << database::error_string(sqlite3_db_handle(handle()),
                                         reset_result);
        throw std::runtime_error(errstr.str());
    }

    int step_result;
    do
    {
        step_result = sqlite3_step(handle());
        switch ( step_result )
        {
        case SQLITE_ROW :
        {
            if ( !callback )
            {
                // don't bother if there's nothing to call
                break;
            }
            database::column_value_pairs column_values;
            for ( int i : boost::irange(0,
                                        sqlite3_column_count(handle())) )
            {
                std::stringstream val;
                val << sqlite3_column_text(handle(), i);
                column_values[sqlite3_column_name(handle(), i)] =
                    val.str();
            }
            callback(column_values);
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
                   << sqlite3_sql(handle())
                   << "\n"
                   << database::error_string(sqlite3_db_handle(handle()),
                                             step_result);
            throw std::runtime_error(errstr.str());
        } // end default
        } // end switch
    }
    while ( step_result == SQLITE_ROW );
}

void database_stmt::execute(const database::column_value_pairs& cvp,
                            row_callback callback)
{
    int reset_result = sqlite3_reset(handle());
    if ( reset_result != SQLITE_OK )
    {
        std::stringstream errstr;
        errstr << "Failed to reset database_stmt:\n"
               << database::error_string(sqlite3_db_handle(handle()),
                                         reset_result);
        throw std::runtime_error(errstr.str());
    }

    for ( const auto& e : cvp )
    {
        int find_result = sqlite3_bind_parameter_index(handle(),
                                                       e.first.c_str());
        if ( find_result == 0 )
        {
            std::stringstream errstr;
            errstr << "Failed to find parameter " << e.first << " in statement:\n"
                   << sqlite3_sql(handle());
            throw std::runtime_error(errstr.str());
        }
        int bind_result = sqlite3_bind_text(handle(),
                                            find_result,
                                            e.second.c_str(),
                                            e.second.size(),
                                            NULL);
        if ( bind_result != SQLITE_OK )
        {
            std::stringstream errstr;
            errstr << "Failed to bind "
                   << e.second
                   << " to parameter "
                   << e.first
                   << " in stmt :\n"
                   << sqlite3_sql(handle()) << " :\n"
                   << database::error_string(sqlite3_db_handle(handle()), bind_result);
            throw std::runtime_error(errstr.str());
        }
    }

    this->execute(callback);
}
