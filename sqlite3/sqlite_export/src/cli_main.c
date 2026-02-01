/*
 * cli_main.c
 *
 * コマンドラインから SQLite DB を読み出し、
 * バイナリファイルに出力する EXE
 */

#include "common.h"
#include "cli_run.h"
#include <stdio.h>
#include <stdlib.h>
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

    return cli_run(db, table, pk, pk_values, cols, out);

}
