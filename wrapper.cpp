#include "wrapper.h"
#include "src/db/pdm_database.h"
#include <cstring>

extern "C" {

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

}
