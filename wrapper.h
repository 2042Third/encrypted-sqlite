#ifndef PDM_DATABASE_WRAPPER_H
#define PDM_DATABASE_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// type for int64_t
typedef long long int64_t;
// type for uint64_t
typedef unsigned long long uint64_t;

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
int pdm_db_execute_full(PDMDatabase* db, const char* sql, int (*callback)(void*, int, char**, char**), void* ptr, char** errmsg, uint64_t errmsg_len);
void pdm_db_close(PDMDatabase* db, char* path);

// Return table operations
PDMReturnTable* pdm_db_get_result(PDMDatabase* db);
void pdm_db_free_result(PDMReturnTable* table);
const char* pdm_db_get_cell_data(PDMReturnTable* table, int row, int col);
const char* pdm_db_get_column_name(PDMReturnTable* table, int col);
int pdm_db_get_row_count(PDMReturnTable* table);
int pdm_db_get_column_count(PDMReturnTable* table);

//// SQLite Fundamental Datatypes
//#define SQLITE_INTEGER  1
//#define SQLITE_FLOAT    2
//#define SQLITE_BLOB     4
//#define SQLITE_NULL     5
//#ifdef SQLITE_TEXT
//# undef SQLITE_TEXT
//#else
//# define SQLITE_TEXT     3
//#endif
//#define SQLITE3_TEXT     3

// Transaction-related functions
int pdm_db_begin(PDMDatabase* db);
int pdm_db_commit(PDMDatabase* db);
int pdm_db_rollback(PDMDatabase* db);
int pdm_db_get_autocommit(PDMDatabase* db);
const char* pdm_db_errmsg(PDMDatabase* db);
int pdm_db_last_insert_rowid(PDMDatabase* db);
int pdm_db_changes(PDMDatabase* db);


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
 * Bind a 64-bit integer value.
 */
int pdm_db_bind_int64(PDMStatement* stmt, int index, int64_t value);

/**
 * Bind a blob value.
 */
int pdm_db_bind_blob(PDMStatement* stmt, int index, const void* blob, int size);

/**
 * Bind a NULL value.
 */
int pdm_db_bind_null(PDMStatement* stmt, int index);

/**
 * Bind a double value.
 */
int pdm_db_bind_double(PDMStatement* stmt, int index, double value);

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
 * Retrieve an 64-bit integer value from the specified column.
 */
int64_t pdm_db_column_int64(PDMStatement* stmt, int col);

/**
 * Retrieve a double value from the specified column.
 */
double pdm_db_column_double(PDMStatement* stmt, int col);

/**
 * Retrieve a blob value from the specified column.
 */
const void* pdm_db_column_blob(PDMStatement* stmt, int col);

/**
 * Retrieve the name of the specified column.
 */
const char* pdm_db_column_name(PDMStatement* stmt, int col);

/**
 * Retrieve a bytes value from the specified column.
 */
int pdm_db_column_bytes(PDMStatement* stmt, int col);

/**
 * Retrieve a bytes16 value from the specified column.
 */
int pdm_db_column_bytes16(PDMStatement* stmt, int col);

/**
 * Retrieve the number of columns in the result set.
 */
int pdm_db_column_count(PDMStatement* stmt);

/**
 * Retrieve the type of the specified column.
 */
int pdm_db_column_type(PDMStatement* stmt, int col);

void pdm_db_reset(PDMStatement* stmt);

#ifdef __cplusplus
}
#endif

#endif // PDM_DATABASE_WRAPPER_H
