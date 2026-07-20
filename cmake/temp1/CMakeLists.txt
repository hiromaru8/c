cmake_minimum_required(VERSION 3.16)

# ============================================================
# Project
# ============================================================
project(MyProject LANGUAGES C)      # C言語のプロジェクトとして定義

# ============================================================
# C Standard
# ============================================================
set(CMAKE_C_STANDARD 99)            # コンパイラが対応するC言語規格を指定
set(CMAKE_C_STANDARD_REQUIRED ON)   # 指定した規格を必ず使用するようにする
set(CMAKE_C_EXTENSIONS OFF)         # 拡張機能を無効化する（GNUなどの独自拡張を禁止し、標準Cのみを使うことで移植性を高める）

# ============================================================
# Build Type
# ============================================================
# ビルドタイプ（DebugやRelease）を指定していない場合は、Releaseをデフォルトにする
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ============================================================
# Compiler Warning Options
# ============================================================
if(MSVC)
    add_compile_options(
        /W4                 # 警告レベル4（最も詳細な警告）を有効化
        /utf-8              # ソースコードの文字エンコーディングをUTF-8に設定  
        /permissive-        # 標準C/C++に準拠していないコードに対してエラーを出す
    )
else()
    add_compile_options(
        -Wall               # すべての警告を有効化
        -Wextra             # 追加の警告を有効化
        -Wpedantic          # 標準Cに準拠していないコードに対して警告を出す
        -Wconversion        # 型変換に関する警告を有効化(int x = 1000;char c = x;値が欠落する可能性がある場合に警告を出す)
        -Wsign-conversion   # 符号付き/符号なしの型変換に関する警告を有効化(int a = -1;unsigned int b = a;符号付きから符号なしへの変換で警告を出す)
        -Wshadow            # 変数のスコープを超えて同じ名前の変数が定義されている場合に警告を出す
        -Wformat=2          # printfやscanfなどのフォーマット文字列に関する警告を有効化(フォーマット文字列と引数の型が一致しない場合に警告を出す)
        -Wundef             # 未定義のマクロを使用した場合に警告を出す(#defineされていないマクロを使用した場合に警告を出す)
        -Wswitch-enum       # enum型のswitch文で、すべての列挙子を網羅していない場合に警告を出す
)
endif()

# ============================================================
# Source Files
# ============================================================
file(GLOB_RECURSE SOURCES
    src/*.c
)

# ============================================================
# Executable
# ============================================================
add_executable(${PROJECT_NAME}
    ${SOURCES}
)

# ============================================================
# Include Directories
# ============================================================
# includeディレクトリを指定することで、ソースコード内で#include "header.h"のようにヘッダファイルを参照できるようになる
target_include_directories(${PROJECT_NAME}
    PRIVATE
        include
)

# ============================================================
# Libraries
# ============================================================
# target_link_libraries(${PROJECT_NAME}
#     PRIVATE
#         m
# )

# ============================================================
# Output Directory
# ============================================================
if(NOT CMAKE_CONFIGURATION_TYPES) # Multi-Config Generator (Visual Studio, Xcode) では CMAKE_CONFIGURATION_TYPES が定義されている
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
            ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    )
endif()
set_target_properties(${PROJECT_NAME}
  PROPERTIES OUTPUT_NAME "MyApp"  # 実行ファイルの出力名を指定
)


# ============================================================
# Definitions ビルド構成に応じてプリプロセッサマクロを定義する
# ============================================================
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:DEBUG> # Debugビルドの場合にDEBUGマクロを定義
)

# ============================================================
# Optimization Options ビルド構成に応じて最適化オプションを設定する
# ============================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:/O2>    # Releaseビルドの場合に最適化レベル2を有効化
        $<$<CONFIG:Debug>:/Od>      # Debugビルドの場合に最適化を無効化
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-O2>    # Releaseビルドの場合に最適化レベル2を有効化
        $<$<CONFIG:Debug>:-O0 -g>   # Debugビルドの場合に最適化を無効化し、デバッグ情報を生成
    )
endif()

# ============================================================
# Install
# ============================================================
# インストール先のディレクトリを指定する
#  example: cmake --install build --prefix C:/MyApp
#               C:/
#               └── MyApp/
#                   └── bin/
#                       └── MyApp.exe
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# ============================================================
# Testing
# ============================================================
# CTestを有効化することで、ctestコマンドでテストを実行できるようになる
include(CTest)

if(BUILD_TESTING)
    add_executable(test_sample
        test/hello.c
    )

    add_test(NAME SampleTest COMMAND test_sample)
endif()
set_target_properties(test_sample PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# ============================================================
# Project Configuration Message ビルド時にプロジェクトの設定情報を表示する
# ============================================================
message(STATUS "========================================")
message(STATUS "Project Configuration")
message(STATUS "========================================")
message(STATUS "Project Name      : ${PROJECT_NAME}")
message(STATUS "Project Version   : ${PROJECT_VERSION}")
message(STATUS "Source Directory  : ${CMAKE_SOURCE_DIR}")
message(STATUS "Build Directory   : ${CMAKE_BINARY_DIR}")
message(STATUS "Generator         : ${CMAKE_GENERATOR}")
message(STATUS "Build Type        : ${CMAKE_BUILD_TYPE}")
message(STATUS "C Standard        : C${CMAKE_C_STANDARD}")
message(STATUS "Compiler          : ${CMAKE_C_COMPILER_ID}")
message(STATUS "Compiler Version  : ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "Compiler Path     : ${CMAKE_C_COMPILER}")
message(STATUS "System            : ${CMAKE_SYSTEM_NAME}")
message(STATUS "Architecture      : ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Install Prefix    : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Testing           : ${BUILD_TESTING}")
message(STATUS "========================================")
