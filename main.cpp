//
// Created by Mike Yang on 2/3/25.
//

#include <iostream>
#include "src/db/pdm_database.h"
#include <cassert>
#include <iostream>
#include <cstring>

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
  std::cout << "In main\n" << std::endl;
  PDMDatabaseTests tests;
  tests.runAllTests();
  return 0;
}
