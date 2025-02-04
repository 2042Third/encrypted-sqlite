#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "wrapper.h"

// Test helper functions
void assert_str_eq(const char* actual, const char* expected, const char* message) {
  if (strcmp(actual, expected) != 0) {
    printf("FAIL: %s\nExpected: %s\nGot: %s\n", message, expected, actual);
    assert(0);
  }
}

void assert_int_eq(int actual, int expected, const char* message) {
  if (actual != expected) {
    printf("FAIL: %s\nExpected: %d\nGot: %d\n", message, expected, actual);
    assert(0);
  }
}

void assert_not_null(void* ptr, const char* message) {
  if (ptr == NULL) {
    printf("FAIL: %s\nExpected non-NULL pointer\n", message);
    assert(0);
  }
}

// Individual test cases
void test_database_open_close() {
  printf("Testing database open/close...\n");
  const char* password = "test_password";
  PDMDatabase* db = pdm_db_open("./test.db", password, strlen(password));
  assert_not_null(db, "Database should open successfully");

  pdm_db_close(db, "./test.db");
  printf("Database open/close test passed\n");
}

void test_basic_crud_operations() {
  printf("Testing basic CRUD operations...\n");
  const char* password = "test_password";
  PDMDatabase* db = pdm_db_open("./test.db", password, strlen(password));
  assert_not_null(db, "Database should open successfully");

  // Create table
  int result = pdm_db_execute(db, "CREATE TABLE IF NOT EXISTS test_table (id INTEGER PRIMARY KEY, name TEXT)");
  assert_int_eq(result, 1, "Create table should succeed");

  // Insert data
  result = pdm_db_execute(db, "INSERT INTO test_table (name) VALUES ('John Doe')");
  assert_int_eq(result, 1, "Insert should succeed");

  // Select data
  result = pdm_db_execute(db, "SELECT * FROM test_table");
  assert_int_eq(result, 1, "Select should succeed");

  PDMReturnTable* table = pdm_db_get_result(db);
  assert_not_null(table, "Result table should not be NULL");

  // Verify results
  assert_int_eq(pdm_db_get_row_count(table), 1, "Should have 1 row");
  assert_int_eq(pdm_db_get_column_count(table), 2, "Should have 2 columns");
  assert_str_eq(pdm_db_get_column_name(table, 1), "name", "Second column should be 'name'");
  assert_str_eq(pdm_db_get_cell_data(table, 0, 1), "John Doe", "First row name should be 'John Doe'");

  // Clean up
  pdm_db_free_result(table);
  pdm_db_execute(db, "DROP TABLE test_table");
  pdm_db_close(db, "test.db");

  printf("CRUD operations test passed\n");
}

void test_multiple_rows() {
  printf("Testing multiple rows handling...\n");
  const char* password = "test_password";
  PDMDatabase* db = pdm_db_open("./test.db", password, strlen(password));
  assert_not_null(db, "Database should open successfully");

  pdm_db_execute(db, "CREATE TABLE IF NOT EXISTS multi_test (id INTEGER PRIMARY KEY, value TEXT)");
  pdm_db_execute(db, "INSERT INTO multi_test (value) VALUES ('First')");
  pdm_db_execute(db, "INSERT INTO multi_test (value) VALUES ('Second')");
  pdm_db_execute(db, "INSERT INTO multi_test (value) VALUES ('Third')");

  pdm_db_execute(db, "SELECT * FROM multi_test ORDER BY id");
  PDMReturnTable* table = pdm_db_get_result(db);
  assert_not_null(table, "Result table should not be NULL");

  assert_int_eq(pdm_db_get_row_count(table), 3, "Should have 3 rows");
  assert_str_eq(pdm_db_get_cell_data(table, 0, 1), "First", "First row value");
  assert_str_eq(pdm_db_get_cell_data(table, 1, 1), "Second", "Second row value");
  assert_str_eq(pdm_db_get_cell_data(table, 2, 1), "Third", "Third row value");

  pdm_db_free_result(table);
  pdm_db_execute(db, "DROP TABLE multi_test");
  pdm_db_close(db, "test.db");

  printf("Multiple rows test passed\n");
}

void test_error_handling() {
  printf("Testing error handling...\n");
  const char* password = "test_password";
  PDMDatabase* db = pdm_db_open("./test.db", password, strlen(password));
  assert_not_null(db, "Database should open successfully");

  // Test invalid SQL
  int result = pdm_db_execute(db, "INVALID SQL STATEMENT");
  assert_int_eq(result, 0, "Invalid SQL should return 0");

  // Test invalid table access
  result = pdm_db_execute(db, "SELECT * FROM nonexistent_table");
  assert_int_eq(result, 0, "Select from nonexistent table should return 0");

  pdm_db_close(db, "test.db");
  printf("Error handling test passed\n");
}

int main() {
  printf("Starting test suite...\n\n");

  test_database_open_close();
  test_basic_crud_operations();
  test_multiple_rows();
  test_error_handling();

  printf("\nAll tests passed successfully!\n");
  return 0;
}
