#ifndef PDM_DATABASE_WRAPPER_H
#define PDM_DATABASE_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque types
typedef struct PDMDatabase PDMDatabase;
typedef struct PDMReturnTable {
  int argc;
  char*** argv;        // 2D array of strings
  char** col_names;    // Array of column names
  int row_count;       // Number of rows
  int col_count;       // Number of columns
} PDMReturnTable;

// Database operations
PDMDatabase* pdm_db_open(const char* path, const char* password, int password_len);
int pdm_db_execute(PDMDatabase* db, const char* sql);
void pdm_db_close(PDMDatabase* db, char* path);

// Return table operations
PDMReturnTable* pdm_db_get_result(PDMDatabase* db);
void pdm_db_free_result(PDMReturnTable* table);

// Getters for return table data
const char* pdm_db_get_cell_data(PDMReturnTable* table, int row, int col);
const char* pdm_db_get_column_name(PDMReturnTable* table, int col);
int pdm_db_get_row_count(PDMReturnTable* table);
int pdm_db_get_column_count(PDMReturnTable* table);

#ifdef __cplusplus
}
#endif

#endif // PDM_DATABASE_WRAPPER_H
