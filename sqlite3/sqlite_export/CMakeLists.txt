cmake_minimum_required(VERSION 3.16)
project(sqlite_export C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================
# SQLite3
# ============================================================
add_library(sqlite3 STATIC
    external/sqlite/sqlite3.c
)
target_include_directories(sqlite3
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite
)

# ============================================================
# Core
# ============================================================
add_library(sqlite_export_core STATIC
    src/common.c
    src/cli_run.c
)
target_link_libraries(sqlite_export_core
    PUBLIC sqlite3
)
target_include_directories(sqlite_export_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# =========================
# Core（テスト用・coverage）
# =========================
add_library(sqlite_export_core_cov STATIC
    src/common.c
    src/cli_run.c
)
target_link_libraries(sqlite_export_core_cov PUBLIC  sqlite3)
target_include_directories(sqlite_export_core_cov 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_options(sqlite_export_core_cov PRIVATE
    -O0 -g --coverage
)
target_link_options(sqlite_export_core_cov PRIVATE
    --coverage
)

# ============================================================
# DLL
# ============================================================
add_library(sqlite_export SHARED
    src/dll_main.c
)
target_link_libraries(sqlite_export
    PRIVATE sqlite_export_core
)

# ============================================================
# CLI
# ============================================================
add_executable(sqlite_export_cli
    src/cli_main.c
)
target_link_libraries(sqlite_export_cli
    PRIVATE sqlite_export_core
)

# ============================================================
# sample for test
# ============================================================
add_executable(create_sample_db
    src/create_sample_db.c
)
target_link_libraries(create_sample_db PRIVATE sqlite3)

# ============================================================
# Unit Test
# ============================================================
add_executable(test_build
    tests/test_build_sql.c
    tests/test_cli_run.c
    tests/test_main.c
)

target_link_libraries(test_build
    PRIVATE sqlite_export_core_cov
)
# Coverage（テストのみ）
target_compile_options(test_build PRIVATE
    -O0
    -g
    --coverage
)
target_link_options(test_build PRIVATE
    --coverage
)
