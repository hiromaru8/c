# memo

## 実行例

サンプルのデータベース

``` powershell
.\create_sample_db.exe
```

-> sample_test.db

.\sqlite_export_cli.exe `
  --db sample_test.db `
  --table tb1 `
  --pk id_src,id_dest `
  --pk-values "(1,10),(2,20)" `
  --columns data1,data2 `
  --out out.bin

## カバレッジ取得方法（MinGW）

cmake -S . -B build -G "MinGW Makefiles" 
cmake --build build
cd build
./test_build_sql
gcov.exe .\CMakeFiles\sqlite_export_core.dir\src\common.c.gcda  
