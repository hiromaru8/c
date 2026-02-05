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
int build_select_sql(
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
 * UPDATE 文を生成する
 *
 * 例:
 *   UPDATE tb1 SET data1=X'0102', data2=X'0304' WHERE (id_src,id_dest) IN ((1,10),(2,20));
 */
int build_update_sql(
    const char* table,
    const char* set_clause,   /* "data1=X'0102', data2=X'0304'" */
    const char* pk_list,      /* "id_src,id_dest" */
    const char* pk_values,    /* "(1,10),(2,20)" */
    char* out_sql,
    int maxlen
) {
    snprintf(out_sql, maxlen,
        "UPDATE %s SET %s "
        "WHERE (%s) IN (%s);",
        table,
        set_clause,
        pk_list,     /* id_src,id_dest */
        pk_values  /* (1,10),(2,20) */
    );
    return 0;
}


/*
 * 読み取り専用モードで SQLite DB をオープンする
 */
int open_database_readonly(
    const char* db_path,
    sqlite3** out_db
) {
    sqlite3* db = NULL;
    int rc = sqlite3_open_v2(db_path, &db,
        SQLITE_OPEN_READONLY | SQLITE_OPEN_URI,
        NULL);
    if (rc != SQLITE_OK) {
        return rc;
    }
    *out_db = db;
    return SQLITE_OK;
}


/*
 * 読み書きモードで SQLite DB をオープンする
 */
int open_database_readwrite(
    const char* db_path,
    sqlite3** out_db
) {
    sqlite3* db = NULL;
    int rc = sqlite3_open_v2(db_path, &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_URI,
        NULL);
    if (rc != SQLITE_OK) {
        return rc;
    }
    *out_db = db;
    return SQLITE_OK;
}

/*
 * SQLite DB をクローズする
 */
int close_database(sqlite3* db) {
    return sqlite3_close(db);
}



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
    sqlite3* db,                  /* SQLite DB ハンドル */
    const char* sql,              /* 実行する SELECT 文 */
    unsigned char** out_buf,      /* 生成したバッファの返却先 */
    int* out_size                 /* バッファサイズ（byte） */
) {
    sqlite3_stmt* stmt = NULL;
    int rc;

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


/*
 * UPDATE 文を実行する（結果行なし）
 *
 * 戻り値:
 *   0  : 成功（1件以上更新）
 *  -1  : DB open 失敗
 *  -2  : SQL prepare 失敗
 *  -3  : 実行失敗
 *  -4  : 更新件数 0
 */
int exec_update(
    sqlite3* db,
    const char* sql,
    int* affected_rows
) {
    sqlite3_stmt* stmt = NULL;
    int rc;

    /* SQL プリペア */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -2;
    }

    /* SQL 実行 */
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -3;
    }

    /* 更新件数取得 */
    int changed = sqlite3_changes(db);
    if (affected_rows) {
        *affected_rows = changed;
    }

    /* 後処理 */
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    //  更新件数 0 の場合はエラーとする
    if (changed == 0) {
        return -4;  // 該当レコードなし
    }

    return 0;
}


