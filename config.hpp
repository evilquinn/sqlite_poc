
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

private:
    sqlite3* database_;
};

