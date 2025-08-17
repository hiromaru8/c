
# ----------------------------
# libhello_dllのビルド
# ----------------------------
# libhelloのソースファイルを指定
add_library(libhello_dll SHARED
    src/libhello.c
    )
target_include_directories(libhello_dll
  PUBLIC include
  )
set_target_properties(libhello_dll
  PROPERTIES OUTPUT_NAME "hello")
target_compile_options(libhello_dll
  PRIVATE -O3 -Wall -Wextra)
