# ----------------------------
# 基本設定
# ----------------------------
cmake_minimum_required(VERSION 3.20)
# プロジェクト名を AES_Project に設定。言語は C に限定
project(AES_Project C)
# C の規格を C11 に設定。
set(CMAKE_C_STANDARD 11)

# DLL / EXE / LIB を build/ 直下にまとめる
# 実行ファイル・DLLの出力先を build ディレクトリ直下に設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# 静的ライブラリ(.lib/.a)の出力先
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# 動的ライブラリ(.dll/.so)の出力先
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})



message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
# ----------------------------
# tiny_aes_dll
# ----------------------------
add_library(tiny_aes SHARED             # tiny_aes という名前の DLL を作成
    tiny_aes_dll/src/aes.c
    tiny_aes_dll/src/aes_256_ctr.c
)
target_include_directories(tiny_aes PUBLIC tiny_aes_dll/include)
set_target_properties(tiny_aes PROPERTIES OUTPUT_NAME "tiny_aes")
target_compile_options(tiny_aes PRIVATE -O3 -Wall -Itiny_aes_dll/include)

# ----------------------------
# ttable_aes_dll
# ----------------------------
add_library(ttable_aes SHARED
    ttable_aes_dll/src/aes_256_ctr.c
)
target_include_directories(ttable_aes PUBLIC ttable_aes_dll/include)
set_target_properties(ttable_aes PROPERTIES OUTPUT_NAME "ttable_aes_t")
target_compile_options(ttable_aes PRIVATE -O3 -Wall -Ittable_aes_dll/include)

# ----------------------------
# aes_ni/test.exe
# ----------------------------
add_executable(aes_ni_test aes_ni/test.c)
target_include_directories(aes_ni_test PRIVATE tiny_aes_dll/include)
target_compile_options(aes_ni_test PRIVATE -maes -msse2)
set_target_properties(aes_ni_test PROPERTIES OUTPUT_NAME "aes_ni_test")

# ----------------------------
# benchmark/test.exe
# ----------------------------
add_executable(benchmark_test benchmark/test.c)
target_include_directories(benchmark_test PRIVATE tiny_aes_dll/include ttable_aes_dll/include)
set_target_properties(benchmark_test PROPERTIES OUTPUT_NAME "benchmark_test")