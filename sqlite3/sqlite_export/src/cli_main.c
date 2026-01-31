/*
 * cli_main.c
 *
 * コマンドラインから SQLite DB を読み出し、
 * バイナリファイルに出力する EXE
 */

#include "common.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    const char *db          = NULL;
    const char *table       = NULL;
    const char *pk          = NULL;
    const char *pk_values   = NULL;
    const char *cols        = "*";
    const char *out         = NULL;

    /*
     * コマンドライン引数の解析
     */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--db")) db = argv[++i];
        else if (!strcmp(argv[i], "--table")) table = argv[++i];
        else if (!strcmp(argv[i], "--pk")) pk = argv[++i];
        else if (!strcmp(argv[i], "--pk-values")) pk_values = argv[++i];
        else if (!strcmp(argv[i], "--columns")) cols = argv[++i];
        else if (!strcmp(argv[i], "--out")) out = argv[++i];
    }
    /* 必須引数チェック */
    if (!db || !table || !pk || !out) {
        printf("Invalid arguments\n");
        return 1;
    }

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

    /* バイナリファイルへ書き込み */
    FILE* fp = fopen(out, "wb");
    fwrite(buf, 1, size, fp);
    fclose(fp);

    free(buf);

    printf("Exported %d bytes\n", size);
    return 0;
}
