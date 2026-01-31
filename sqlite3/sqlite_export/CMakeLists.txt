# プロジェクト設定
cmake_minimum_required(VERSION 3.16)
project(sqlite_export C)

# C99 を使用
set(CMAKE_C_STANDARD 99)

# SQLite3 を探す
find_package(SQLite3 REQUIRED)

# ヘッダ検索パス
include_directories(include)

# DLL ビルド
add_library(sqlite_export SHARED
    src/common.c
    src/dll_main.c
)
target_link_libraries(sqlite_export SQLite::SQLite3)

# CUI EXE ビルド
add_executable(sqlite_export_cli
    src/common.c
    src/cli_main.c
)
target_link_libraries(sqlite_export_cli SQLite::SQLite3)

# ===== Unit Test =====
add_executable(test_build_sql
    src/common.c
    tests/test_build_sql.c
    tests/test_main.c
)

# カバレッジ用フラグ（GCC）
target_compile_options(test_build_sql PRIVATE
    -O0
    -g
    --coverage
)

target_link_options(test_build_sql PRIVATE
    --coverage
)
