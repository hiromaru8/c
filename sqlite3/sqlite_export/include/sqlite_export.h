/*
 * sqlite_export.h
 *
 * SQLite データベースからデータを取得するための
 * DLL 公開用ヘッダファイル。
 *
 * ・CUI(EXE) と DLL の両方で使用される
 * ・DLL から外部（Python 等）に公開する API を定義
 */

#ifndef SQLITE_EXPORT_H
#define SQLITE_EXPORT_H

/*
 * Windows 用 DLL のエクスポート指定
 *
 * DLL を作る側:
 *   SQLITE_EXPORT_DLL を定義 → dllexport
 *
 * DLL を使う側:
 *   定義なし → dllimport
 */
#ifdef _WIN32
  #ifdef SQLITE_EXPORT_DLL
    #define SE_API __declspec(dllexport)
  #else
    #define SE_API __declspec(dllimport)
  #endif
#else
  #define SE_API
#endif

/*
 * SQLite からデータを取得する関数
 *
 * 引数:
 *  db_path  : SQLite DB ファイルパス
 *  table    : テーブル名
 *  pk_list  : 主キー名（例 "id_src,id_dest" or "*"）
 *  col_list : 取得するカラム（例 "data1,data2" or "*"）
 *
 * 出力:
 *  out_buf  : 取得したバイナリデータ（mallocで確保）
 *  out_size : バイト数
 *
 * 戻り値:
 *  0  : 成功
 *  非0 : エラー
 */
SE_API int se_query(
    const char* db_path,
    const char* table,
    const char* pk_list,
    const char* col_list,
    unsigned char** out_buf,
    int* out_size
);

#endif
