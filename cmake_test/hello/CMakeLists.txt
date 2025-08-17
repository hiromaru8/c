# ----------------------------
# hello_use_dll_loadのビルド
# ----------------------------
# helloのソースファイルを指定
add_executable(hello_use_dll_load
    src/main_use_dll_load.c
    src/hello.c
    )
# helloのインクルードディレクトリを指定
target_include_directories(hello_use_dll_load
  PUBLIC include  
  ../libhello/include
  )
# helloの出力名を指定
set_target_properties(hello_use_dll_load
  PROPERTIES OUTPUT_NAME "hello_use_dll_load")
target_compile_options(hello_use_dll_load
  PRIVATE -O3 -Wall -Wextra)

# ----------------------------
# hello_use_dll_staticのビルド
# ----------------------------
# helloのソースファイルを指定
add_executable(hello_use_dll_static
    src/main_use_dll_static.c
    src/hello.c
    )
# helloのインクルードディレクトリを指定
target_include_directories(hello_use_dll_static
  PUBLIC include
  ../libhello/include
  )
# helloの出力名を指定
set_target_properties(hello_use_dll_static
  PROPERTIES OUTPUT_NAME "hello_use_dll_static")
target_compile_options(hello_use_dll_static
  PRIVATE -O3 -Wall -Wextra)

# libhelloのリンクライブラリを指定
target_link_libraries(hello_use_dll_static
  PRIVATE libhello_dll
  )
