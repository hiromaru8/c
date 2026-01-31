/*
 * dll_main.c
 *
 * Python など外部言語から呼ばれる DLL 実装
 */

#define SQLITE_EXPORT_DLL
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
    const char* col_list,
    unsigned char** out_buf,
    int* out_size
) {
    char sql[512];
    build_sql(table, pk_list, col_list, sql, sizeof(sql));

    return exec_query_to_buffer(db_path, sql, out_buf, out_size);
}
