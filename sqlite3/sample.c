#include <sqlite3.h>
#include <stdio.h>

sqlite3 *db;
int rc = sqlite3_open("test.db", &db);

if (rc != SQLITE_OK) {
    printf("Cannot open DB: %s\n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 1;
}
