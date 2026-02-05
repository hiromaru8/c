
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

# define TB1_CREATE_SQL \
        "CREATE TABLE tb1 (" \
        "  id_src  INTEGER," \
        "  id_dest INTEGER," \
        "  data1 BLOB,"  \
        "  data2 BLOB," \
        "  PRIMARY KEY (id_src, id_dest)" \
        ");" \
        "INSERT INTO tb1 VALUES (1,1, X'010203', X'040506');" \
        "INSERT INTO tb1 VALUES (2,2, X'AABBCC', X'DDCCBB');" \
        "INSERT INTO tb1 VALUES (3,3, X'FFEE00', X'112233');" \
        "INSERT INTO tb1 VALUES (1,10, X'123456', X'654321');" \
        "INSERT INTO tb1 VALUES (2,20, X'789ABC', X'CBA987');" \
        "INSERT INTO tb1 VALUES (3,30, X'000FFF', X'FFF000');" 

#define TB2_CREATE_SQL \
        "CREATE TABLE tb2 (" \
        "  id  INTEGER," \
        "  data1 BLOB,"  \
        "  data2 BLOB," \
        "  PRIMARY KEY (id)" \
        ");" \
        "INSERT INTO tb2 VALUES (1, X'010203', X'040506');" \
        "INSERT INTO tb2 VALUES (2, X'AABBCC', X'DDCCBB');" \
        "INSERT INTO tb2 VALUES (3, X'FFEE00', X'112233');" \
        "INSERT INTO tb2 VALUES (10, X'123456', X'654321');" 

int create_test_db(const char* path) {
    sqlite3* db;
    char* err = NULL;

    if (sqlite3_open(path, &db) != SQLITE_OK) {
        return -1;
    }

    // 複合主キーのテーブル作成とデータ挿入
    const char* sql_tb1 = TB1_CREATE_SQL;
        
    if (sqlite3_exec(db, sql_tb1, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "tb1 error: %s\n", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return -2;
    }


    // 単一主キーのテーブル作成とデータ挿入
    const char* sql_tb2 = TB2_CREATE_SQL;
        
    if (sqlite3_exec(db, sql_tb2, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "tb2 error: %s\n", err);
        sqlite3_free(err);
        sqlite3_close(db);
        return -2;
    }

    
    return 0;
}

int main(void) {
    const char* db_path = "sample_test.db";
    if (create_test_db(db_path) != 0) {
        fprintf(stderr, "Failed to create test database\n");
        return 1;
    }
    printf("Test database created successfully: %s\n", db_path);

    printf("Tables:\n");
    // ;を改行に置換して表示
    char tb1_display[1024];
    strcpy(tb1_display, TB1_CREATE_SQL);
    for (int i = 0; tb1_display[i] != '\0'; i++) {
        if (tb1_display[i] == ';') {
            tb1_display[i] = '\n';
        }
    }
    printf("%s\n", tb1_display);

    char tb2_display[1024];
    strcpy(tb2_display, TB2_CREATE_SQL);
    for (int i = 0; tb2_display[i] != '\0'; i++) {
        if (tb2_display[i] == ';') {
            tb2_display[i] = '\n';
        }
    }
    printf("%s\n", tb2_display);

    return 0;
}