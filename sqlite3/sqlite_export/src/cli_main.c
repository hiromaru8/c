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

typedef enum {
    MODE_NONE,
    MODE_EXPORT,
    MODE_UPDATE
} cli_mode_t;


int main(int argc, char* argv[]) {
    cli_mode_t mode = MODE_NONE;
    const char *db        = NULL;
    const char *table     = NULL;
    const char *pk        = NULL;
    const char *pk_values = NULL;
    const char *cols      = "*";
    const char *out       = NULL;
    const char *set       = NULL;

    /*
     * 引数解析
     */
    if (argc < 2) {
        printf("usage: export | update ...\n");
        return 1;
    }

    /* サブコマンド判定 */
    if (!strcmp(argv[1], "export")) {
        mode = MODE_EXPORT;
    } else if (!strcmp(argv[1], "update")) {
        mode = MODE_UPDATE;
    } else {
        printf("Unknown command: %s\n", argv[1]);
        return 1;
    }

    /* 共通オプション解析 */
    for (int i = 2; i < argc; i++) {
        if      (!strcmp(argv[i], "--db"))        db = argv[++i];
        else if (!strcmp(argv[i], "--table"))     table = argv[++i];
        else if (!strcmp(argv[i], "--pk"))        pk = argv[++i];
        else if (!strcmp(argv[i], "--pk-values")) pk_values = argv[++i];
        else if (!strcmp(argv[i], "--columns"))   cols = argv[++i];
        else if (!strcmp(argv[i], "--out"))       out = argv[++i];
        else if (!strcmp(argv[i], "--set"))       set = argv[++i];
    }
    /* モード別必須チェック & 実行 */
    if (mode == MODE_EXPORT) {
        if (!db || !table || !pk || !out) {
            printf("export: missing arguments\n");
            return 1;
        }
        return cli_run_export(db, table, pk, pk_values, cols, out);
    }

    if (mode == MODE_UPDATE) {
        if (!db || !table || !pk || !pk_values || !set) {
            printf("update: missing arguments\n");
            return 1;
        }
        return cli_run_update(db, table, pk, pk_values, set);
    }

    return 0;
}
