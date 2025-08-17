# 概要

Cmakeを使用したMakefileの作成とビルドの実行のメモ。

* WindowsによるホストPCによるビルドと
* Vitisのコンパイラを使用したクロスコンパイルの手順

## MINGWによるビルド

1. buildディレクトリを作成し、移動

    ``` powershell
    mkdir build_mingw
    cd build_mingw
    ```

2. Makefileの生成

    下記を実行する。

    ``` powershell
    cmake -G "MinGW Makefiles"  -DCMAKE_BUILD_TYPE=Release ..
    ```

    または

    ``` powershell
    $env:PATH="C:\msys64\mingw64\bin;$env:PATH"
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    ```

    または（ただし、公式が提供しているコンパイラは古い）

    ``` powershell
    $env:PATH="C:\MinGW\bin;$env:PATH"
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
    ```

    次の警告がでたら、コンパイラのパスを環境変数に設定する。

    !!! warning "コンパイラのパスが環境変数にない場合は下記のエラー"
        -- The C compiler identification is unknown
        CMake Error at CMakeLists.txt:6 (project):
        No CMAKE_C_COMPILER could be found.
        Tell CMake where to find the compiler by setting either the environment
        variable "CC" or the CMake cache entry CMAKE_C_COMPILER to the full path to
        the compiler, or to the compiler name if it is in the PATH.


    !!! note "-G "MinGW Makefiles""  
        * ジェネレータ指定  
        * 「どのビルドツール向けの設定を生成するか」を選びます。  
        * "MinGW Makefiles" を指定すると、Makefile が MinGW 用に作成され、後で mingw32-make でビルドできるようになります。
        * 👉 もし -G を省略すると、環境によっては Visual Studio 用や Ninja 用が選ばれるので、明示的に指定しています。

    !!! note "-DCMAKE_BUILD_TYPE=Release"
        * ビルドタイプの指定
        * Makefile や Ninja のような 単一構成ジェネレータでは、ここで Debug / Release / RelWithDebInfo / MinSizeRel を指定する必要があります。  
        * Exsample  
          * Release : 最適化有効（-O3）、デバッグ情報なし
          * Debug : デバッグ情報付き（-g）、最適化なし
        * Visual Studio のような マルチ構成ジェネレータでは無視され、代わりに --config Release を指定する。

3. ビルド実行

    ``` powershell
    cmake --build .
    ```

    !!! note "内部的には.."
        現在のディレクトリ (. = ビルドディレクトリ) にある CMake で生成されたビルドシステムを実行します。
        ジェネレータに応じて、内部的には次を呼び出します:
        |ビルドシステム     |実行プログラム|
        |--                 |--            |
        |MinGW Makefiles    | mingw32-make |
        |Unix Makefiles     | make         |
        |Ninja              | ninja        |
        |Visual Studio      | MSBuild.exe  |


## クロスコンパイル（Vitis）

1. buildディレクトリを作成し、移動

    ``` powershell
    mkdir build_vitis
    cd build_vitis
    ```

2. Makefileの生成

    ``` powershell
    cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE="D:/work_c/cmake_test/vitis_toolchain.cmake"  -DCMAKE_BUILD_TYPE=Release ..
    ```

3. ビルド実行

    ``` powershell
    cmake --build .
    ```
