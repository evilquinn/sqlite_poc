
#include <string>
#include <sqlite3.h>

class config
{
public:
    config();
    config(std::string database_path);
    virtual ~config();

    bool save(const std::string& name, const std::string& value);
    std::string read(const std::string& name) const;
    int backup_or_recover(const std::string& backup_path, bool backup);
    std::ostream& print_config(std::ostream& os) const;

private:

    static int set_value_callback(void* void_value,
                                  int columns,
                                  char** values,
                                  char** column_names);
    static int print_config_callback(void* void_os,
                                  int columns,
                                  char** values,
                                  char** column_names);
    sqlite3* database_;
};

inline std::ostream& operator<<(std::ostream& os, const config& obj)
{
    // write obj to stream
    return obj.print_config(os);
}


