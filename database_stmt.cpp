
#include <sstream>
#include <boost/range/irange.hpp>
#include <map>
#include <database_stmt.hpp>

database_stmt::database_stmt(database& db,
                             const std::string& statement):
    db_stmt_(NULL, sqlite3_finalize)
{
    sqlite3_stmt* temp_handle;
    int prepare_result = sqlite3_prepare_v2(db.get(),
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
               << database::error_string(db.get(), prepare_result);
        throw std::runtime_error(errstr.str());
    }
}

int database_stmt::execute(row_callback callback)
{
    int step_result;
    do
    {
        step_result = sqlite3_step(db_stmt_.get());
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
                                        sqlite3_column_count(db_stmt_.get())) )
            {
                std::stringstream val;
                val << sqlite3_column_text(db_stmt_.get(), i);
                column_values[sqlite3_column_name(db_stmt_.get(), i)] =
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
                   << sqlite3_sql(db_stmt_.get())
                   << "\n"
                   << database::error_string(sqlite3_db_handle(db_stmt_.get()),
                                             step_result);
            throw std::runtime_error(errstr.str());
        } // end default
        } // end switch
    }
    while ( step_result == SQLITE_ROW );

    return step_result;
}
