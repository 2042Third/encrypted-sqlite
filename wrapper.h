#ifndef PDM_DATABASE_WRAPPER_H
#define PDM_DATABASE_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// Opaque type for the database
typedef struct PDMDatabase PDMDatabase;

// The existing return table structure
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
const char* pdm_db_get_cell_data(PDMReturnTable* table, int row, int col);
const char* pdm_db_get_column_name(PDMReturnTable* table, int col);
int pdm_db_get_row_count(PDMReturnTable* table);
int pdm_db_get_column_count(PDMReturnTable* table);

// ------------------------------------------------------------------
// New: Prepared Statement API for Go usage
// ------------------------------------------------------------------

// Opaque type for prepared statements.
typedef struct PDMStatement PDMStatement;

/**
 * Prepare a SQL statement.
 * Returns a pointer to a new PDMStatement (or NULL on error).
 */
PDMStatement* pdm_db_prepare(PDMDatabase* db, const char* sql);

/**
 * Bind a text value to a prepared statement.
 * 'index' is 1-based (as in SQLite).
 */
int pdm_db_bind_text(PDMStatement* stmt, int index, const char* text);

/**
 * Bind an integer value.
 */
int pdm_db_bind_int(PDMStatement* stmt, int index, int value);

/**
 * Bind a blob value.
 */
int pdm_db_bind_blob(PDMStatement* stmt, int index, const void* blob, int size);

/**
 * Bind a NULL value.
 */
int pdm_db_bind_null(PDMStatement* stmt, int index);

/**
 * Step the prepared statement (i.e. execute one step).
 * Returns the result code (e.g. SQLITE_ROW, SQLITE_DONE, or an error code).
 */
int pdm_db_step(PDMStatement* stmt);

/**
 * Finalize (destroy) the prepared statement.
 */
int pdm_db_finalize(PDMStatement* stmt);

/**
 * Retrieve text from the specified column of the current result row.
 */
const char* pdm_db_column_text(PDMStatement* stmt, int col);

/**
 * Retrieve an integer value from the specified column.
 */
int pdm_db_column_int(PDMStatement* stmt, int col);

/**
 * Retrieve a double value from the specified column.
 */
double pdm_db_column_double(PDMStatement* stmt, int col);

/**
 * Bind a double value.
 */
int pdm_db_bind_double(PDMStatement* stmt, int index, double value);

#ifdef __cplusplus
}
#endif

#endif // PDM_DATABASE_WRAPPER_H
