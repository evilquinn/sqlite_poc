
#include <iostream>
#include <config.hpp>

int main(int argc, char* argv[])
{
  try
  {
      config c("/home/evilquinn/git/sqlite_poc/build/config.db");
      std::cout << "read host: " << c.read("host") << std::endl;
      c.save("name2", "vali2");
      int pause = 0;
      std::cin >> pause;
      pause = 0;
      c.save("name3", "vali3");
      std::cin >> pause;
      pause = 0;
      c.save("name4", "vali4");
      std::cin >> pause;
      pause = 0;
      c.save("name5", "vali5");
      std::cin >> pause;
      std::cout << "read value2: " << c.read("name2") << std::endl;
      pause = 0;
      std::cout << "read value3: " << c.read("name3") << std::endl;
      std::cout << "read name5: " << c.read("name5") << std::endl;
      c.save("name5", "helilo!");
      std::cin >> pause;
      pause = 0;
      std::cout << "read name5: " << c.read("name5") << std::endl;
      std::cin >> pause;
      pause = 0;

      std::cout << "FULL DUMP:" << std::endl << c << std::endl;

      c.backup_or_recover("/home/evilquinn/git/sqlite_poc/build/config.db.bk2",
                          true);

      c.save("name3", "niall");
      c.save("name4", "is");
      c.save("name5", "king");

      std::cout << "FULL DUMP:" << std::endl << c << std::endl;

      c.backup_or_recover("/home/evilquinn/git/sqlite_poc/build/config.db.bk2",
                          false);

      std::cout << "FULL DUMP:" << std::endl << c << std::endl;

  }
  catch ( std::runtime_error& e )
  {
      std::cout << e.what() << std::endl;
      return 1;
  }

  return 0;
}
