# ----------------------------
# 基本設定
# ----------------------------
cmake_minimum_required(VERSION 3.20)
project(AES_Project C)
set(CMAKE_C_STANDARD 99)

# 出力ディレクトリの設定
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")

# 共通コンパイルオプション
add_compile_options(
    -O3                 # 最適化レベル3を有効化
    -fverbose-asm       # 詳細なアセンブリ出力を有効化
    -masm=intel         # Intelアセンブリ構文を使用し、 -fverbose-asm と組み合わせて、アセンブリ出力を詳細にする
)
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
#------------------------------------------------------------
# tiny-AES DLL生成関数
#------------------------------------------------------------
function(add_tinyaes_dll TARGET_NAME KEY_SIZE MODE)

    add_library(${TARGET_NAME} SHARED
        tiny_aes_dll/src/aes.c
    )

    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    #
    # 暗号モード
    #
    if(MODE STREQUAL "CTR")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                CTR=1
                CBC=0
                ECB=0
        )

    elseif(MODE STREQUAL "CBC")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                CTR=0
                CBC=1
                ECB=0
        )

    elseif(MODE STREQUAL "ECB")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                CTR=0
                CBC=0
                ECB=1
        )
    endif()

    #
    # 鍵長
    #
    if(KEY_SIZE STREQUAL "128")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                AES128=1
        )

    elseif(KEY_SIZE STREQUAL "192")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                AES192=1
        )

    elseif(KEY_SIZE STREQUAL "256")
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                AES256=1
        )
    endif()

    #
    # DLL名(libプレフィックスは付けない)
    #
    set_target_properties(${TARGET_NAME}
        PROPERTIES
            PREFIX ""
            OUTPUT_NAME ${TARGET_NAME}
    )
endfunction()
#------------------------------------------------------------
# CTR
#------------------------------------------------------------
add_tinyaes_dll(libtinyAES128_CTR 128 CTR)
add_tinyaes_dll(libtinyAES192_CTR 192 CTR)
add_tinyaes_dll(libtinyAES256_CTR 256 CTR)

#------------------------------------------------------------
# ECB
#------------------------------------------------------------
add_tinyaes_dll(libtinyAES128_ECB 128 ECB)
add_tinyaes_dll(libtinyAES192_ECB 192 ECB)
add_tinyaes_dll(libtinyAES256_ECB 256 ECB)

#------------------------------------------------------------
# CBC
#------------------------------------------------------------
add_tinyaes_dll(libtinyAES128_CBC 128 CBC)
add_tinyaes_dll(libtinyAES192_CBC 192 CBC)
add_tinyaes_dll(libtinyAES256_CBC 256 CBC)

# ----------------------------
# ttable_aes_dll
# ----------------------------
add_library(ttable_aes SHARED
    ttable_aes_dll/src/aes_256_ctr.c
)
target_include_directories(ttable_aes PUBLIC ttable_aes_dll/include)
set_target_properties(ttable_aes PROPERTIES OUTPUT_NAME "ttable_aes_t")

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
add_executable(benchmark_test 
                benchmark/benchmark.c
                benchmark/tinyaes_loader.c
            )
target_include_directories(benchmark_test PRIVATE tiny_aes_dll/include ttable_aes_dll/include)
target_link_libraries(benchmark_test PRIVATE
    bcrypt
)
set_target_properties(benchmark_test PROPERTIES OUTPUT_NAME "benchmark_test")

# ----------------------------
# アセンブリ出力用ターゲット
# ----------------------------
add_custom_target(asm_all
    COMMAND ${CMAKE_C_COMPILER} -S -fverbose-asm -masm=intel -Ittable_aes_dll/include ${CMAKE_SOURCE_DIR}/ttable_aes_dll/src/aes_256_ctr.c -o ${CMAKE_BINARY_DIR}/ttable_aes_256_ctr.s
    COMMAND ${CMAKE_C_COMPILER} -S -fverbose-asm -masm=intel -maes -msse2 -Iaes_ni/include ${CMAKE_SOURCE_DIR}/aes_ni/test.c -o ${CMAKE_BINARY_DIR}/aes_ni_test.s
    COMMAND ${CMAKE_C_COMPILER} -S -fverbose-asm -masm=intel -Ibenchmark/include ${CMAKE_SOURCE_DIR}/benchmark/test.c -o ${CMAKE_BINARY_DIR}/benchmark_test.s
    COMMENT "Generating assembly files (.s) in build directory"
    VERBATIM
)

# make / ninja で asm_all も同時にビルドさせたい場合
add_dependencies(asm_all ttable_aes aes_ni_test benchmark_test)

# ----------------------------
# ビルド時の詳細表示
# ----------------------------
# set(CMAKE_VERBOSE_MAKEFILE ON)
