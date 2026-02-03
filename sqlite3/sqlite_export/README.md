# sqlite_export

SQLiteデータベースの特定テーブルから指定した主キーのレコードをバイナリ形式でエクスポート・更新するツール群。

## ビルド方法（MinGW）

``` powershell
$env:PATH="C:\msys64\mingw64\bin;$env:PATH"

cmake -S . -B build -G "MinGW Makefiles" 
cmake --build build
```

依存関係確認

``` powershell
objdump -p .\build\sqlite_export_cli.exe | Select-String dll
```

## 実行例

### サンプルのデータベース作成

``` powershell
.\create_sample_db.exe
```

-> sample_test.db

### エクスポート

``` powershell
.\sqlite_export_cli.exe export`
  --db sample_test.db `
  --table tb1 `
  --pk id_src,id_dest `
  --pk-values "(1,10),(2,20)" `
  --columns data1,data2 `
  --out out.bin
```

### Update

``` powershell
.\sqlite_export_cli.exe update `
  --db sample_test.db `
  --table tb1 `
  --pk id_src,id_dest `
  --pk-values "(1,1)" `
  --set "data1=X'0102', data2=X'0304'" `
  --out out.bin
```

## カバレッジ取得方法（MinGW）

``` powershell
cmake -S . -B build -G "MinGW Makefiles" 
cmake --build build
cd build
./test_build_sql
gcov.exe .\CMakeFiles\sqlite_export_core_cov.dir\src\common.c.gcda
gcov.exe .\CMakeFiles\sqlite_export_core_cov.dir\src\cli_run.c.gcda
```
