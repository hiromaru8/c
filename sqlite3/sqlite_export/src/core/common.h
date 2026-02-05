/*
 * common.h
 *
 * CUI(EXE) と DLL の両方で共通に使用する処理の宣言
 */

#pragma once

typedef struct sqlite3 sqlite3;  // ← 前方宣言だけ

/*
 * SELECT 文を組み立てる関数
 *
 * table    : テーブル名
 * pk_list  : ORDER BY に使う PK
 * col_list : SELECT するカラム
 * out_sql  : 生成された SQL
 */
int build_select_sql(
    const char* table,
    const char* pk_list,      // "id_src,id_dest"
    const char* pk_values,    // "(1,10),(2,20)"
    const char* col_list,
    char* out_sql,
    int maxlen
);

/*
 * UPDATE 文を組み立てる関数
 *
 * table      : テーブル名
 * set_clause : SET に使う句
 * pk_list    : WHERE に使う PK
 * pk_values  : WHERE に使う PK値リスト
 * out_sql    : 生成された SQL
 */
int build_update_sql(
    const char* table,
    const char* set_clause,   /* "data1=X'0102', data2=X'0304'" */
    const char* pk_list,      /* "id_src,id_dest" */
    const char* pk_values,    /* "(1,10),(2,20)" */
    char* out_sql,
    int maxlen
);


/*
 * 読み取り専用モードで SQLite DB をオープンする
 */
int open_database_readonly(
    const char* db_path,
    sqlite3** out_db
);

/*
 * 読み書きモードで SQLite DB をオープンする
 */
int open_database_readwrite(
    const char* db_path,
    sqlite3** out_db
);

/*
 * SQLite DB をクローズする
 */
int close_database(sqlite3* db);


/*
 * SQL を実行し、結果を 1 つのバッファにまとめる
 *
 * SQLite の BLOB をそのまま連結して返す
 */
int exec_query_to_buffer(
    sqlite3* db,
    const char* sql,
    unsigned char** out_buf,
    int* out_size
);


/*
 * UPDATE 文を実行する（結果行なし）
 */
int exec_update(
    sqlite3* db,
    const char* sql,
    int* affected_rows
);
