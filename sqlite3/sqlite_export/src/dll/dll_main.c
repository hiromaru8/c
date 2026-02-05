/*
 * dll_main.c
 *
 * Python など外部言語から呼ばれる DLL 実装
 */

#define SQLITE_EXPORT_DLL
#include <stddef.h>
#include <sqlite3.h>
#include "sqlite_export.h"
#include "common.h"

/*
 * DLL から呼び出される関数
 *
 * CUI の main() と違い、
 * ・ファイル出力はしない
 * ・メモリ上で結果を返す
 */
SE_API int se_query(
    const char* db_path,
    const char* table,
    const char* pk_list,
    const char* pk_values,
    const char* col_list,
    unsigned char** out_buf,
    int* out_size
) {
    char sql[1024];
    build_select_sql(table, pk_list, pk_values, col_list, sql, sizeof(sql));

    int rc;
    sqlite3* db = NULL;
    rc = open_database_readonly(db_path, &db);
    if (rc != 0) {
        return rc;
    }

    rc = exec_query_to_buffer(db, sql, out_buf, out_size);
    close_database(db);
    return rc;
}

/*  
 * DLL から呼び出される関数
 *
 * CUI の update コマンドと同等の処理を行う
 */
SE_API int se_update(
    const char* db_path,
    const char* table,
    const char* pk_list,
    const char* pk_values,
    const char* set_clause,
    int* affected_rows
) {
    char sql[1024];

    if (!db_path || !table || !pk_list || !pk_values || !set_clause) {
        return -10;  // 引数不正
    }

    build_update_sql(
        table,
        set_clause,
        pk_list,
        pk_values,
        sql,
        sizeof(sql)
    );

    /* デバッグ用（必要なら） */
    /* printf("SQL: %s\n", sql); */

    int rc;
    sqlite3* db = NULL;
    rc = open_database_readwrite(db_path, &db);
    if (rc != 0) {
        return rc;
    }
    
    rc = exec_update(db, sql, affected_rows);
    close_database(db);
    return rc;
}
