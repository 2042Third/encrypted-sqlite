#include "wrapper.h"
#include "src/db/pdm_database.h"  // Your C++ database implementation
#include <cstring>
#include <cstdlib>
#include <sqlite3.h>              // Include SQLite header for prepared-statement functions

extern "C" {

// =====================
// Existing Functions
// =====================

PDMDatabase* pdm_db_open(const char* path, const char* password, int password_len) {
  auto* db = new PDM::pdm_database();
  if (db->open_db(path, password, password_len)) {
    return reinterpret_cast<PDMDatabase*>(db);
  }
  delete db;
  return nullptr;
}

int pdm_db_execute(PDMDatabase* db, const char* sql) {
  auto* pdm_db = reinterpret_cast<PDM::pdm_database*>(db);
  return pdm_db->execute(sql) ? 1 : 0;
}

void pdm_db_close(PDMDatabase* db, char* path) {
  auto* pdm_db = reinterpret_cast<PDM::pdm_database*>(db);
  pdm_db->close_db(path);
  delete pdm_db;
}

PDMReturnTable* pdm_db_get_result(PDMDatabase* db) {
  auto* pdm_db = reinterpret_cast<PDM::pdm_database*>(db);
  auto& cpp_table = pdm_db->current_display_table;

  auto* c_table = new PDMReturnTable();
  c_table->argc = cpp_table.argc;
  c_table->row_count = cpp_table.argv.size();
  c_table->col_count = cpp_table.argv.empty() ? 0 : cpp_table.argv[0].size();

  // Allocate and copy column names
  c_table->col_names = new char*[cpp_table.col_name.size()];
  for (size_t i = 0; i < cpp_table.col_name.size(); i++) {
    c_table->col_names[i] = strdup(cpp_table.col_name[i].c_str());
  }

  // Allocate and copy data
  c_table->argv = new char**[c_table->row_count];
  for (int i = 0; i < c_table->row_count; i++) {
    c_table->argv[i] = new char*[c_table->col_count];
    for (int j = 0; j < c_table->col_count; j++) {
      c_table->argv[i][j] = strdup(cpp_table.argv[i][j].c_str());
    }
  }

  return c_table;
}

void pdm_db_free_result(PDMReturnTable* table) {
  if (table) {
    // Free column names
    for (int i = 0; i < table->col_count; i++) {
      free(table->col_names[i]);
    }
    delete[] table->col_names;

    // Free data
    for (int i = 0; i < table->row_count; i++) {
      for (int j = 0; j < table->col_count; j++) {
        free(table->argv[i][j]);
      }
      delete[] table->argv[i];
    }
    delete[] table->argv;

    delete table;
  }
}

const char* pdm_db_get_cell_data(PDMReturnTable* table, int row, int col) {
  if (table && row >= 0 && row < table->row_count &&
      col >= 0 && col < table->col_count) {
    return table->argv[row][col];
  }
  return nullptr;
}

const char* pdm_db_get_column_name(PDMReturnTable* table, int col) {
  if (table && col >= 0 && col < table->col_count) {
    return table->col_names[col];
  }
  return nullptr;
}

int pdm_db_get_row_count(PDMReturnTable* table) {
  return table ? table->row_count : 0;
}

int pdm_db_get_column_count(PDMReturnTable* table) {
  return table ? table->col_count : 0;
}

// =====================
// New: Prepared Statement Functions
// =====================

/*
 * In the following functions we assume that the underlying C++ pdm_database
 * object (pointed to by PDMDatabase*) has a member (or getter) 'db' of type
 * sqlite3*. If that’s not publicly accessible, you may need to adjust your
 * C++ class to expose it (or add a dedicated prepare method).
 */

PDMStatement* pdm_db_prepare(PDMDatabase* db, const char* sql) {
  auto* pdm_db = reinterpret_cast<PDM::pdm_database*>(db);
  sqlite3_stmt* stmt = nullptr;
  // Prepare the SQL statement.
  int rc = sqlite3_prepare_v2(pdm_db->db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    return nullptr;
  }
  return reinterpret_cast<PDMStatement*>(stmt);
}

int pdm_db_bind_text(PDMStatement* stmt, int index, const char* text) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  // SQLite uses 1-based indexing for bind parameters.
  return sqlite3_bind_text(s, index, text, -1, SQLITE_TRANSIENT);
}

int pdm_db_bind_int(PDMStatement* stmt, int index, int value) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_bind_int(s, index, value);
}

int pdm_db_bind_blob(PDMStatement* stmt, int index, const void* blob, int size) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_bind_blob(s, index, blob, size, SQLITE_TRANSIENT);
}

int pdm_db_bind_null(PDMStatement* stmt, int index) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_bind_null(s, index);
}

int pdm_db_step(PDMStatement* stmt) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_step(s);
}

int pdm_db_finalize(PDMStatement* stmt) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_finalize(s);
}

const char* pdm_db_column_text(PDMStatement* stmt, int col) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return reinterpret_cast<const char*>(sqlite3_column_text(s, col));
}

int pdm_db_column_int(PDMStatement* stmt, int col) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_column_int(s, col);
}

double pdm_db_column_double(PDMStatement* stmt, int col) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_column_double(s, col);
}

int pdm_db_bind_double(PDMStatement* stmt, int index, double value) {
  sqlite3_stmt* s = reinterpret_cast<sqlite3_stmt*>(stmt);
  return sqlite3_bind_double(s, index, value);
}

} // end extern "C"
