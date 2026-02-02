/*
 * common.c
 *
 * SQLite に関する共通処理の実装
 */

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

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

void* (*malloc_hook)(size_t) = malloc;


/*
 * SQL クエリを実行し、結果セットを 1 つのバイナリバッファに連結して返す
 *
 * - 各行・各カラムを sqlite3_column_blob() で取得する
 * - INTEGER / TEXT / BLOB すべて「バイト列」として扱われる
 * - 行区切り・列区切りの情報は保持しない（純粋な生バイト連結）
 *
 * 戻り値:
 *   0  : 成功
 *  <0  : エラー
 */
int exec_query_to_buffer(
    const char* db_path,          /* SQLite DB ファイルパス */
    const char* sql,              /* 実行する SELECT 文 */
    unsigned char** out_buf,      /* 生成したバッファの返却先 */
    int* out_size                 /* バッファサイズ（byte） */
) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;

    /* =========================================================
     * DB をオープン
     * ========================================================= */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        /* db は内部で確保されている可能性があるが、
           sqlite3_open 失敗時は close 不要 */
        return -1;
    }

    /* =========================================================
     * SQL をプリペア（コンパイル）
     * ========================================================= */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -2;
    }

    /* =========================================================
     * 動的に拡張される出力バッファ
     * =========================================================
     * cap  : 現在の確保容量
     * size : 実際に使用しているサイズ
     */
    int cap = 1024;
    int size = 0;
    unsigned char* buf = malloc(cap);
    if (buf == NULL) { 
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -3;
    }

    /* =========================================================
     * クエリ結果を 1 行ずつ処理
     * ========================================================= */
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        /* この行のカラム数 */
        int cols = sqlite3_column_count(stmt);

        for (int i = 0; i < cols; i++) {

            /*
             * カラムのデータを BLOB として取得
             *
             * 注意:
             *  - INTEGER 型でも sqlite3_column_blob() を使うと
             *    「内部表現のバイト列」が返る
             *  - 例: INTEGER 1 → '1' → 0x31（ASCII）
             *  - 数値のバイナリ表現（0x01）にはならない
             */
            const void* blob = sqlite3_column_blob(stmt, i);
            int len = sqlite3_column_bytes(stmt, i);

            /*
             * NULL カラムの場合:
             *   blob == NULL
             *   len  == 0
             * memcpy しても問題はないが、
             * NULL を区別したい場合はここで判定が必要
             */

            /* =================================================
             * バッファ容量が足りなければ拡張
             * ================================================= */
            if (size + len > cap) {
                /* 必要になるまで 2 倍拡張 */
                while (size + len > cap) {
                    cap *= 2;
                }
                unsigned char* new_buf = realloc(buf, cap);
                if (!new_buf) {
                    free(buf);
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    return -4;
                }
                buf = new_buf;
            }

            /* =================================================
             * カラムのバイト列を連結
             * ================================================= */
            if (len > 0) {
                memcpy(buf + size, blob, len);
                size += len;
            }
        }
    }

    /*
     * SQLITE_DONE 以外で抜けた場合はエラー
     */
    if (rc != SQLITE_DONE) {
        free(buf);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -5;
    }

    /* =========================================================
     * 後処理
     * ========================================================= */
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    /* 呼び出し元へ返却 */
    *out_buf = buf;
    *out_size = size;

    return 0;
}
