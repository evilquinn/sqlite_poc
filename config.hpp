
#include <string>
#include <database.hpp>

class config
{
public:
    config(std::string database_path);

    void save(const std::string& name, const std::string& value);
    std::string read(const std::string& name) const;
    std::ostream& print_config(std::ostream& os) const;

private:
    void init_table();
    std::shared_ptr<database> db_;
};

inline std::ostream& operator<<(std::ostream& os, const config& obj)
{
    // write obj to stream
    return obj.print_config(os);
}


