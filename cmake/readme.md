# このプロジェクト

CMakeLists.txtのテンプレートを作りたい

## 実行方法のメモ

``` powershell
$env:PATH="C:\msys64\mingw64\bin;$env:PATH"

cmake -S . -B build -G "MinGW Makefiles" 
cmake --build build
```

