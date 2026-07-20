cmake_minimum_required(VERSION 3.16)

# ============================================================
# プロジェクト基本設定
# ============================================================
# C言語のプロジェクトとして定義
project(sqlite_export C)

# 使用するC言語規格（C99）
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================
# コンパイラ共通設定
# ============================================================
# MSVCの場合：
#   ・ランタイムライブラリを静的リンク (/MT, /MTd)
#   ・配布時に Visual C++ 再頒布パッケージを不要にする
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(MSVC_RUNTIME_LIBRARY  "MultiThreadedDLL")
endif()
if (MSVC)
    add_compile_options(
        /utf-8
        /W4
    )
endif()
# ============================================================
# 外部ライブラリ：SQLite3
# ============================================================
# SQLite は DLL にせず、ソースコードを直接静的ライブラリとして組み込む
# → 配布時に sqlite3.dll が不要になる
add_library(sqlite3 STATIC
    external/sqlite/sqlite3.c
)

# SQLite のヘッダファイルの場所
target_include_directories(sqlite3
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite
)

# ============================================================
# 共通コアライブラリ（製品用）
# ============================================================
# DLL / CLI から共通で利用する処理をまとめたライブラリ
add_library(sqlite_export_core STATIC
    src/core/common.c
    src/cli/cli_run.c
)

# SQLite3 を利用
target_link_libraries(sqlite_export_core
    PUBLIC sqlite3
)

# 公開ヘッダの場所
target_include_directories(sqlite_export_core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

# MSVCでは C11 を明示（_Static_assert 等を確実に使うため）
if (MSVC)
    target_compile_options(sqlite_export_core PRIVATE /std:c11)
endif()

# ============================================================
# 共通コアライブラリ（テスト用・coverage有効）
# ============================================================
# 製品用と同じソースだが、テスト時のみカバレッジを取得するため別ターゲットにする
add_library(sqlite_export_core_cov STATIC
    src/core/common.c
)

target_link_libraries(sqlite_export_core_cov
    PUBLIC sqlite3
)

target_include_directories(sqlite_export_core_cov
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/cli
)

# GCC / MinGW の場合のみカバレッジを有効化
# （MSVC では --coverage が使えないため）
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(sqlite_export_core_cov PRIVATE
        -O0        # 最適化OFF（正確なカバレッジ取得のため）
        -g         # デバッグ情報付与
        --coverage # カバレッジ計測
    )
    target_link_options(sqlite_export_core_cov PRIVATE
        --coverage
    )
endif()

# ============================================================
# DLL（外部から利用されるライブラリ）
# ============================================================
add_library(sqlite_export SHARED
    src/dll/dll_main.c
)

target_link_libraries(sqlite_export
    PRIVATE sqlite_export_core
)

# Windows DLL の全シンボルを自動エクスポート
# → __declspec(dllexport) や .def ファイルを不要にする
set_target_properties(sqlite_export PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# MSVC：C11を有効化
if (MSVC)
    target_compile_options(sqlite_export PRIVATE /std:c11)
endif()

# MinGW：
#   ・libgcc を静的リンク
#   ・Windows標準DLL（msvcrt.dll 等）のみ依存させる
if (MINGW)
    target_link_options(sqlite_export PRIVATE
        -static-libgcc
    )
endif()

# ============================================================
# CLI（コマンドラインツール）
# ============================================================
add_executable(sqlite_export_cli
    src/cli/cli_run.c
    src/cli/cli_main.c
)

target_link_libraries(sqlite_export_cli
    PRIVATE sqlite_export_core
)

# MSVC：C11を有効化
if (MSVC)
    target_compile_options(sqlite_export_cli PRIVATE /std:c11)
endif()

# MinGW：
#   ・完全静的リンク
#   ・ユーザーPCに MinGW ランタイム不要
if (MINGW)
    target_link_options(sqlite_export_cli PRIVATE
        -static
        -static-libgcc
    )
endif()

# ============================================================
# サンプルツール（テスト用DB生成）
# ============================================================
add_executable(create_sample_db
    tests/fixtures/create_sample_db.c
)

target_link_libraries(create_sample_db
    PRIVATE sqlite3
)

if (MSVC)
    target_compile_options(create_sample_db PRIVATE /std:c11)
endif()

if (MINGW)
    target_link_options(create_sample_db PRIVATE
        -static
        -static-libgcc
    )
endif()

# ============================================================
# 単体テスト
# ============================================================
add_executable(test_build
    tests/test_build_sql.c
    tests/test_cli_run.c
    tests/test_main.c
    src/cli/cli_run.c
    src/core/common.c
    )

target_link_libraries(test_build
    PRIVATE sqlite_export_core_cov
)

# テスト時のみカバレッジを有効化（GCC / MinGWのみ）
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_compile_options(test_build PRIVATE
        -O0
        -g
        --coverage
    )
    target_link_options(test_build PRIVATE
        --coverage
    )
endif()
