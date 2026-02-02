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

#define REQUIRE_VALUE(opt) \
    do { \
        if (i + 1 >= argc) { \
            printf("Option %s requires a value\n\n", opt); \
            goto arg_error; \
        } \
    } while (0)

static void print_usage_common(void) {
    printf("Usage:\n");
    printf("  sqlite_export_cli <command> [options]\n\n");
    printf("Commands:\n");
    printf("  export    Export rows to binary file\n");
    printf("  update    Update rows in table\n\n");
}
static void print_usage_export(void) {
    printf("Usage:\n");
    printf("  sqlite_export_cli export \\\n");
    printf("    --db <dbfile> \\\n");
    printf("    --table <table> \\\n");
    printf("    --pk <col[,col...]> \\\n");
    printf("    [--pk-values \"(v1,v2)\"] \\\n");
    printf("    [--columns <col[,col...]>] \\\n");
    printf("    --out <outfile>\n\n");

    printf("Example:\n");
    printf("  sqlite_export_cli export \\\n");
    printf("    --db sample.db \\\n");
    printf("    --table tb1 \\\n");
    printf("    --pk id_src,id_dest \\\n");
    printf("    --out out.bin\n");
}
static void print_usage_update(void) {
    printf("Usage:\n");
    printf("  sqlite_export_cli update \\\n");
    printf("    --db <dbfile> \\\n");
    printf("    --table <table> \\\n");
    printf("    --pk <col[,col...]> \\\n");
    printf("    --pk-values \"(v1,v2)[,(v3,v4)...]\" \\\n");
    printf("    --set \"col1=val1, col2=val2\"\n\n");

    printf("Example (PowerShell):\n");
    printf("  sqlite_export_cli update \\\n");
    printf("    --db sample.db \\\n");
    printf("    --table tb1 \\\n");
    printf("    --pk id_src,id_dest \\\n");
    printf("    --pk-values \"(1,1)\" \\\n");
    printf("    --set \"data1=X'0102', data2=X'0304'\"\n");
}




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
        goto arg_error;
    }
    /* --help / -h（どこでもOK） */
    if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
        goto arg_error;
    }
    
    /* サブコマンド判定 */
    if (!strcmp(argv[1], "export")) {
        mode = MODE_EXPORT;
    } else if (!strcmp(argv[1], "update")) {
        mode = MODE_UPDATE;
    } else {
        printf("Unknown command: %s\n", argv[1]);
        goto arg_error;
    }

    /* 共通オプション解析 */
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--db")) {
            REQUIRE_VALUE("--db");
            db = argv[++i];
        }
        else if (!strcmp(argv[i], "--table")) {
            REQUIRE_VALUE("--table");
            table = argv[++i];
        }
        else if (!strcmp(argv[i], "--pk")) {
            REQUIRE_VALUE("--pk");
            pk = argv[++i];
        }
        else if (!strcmp(argv[i], "--pk-values")) {
            REQUIRE_VALUE("--pk-values");
            pk_values = argv[++i];
        }
        else if (!strcmp(argv[i], "--columns")) {
            if (mode != MODE_EXPORT) {
                printf("--columns is only valid for export\n\n");
                goto arg_error;
            }
            REQUIRE_VALUE("--columns");
            cols = argv[++i];
        }
        else if (!strcmp(argv[i], "--out")) {
            REQUIRE_VALUE("--out");
            out = argv[++i];
        }
        else if (!strcmp(argv[i], "--set")) {
            REQUIRE_VALUE("--set");
            set = argv[++i];
        }else if (strcmp(argv[i], "--help") || strcmp(argv[i], "-h")) {
            goto arg_error;
        }
        else {
            printf("Unknown option: %s\n\n", argv[i]);
            goto arg_error;
        }
    }

    /* モード別必須チェック & 実行 */
    if (mode == MODE_EXPORT) {
        if (!db || !table || !pk || !out) {
            printf("export: missing arguments\n");
            print_usage_export();
            return 1;
        }
        return cli_run_export(db, table, pk, pk_values, cols, out);
    }

    if (mode == MODE_UPDATE) {
        if (!db || !table || !pk || !pk_values || !set) {
            printf("update: missing arguments\n");
            print_usage_update();
            return 1;
        }
        return cli_run_update(db, table, pk, pk_values, set);
    }

    arg_error:
        if (mode == MODE_EXPORT) {
            print_usage_export();
        } else if (mode == MODE_UPDATE) {
            print_usage_update();
        } else {
            print_usage_common();
        }
        return 1;    
}
