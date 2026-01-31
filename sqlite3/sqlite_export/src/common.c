/*
 * common.c
 *
 * SQLite に関する共通処理の実装
 */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * SQL 文を生成する
 *
 * 例:
 *   SELECT data1,data2 FROM tb1 ORDER BY id_src,id_dest ASC;
 */
int build_sql(
    const char* table,
    const char* pk_list,
    const char* pk_values,
    const char* col_list,
    char* out_sql,
    int maxlen
) {
    const char* cols = strcmp(col_list, "*") == 0 ? "*" : col_list;

    /* PK値が指定されている場合 */
    if (pk_values && pk_values[0]) {
        snprintf(out_sql, maxlen,
            "SELECT %s FROM %s "
            "WHERE (%s) IN (%s) "
            "ORDER BY %s ASC;",
            cols,
            table,
            pk_list,     /* id_src,id_dest */
            pk_values,  /* (1,10),(2,20) */
            pk_list
        );
    }
    /* PK指定なし（全件） */
    else {
        snprintf(out_sql, maxlen,
            "SELECT %s FROM %s "
            "ORDER BY %s ASC;",
            cols,
            table,
            pk_list
        );
    }

    return 0;
}


/*
 * SQL を実行し、取得した BLOB をバッファに格納
 */
int exec_query_to_buffer(
    const char* db_path,
    const char* sql,
    unsigned char** out_buf,
    int* out_size
) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    /* DB をオープン */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) return -1;

    /* SQL をコンパイル */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) return -2;

    /*
     * 動的にサイズが増えるバッファ
     */
    int cap = 1024;
    int size = 0;
    unsigned char* buf = malloc(cap);

    /*
     * 1 レコードずつ処理
     */
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int cols = sqlite3_column_count(stmt);

        for (int i = 0; i < cols; i++) {
            const void* blob = sqlite3_column_blob(stmt, i);
            int len = sqlite3_column_bytes(stmt, i);

            /* バッファ不足なら拡張 */
            if (size + len > cap) {
                cap *= 2;
                buf = realloc(buf, cap);
            }

            /* BLOB を連結 */
            memcpy(buf + size, blob, len);
            size += len;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    *out_buf = buf;
    *out_size = size;
    return 0;
}
