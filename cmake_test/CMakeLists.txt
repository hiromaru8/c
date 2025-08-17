# ----------------------------
# 基本設定
# ----------------------------
cmake_minimum_required(VERSION 3.20)
# プロジェクト名を HelloPJ に設定。言語は C に限定
project(HelloPJ C)
# C の規格を C11 に設定。
set(CMAKE_C_STANDARD 11)


# 実行ファイル・DLLの出力先を build ディレクトリ（カレントディレクトリ）直下に設定
# CMAKE_BINARY_DIRはこのプロジェクトのビルドディレクトリの絶対パス
# DLL / EXE / LIB を build/ 直下にまとめる
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# 静的ライブラリ(.lib/.a)の出力先
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# 動的ライブラリ(.dll/.so)の出力先
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)


# サブディレクトリを追加
add_subdirectory(hello)
add_subdirectory(libhello)
