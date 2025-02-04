#include "wrapper.h"
#include "pdm_database.h"

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
}