




/*
 * cli_run.c
 *
 * コマンドライン EXE の処理本体
 */
#include "cli_run.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cli_run(
    const char* db,
    const char* table,
    const char* pk,
    const char* pk_values,
    const char* cols,
    const char* out
) {

    char sql[1024];
    build_sql(table, pk, pk_values, cols, sql, sizeof(sql));

    printf("SQL: %s\n", sql);

    unsigned char* buf;
    int size;

    /* DB からデータ取得 */
    if (exec_query_to_buffer(db, sql, &buf, &size) != 0) {
        printf("Query failed\n");
        return 2;
    }
    if(buf == NULL || size <= 0) {
        printf("No data retrieved\n");
        return 3;
    }

    /* バイナリファイルへ書き込み */
    FILE* fp = fopen(out, "wb");
    fwrite(buf, 1, size, fp);
    fclose(fp);

    free(buf);

    printf("Exported %d bytes\n", size);
    return 0;

}