# memo

## 実行例

sqlite_export.exe ^
  --db test.db ^
  --table tb1 ^
  --pk id_src,id_dest ^
  --pk-values "(1,10),(2,20),(5,99)" ^
  --columns data1,data2 ^
  --out out.bin

## カバレッジ取得方法（MinGW）

cmake -S . -B build
cmake --build build
cd build
./test_build_sql
gcov ../src/common.c
