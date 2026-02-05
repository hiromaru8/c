/*
 * test_cli_run.c
 *
 * cli_runの単体テスト
 *  - テストケースは関数単位
 *  - main はテストランナーのみ
 */

#include "cli_run.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Utility for test:
/* テスト用 SQLite DB を作成 */
int create_test_db(const char* path) {
    sqlite3* db;
    char* err = NULL;

    if (sqlite3_open(path, &db) != SQLITE_OK) {
        return -1;
    }

    // 複合主キーのテーブル作成とデータ挿入
    const char* sql_tb1 =
        "CREATE TABLE tb1 ("
        "  id_src  INTEGER,"
        "  id_dest INTEGER,"
        "  data1 BLOB," 
        "  data2 BLOB,"
        "  PRIMARY KEY (id_src, id_dest)"
        ");"
        "INSERT INTO tb1 VALUES (1,1, X'010203', X'040506');"
        "INSERT INTO tb1 VALUES (2,2, X'AABBCC', X'DDCCBB');"
        "INSERT INTO tb1 VALUES (3,3, X'FFEE00', X'112233');"
        "INSERT INTO tb1 VALUES (1,10, X'123456', X'654321');"
        "INSERT INTO tb1 VALUES (2,20, X'789ABC', X'CBA987');"
        "INSERT INTO tb1 VALUES (3,30, X'000FFF', X'FFF000');"
        ;
    if (sqlite3_exec(db, sql_tb1, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return -2;
    }

    // 単一主キーのテーブル作成とデータ挿入
    const char* sql_tb2 =
        "CREATE TABLE tb2 ("
        "  id  INTEGER,"
        "  data1 BLOB," 
        "  data2 BLOB,"
        "  PRIMARY KEY (id)"
        ");"
        "INSERT INTO tb2 VALUES (1, X'010203', X'040506');"
        "INSERT INTO tb2 VALUES (2, X'AABBCC', X'DDCCBB');"
        "INSERT INTO tb2 VALUES (3, X'FFEE00', X'112233');"
        "INSERT INTO tb2 VALUES (10, X'123456', X'654321');"
        ;
    if (sqlite3_exec(db, sql_tb2, NULL, NULL, &err) != SQLITE_OK) {
        sqlite3_free(err);
        sqlite3_close(db);
        return -2;
    }

    sqlite3_close(db);
    
    return 0;
}

// ファイル読み込みユーティリティ
int load_file(
    const char* path,
    unsigned char** buf,
    int* size
) {
    FILE* fp = fopen(path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    *size = (int)ftell(fp);
    rewind(fp);

    *buf = (unsigned char*)malloc(*size);
    if (!*buf) {
        fclose(fp);
        return -2;
    }

    fread(*buf, 1, *size, fp);
    fclose(fp);
    return 0;
}


/* ===== 簡易 ASSERT ===== */
// 真偽 ASSERTマクロ
#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            printf("ASSERT FAILED: %s (%s:%d)\n", \
                   #cond, __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

// 真偽 ASSERTマクロ
#define ASSERT_EQ(exp, act) \
    do { \
        if ((exp) != (act)) { \
            printf("ASSERT_EQ FAILED: %s=%d, %s=%d (%s:%d)\n", \
                   #exp, (int)(exp), #act, (int)(act), \
                   __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)

// メモリ内容比較 ASSERTマクロ
#define ASSERT_MEMEQ(exp, act, size) \
    do { \
        if (memcmp((exp), (act), (size)) != 0) { \
            printf("ASSERT_MEMEQ FAILED (%s:%d)\n", __FILE__, __LINE__); \
            return 1; \
        } \
    } while (0)




/* ===== テストケース ===== */
static const char* TEST_DB = "test.db";
static const char* OUT_BINARY_FILE = "out.bin";

// ==========
// 正常系テスト
// ==========
// 複合主キー + PK値指定
int test_cli_composite_pk_values(void) {
    remove(OUT_BINARY_FILE);
    int rc = cli_run_export(
        TEST_DB,
        "tb1",
        "id_src,id_dest",
        "(1,1)",
        "data1,data2",
        OUT_BINARY_FILE
    );
    ASSERT(rc == 0);

    /* actual: out.bin */
    unsigned char* act;
    int act_size;
    ASSERT(load_file(OUT_BINARY_FILE, &act, &act_size) == 0);

    // expected 
    unsigned char exp[] = {0x01,0x02,0x03,0x04,0x05,0x06};
    int exp_size = sizeof(exp);

    ASSERT_EQ(exp_size, act_size);
    ASSERT_MEMEQ(exp, act, exp_size);

    free(act);
    return 0;
}

// 複合主キー + 全件
int test_cli_composite_pk_all(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        TEST_DB,
        "tb1",
        "id_src,id_dest",
        NULL,
        "*",
        OUT_BINARY_FILE
    );
    ASSERT(rc == 0);

    /* actual: out.bin */
    unsigned char* act;
    int act_size;
    ASSERT(load_file(OUT_BINARY_FILE, &act, &act_size) == 0);

    // expected 
    unsigned char exp[] = {
        0x31,0x31,      0x01,0x02,0x03,0x04,0x05,0x06,
        0x31,0x31,0x30, 0x12,0x34,0x56,0x65,0x43,0x21,
        0x32,0x32,      0xAA,0xBB,0xCC,0xDD,0xCC,0xBB,
        0x32,0x32,0x30, 0x78,0x9A,0xBC,0xCB,0xA9,0x87,
        0x33,0x33,      0xFF,0xEE,0x00,0x11,0x22,0x33,
        0x33,0x33,0x30, 0x00,0x0F,0xFF,0xFF,0xF0,0x0
    };
    int exp_size = sizeof(exp);

    ASSERT_EQ(exp_size, act_size);
    ASSERT_MEMEQ(exp, act, exp_size);

    free(act);
    return 0;
}

// 単一主キー + PK値指定
int test_cli_single_pk_values(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        TEST_DB,
        "tb2",
        "id",
        "1",
        "data1",
        OUT_BINARY_FILE
    );
    ASSERT(rc == 0);

    /* actual: out.bin */
    unsigned char* act;
    int act_size;
    ASSERT(load_file(OUT_BINARY_FILE, &act, &act_size) == 0);

    // expected 
    unsigned char exp[] = {0x01,0x02,0x03};
    int exp_size = sizeof(exp);

    ASSERT_EQ(exp_size, act_size);
    ASSERT_MEMEQ(exp, act, exp_size);

    free(act);
    return 0;
}

// 単一主キー + 全件
int test_cli_single_pk_all(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        TEST_DB,
        "tb2",
        "id",
        NULL,
        "data2",
        OUT_BINARY_FILE
    );
    ASSERT(rc == 0);

    /* actual: out.bin */
    unsigned char* act;
    int act_size;
    ASSERT(load_file(OUT_BINARY_FILE, &act, &act_size) == 0);

    // expected 
    unsigned char exp[] = {
        0x04,0x05,0x06,
        0xDD,0xCC,0xBB,
        0x11,0x22,0x33,
        0x65,0x43,0x21
    };
    int exp_size = sizeof(exp);

    ASSERT_EQ(exp_size, act_size);
    ASSERT_MEMEQ(exp, act, exp_size);

    free(act);
    return 0;
}

// ==========
// 準正常系テスト
// ==========

// no data case
int test_cli_no_data(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        TEST_DB,
        "tb1",
        "id_src,id_dest",
        "(999,999)",
        "*",
        OUT_BINARY_FILE
    );
    ASSERT_EQ(3, rc);  // No data retrieved

    return 0;
}

// query failed
// (table not exist)
int test_cli_query_failed(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        TEST_DB,
        "non_existing_table",
        "id",
        NULL,
        "*",
        OUT_BINARY_FILE
    );
    ASSERT_EQ(2, rc);  // Query failed

    return 0;
}

// DB Open失敗
int test_cli_db_open_failed(void) {
    remove(OUT_BINARY_FILE);

    int rc = cli_run_export(
        "Z:/this/path/does/not/exist/test.db",
        "tb1",
        "id_src,id_dest",
        NULL,
        "*",
        OUT_BINARY_FILE
    );
    ASSERT_EQ(1, rc);  // Query failed (DB open failed)

    return 0;
}

