# memo

``` powershell
$env:PATH="C:\msys64\mingw64\bin;$env:PATH"      

mkdir build

cd build

cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..

cmake --build . --config Release
```


cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe ..
