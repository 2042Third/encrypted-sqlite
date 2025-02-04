#ifdef __cplusplus
extern "C" {
#endif

typedef void* PDMDatabase;

PDMDatabase* pdm_db_open(const char* path, const char* password, int password_len);
int pdm_db_execute(PDMDatabase* db, const char* sql);
void pdm_db_close(PDMDatabase* db, char* path);

#ifdef __cplusplus
}
#endif