//
// Created by Mike Yang on 2/3/25.
//

#include <iostream>
#include "src/db/pdm_database.h"
#include <cassert>
#include <iostream>
#include <cstring>
#include <vector>

static int callback(void *ptr, int argc, char **argv, char **azColName) {
  std::cout << "=== Callback Start ===" << std::endl;
  std::cout << "Number of columns: " << argc << std::endl;

  // For CREATE TABLE statements, we might not get any rows
  if (argc == 0) {
    std::cout << "No columns returned (typical for CREATE TABLE)" << std::endl;
    std::cout << "=== Callback End ===" << std::endl;
    return 0;
  }

  try {
    if (ptr) {
      auto* table = static_cast<PDM::pdm_database::return_table*>(ptr);

      // Print column names
      std::cout << "Column names:" << std::endl;
      for (int i = 0; i < argc; i++) {
        std::cout << (azColName[i] ? azColName[i] : "NULL") << "\t";
      }
      std::cout << std::endl;

      // Print values
      std::cout << "Values:" << std::endl;
      for (int i = 0; i < argc; i++) {
        std::cout << (argv[i] ? argv[i] : "NULL") << "\t";
      }
      std::cout << std::endl;

      // Store in return table structure
      if (table->argc == 0) {
        table->argc = argc;
        for (int f = 0; f < argc; f++) {
          table->col_name.push_back(azColName[f] ? azColName[f] : "");
        }
      }

      std::vector<std::string> row;
      for (int i = 0; i < argc; i++) {
        row.push_back(argv[i] ? argv[i] : "");
      }
      table->argv.push_back(std::move(row));
    } else {
      std::cout << "Null pointer passed to callback" << std::endl;
    }
  }
  catch (const std::exception& e) {
    std::cerr << "Exception in callback: " << e.what() << std::endl;
    return 1;  // Return non-zero to indicate error
  }
  catch (...) {
    std::cerr << "Unknown exception in callback" << std::endl;
    return 1;
  }

  std::cout << "=== Callback End ===" << std::endl;
  return 0;
}

class PDMDatabaseTests {
private:
  PDM::pdm_database db;
  const char* TEST_DB_PATH = "./test_db.sqlite";
  const char* TEST_DB_KEYFILE_PATH = "./test_db.sqlite-keyfile";
  const char* TEST_PASSWORD = "test_password";

  void printTestResult(const char* testName, bool success) {
    std::cout << testName << ": " << (success ? "PASSED" : "FAILED") << std::endl;
  }

  bool testDatabaseOpen() {
    bool success = db.open_db(TEST_DB_PATH, TEST_PASSWORD, strlen(TEST_PASSWORD));
    printTestResult("Database Open Test", success);
    return success;
  }

  bool testCreateTable() {
    std::cout << "About to create table..." << std::endl;
    auto createTableSQL =
        "CREATE TABLE IF NOT EXISTS test_table ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "value INTEGER"
        ");";

    std::cout << "Executing SQL: " << createTableSQL << std::endl;
    bool success = false;
    try {
      success = db.execute(createTableSQL);
    } catch (const std::exception& e) {
      std::cout << "Exception during create table: " << e.what() << std::endl;
      return false;
    }
    std::cout << "Create table execution completed" << std::endl;
    printTestResult("Create Table Test", success);
    return success;
  }


  bool testInsertData() {
    const char* insertSQL =
      "INSERT INTO test_table (name, value) VALUES ('test1', 100);";
    bool success = db.execute(insertSQL);
    printTestResult("Insert Data Test", success);
    return success;
  }

  bool testSelectData() {
    const char* selectSQL = "SELECT * FROM test_table WHERE name='test1';";
    bool success = db.execute(selectSQL);

    // Verify the returned data
    if (success) {
      auto& table = db.current_display_table;  // Changed from get_current_table()
      success = !table.argv.empty() &&
                table.argv[0].size() == 3 &&
                table.argv[0][1] == "test1" &&
                table.argv[0][2] == "100";
    }

    printTestResult("Select Data Test", success);
    return success;
  }

  bool testDatabaseClose() {
    bool success = db.close_db(const_cast<char*>(TEST_DB_PATH));
    printTestResult("Database Close Test", success);
    return success;
  }

  void cleanup() {
    // Remove the test database file
    std::remove(TEST_DB_PATH);
    std::remove(TEST_DB_KEYFILE_PATH);
  }

public:
  void runAllTests() {
    std::cout << "Starting PDM Database Tests...\n" << std::endl;

    bool allTestsPassed = true;

    // Run tests in sequence
    allTestsPassed &= testDatabaseOpen();
    allTestsPassed &= testCreateTable();
    allTestsPassed &= testInsertData();
    allTestsPassed &= testSelectData();
    allTestsPassed &= testDatabaseClose();

    std::cout << "\nTest Summary:" << std::endl;
    std::cout << "All tests " << (allTestsPassed ? "PASSED" : "FAILED") << std::endl;

    // Clean up after tests
    cleanup();
  }
};




// Usage example:
int main() {
    // test_encryption();
  PDMDatabaseTests tests;
  tests.runAllTests();
  return 0;

    std::cout << "=== Database Test Start ===" << std::endl;

    PDM::pdm_database db;
    if (!db.open_db("./test.db", "password", 8)) {
        std::cerr << "Failed to open database" << std::endl;
        return 1;
    }
    std::cout << "Database opened successfully" << std::endl;

    // Test connection
    if (!db.test_connection()) {
        std::cerr << "Database connection test failed!" << std::endl;
        return 1;
    }
    std::cout << "Connection test passed" << std::endl;

    // Create table
    const char* create_query = "CREATE TABLE IF NOT EXISTS test_table ("
                              "id INTEGER PRIMARY KEY,"
                              "name TEXT NOT NULL,"
                              "value INTEGER);";

    std::cout << "\nExecuting CREATE TABLE query..." << std::endl;
    if (!db.execute(create_query, callback)) {
        std::cerr << "Create table failed!" << std::endl;
        return 1;
    }
    std::cout << "Table created successfully" << std::endl;

    // Insert test data
    const char* insert_query = "INSERT INTO test_table (name, value) VALUES ('test', 123);";
    std::cout << "\nExecuting INSERT query..." << std::endl;
    if (!db.execute(insert_query, callback)) {
        std::cerr << "Insert failed!" << std::endl;
        return 1;
    }

    // Select data
    const char* select_query = "SELECT * FROM test_table;";
    std::cout << "\nExecuting SELECT query..." << std::endl;
    if (!db.execute(select_query, callback)) {
        std::cerr << "Select failed!" << std::endl;
        return 1;
    }

    std::cout << "=== Database Test End ===" << std::endl;
    return 0;
}

