//
// Created by Mike Yang on 11/25/2022.
//

#ifndef PDM_PLATFORMS_UI_PDM_DATABASE_H
#define PDM_PLATFORMS_UI_PDM_DATABASE_H

#include "sqlite3.h"
#include "cryptosqlite/cryptosqlite.h"

void test_encryption();

namespace PDM {
class pdm_database {
public:
  // static int callback(void *ptr, int argc, char **argv, char **azColName);
  struct return_table{
    int argc=0;
    std::vector<std::vector<std::string> > argv;
    std::vector<std::string> col_name;
  };
  pdm_database();
  ~pdm_database();

  bool test_connection() const;

  int open_db(const char* name, const char*pas,int pas_size);
  int close_db(char* name);
  int execute(const char *input, int (*callback)(void*, int, char**, char**) = nullptr);
  int execute_full(const char *input,
                    int (*callback)(void*, int, char**, char**) ,
                    void* ptr,
                    char** errmsg,
                    size_t errmsg_len
                    );


  static void reset (return_table* a) {
    a->argc=0;
    a->argv.clear();
    a->col_name.clear();
  }
  static std::string getStrReturn(sqlite3_stmt *stmt, int i);

  sqlite3 *db{};
  cryptosqlite enc_db;
  char *zErrMsg ;
  int rc;
  return_table current_display_table;
  std::string last_command ;
  int status_open = 0;

  const char *local_table_create_query = "CREATE TABLE IF NOT EXISTS pdm_local("
    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "last_time_open DATETIME DEFAULT CURRENT_TIMESTAMP,"
    "data TEXT );";

  // Helper function to copy error message with limit
  void copy_error_msg(char* dest, const char* src, size_t max_len);
};

}


#endif //PDM_PLATFORMS_UI_PDM_DATABASE_H
