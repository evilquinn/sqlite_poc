
#include <iostream>
#include <sqlite3.h>
#include <config.hpp>

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;

  try
  {
      config c("/home/evilquinn/git/sqlite_poc/build/config.db");
      std::cout << "read host: " << c.read("host") << std::endl;
      c.save("name2", "value2");
      c.save("name3", "value3");
      c.save("name4", "value4");
      c.save("name5", "value5");
      c.save("name5", "value5");
      std::cout << "read value2: " << c.read("value2") << std::endl;
      std::cout << "read value3: " << c.read("value3") << std::endl;
      std::cout << "read value4: " << c.read("value4") << std::endl;

  }
  catch ( std::runtime_error e )
  {
      std::cout << e.what() << std::endl;
      return 1;
  }


  if( argc!=3 ){
    fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
    return(1);
  }
  rc = sqlite3_open(argv[1], &db);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return(1);
  }
  rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  sqlite3_close(db);
  return 0;
}
