/*
 * common.h
 *
 * CUI(EXE) と DLL の両方で共通に使用する処理の宣言
 */

#pragma once
#include <sqlite3.h>

/*
 * SELECT 文を組み立てる関数
 *
 * table    : テーブル名
 * pk_list  : ORDER BY に使う PK
 * col_list : SELECT するカラム
 * out_sql  : 生成された SQL
 */
int build_sql(
    const char* table,
    const char* pk_list,      // "id_src,id_dest"
    const char* pk_values,    // "(1,10),(2,20)"
    const char* col_list,
    char* out_sql,
    int maxlen
);


/*
 * SQL を実行し、結果を 1 つのバッファにまとめる
 *
 * SQLite の BLOB をそのまま連結して返す
 */
int exec_query_to_buffer(
    const char* db_path,
    const char* sql,
    unsigned char** out_buf,
    int* out_size
);
