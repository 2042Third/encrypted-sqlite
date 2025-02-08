//
// Created by Mike Yang on 11/25/2022.
//

#include "pdm_database.h"
#include "crypto/pdmCryptoDB.hpp"
#include <memory>
#include <string>
#include <filesystem>
#include <thread>

namespace PDM {
  // static int callback(void *ptr, int argc, char **argv, char **azColName) {
  //   std::cout << "Callback entered with " << argc << " columns" << std::endl;
  //   using tbl_type = PDM::pdm_database::return_table;
  //   auto* table = static_cast<tbl_type*>(ptr);
  //   try {
  //
  //     int i;
  //     std::vector<std::string> tmp;
  //     if(table->argc==0){ // first time recording
  //       table->argc = argc;
  //       for (unsigned f =0 ;f<argc;f++) {
  //         if (azColName[f]) table->col_name.push_back(std::move(std::string(azColName[f])));
  //         else table->col_name.push_back(std::move(std::string("")));
  //       }
  //     }
  //     for (i = 0; i < argc; i++) {
  //       if (argv[i]) tmp.push_back(std::move(std::string(argv[i])));
  //       else tmp.push_back(std::move(std::string("")));
  //     }
  //     table->argv.push_back(std::move(tmp));
  //     std::cout << "Callback completed successfully" << std::endl;
  //   }
  //   catch (const std::exception& e) {
  //     std::cerr << "Error in callback: " << e.what() << std::endl;
  //   }
  //   return 0;
  // }

  pdm_database::pdm_database() {
  }

  pdm_database::~pdm_database() {

  }
}

bool PDM::pdm_database::test_connection() const {
    std::cout << "Testing database connection..." << std::endl;

    const char* test_query = "SELECT 1;";
    char* err_msg = nullptr;

    std::cout << "Executing simple test query..." << std::endl;
    int test_rc = sqlite3_exec(db, test_query, nullptr, nullptr, &err_msg);

    if (test_rc != SQLITE_OK) {
        std::cerr << "Database test failed: " << (err_msg ? err_msg : "Unknown error") << std::endl;
        sqlite3_free(err_msg);
        return false;
    }

    std::cout << "Database test successful" << std::endl;
    return true;
}


/**
 * Open a database file.
 * @param name name and path of the database file
 * @param pas password
 * @param pas_size size of the password
 * Checks if the path given is good. If not, make path.
 * */
int PDM::pdm_database::open_db(const char *name, const char*pas, int pas_size) {
  namespace fs = std::filesystem;
  fs::create_directories(fs::path(name).parent_path()); // Create user config dir
  cryptosqlite::setCryptoFactory([] (std::unique_ptr<IDataCrypt> &crypt) { // set the crypto factory
    crypt = std::make_unique<pdm_crypto_db>();
  });
  rc = sqlite3_open_encrypted(name, &db, pas, pas_size); // open the encrypted database
  if( rc ){
    status_open = 0;
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 0;
  }
  status_open = 1;
  return 1;
}

int PDM::pdm_database::close_db(char *name) {
  sqlite3_close(db);
  status_open=0;
  return 1;
}

int PDM::pdm_database::execute(const char *input, int (*callback)(void*, int, char**, char**)) {
  std::cout << "Starting execute for: " << input << std::endl;
  reset(&current_display_table);

  // Check if database is valid
  if (!db) {
    std::cerr << "Database handle is null" << std::endl;
    return 0;
  }

  // Get current SQLite status
  std::cout << "SQLite status before exec: " << sqlite3_errcode(db) << std::endl;

  rc = sqlite3_exec(db, input, callback, &current_display_table, &zErrMsg);

  std::cout << "SQLite exec completed with rc: " << rc << std::endl;

  if(rc != SQLITE_OK){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return 0;
  }
  last_command = input;
  return 1;
}


std::string PDM::pdm_database::getStrReturn(sqlite3_stmt *stmt, int i) {
  return
  reinterpret_cast<const char*>((sqlite3_column_type(stmt, i)!=SQLITE_NULL
  ? sqlite3_column_text(stmt,i)
  :(const unsigned char * )"") );
}


