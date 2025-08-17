add_executable(main
    main.c
    hello.c
    )
# greetingライブラリのインクルードディレクトリを教えてあげる
# PROJECT_SOURCE_DIRはこのプロジェクトのルートディレクトリの絶対パス
target_include_directories(main
  PUBLIC ${PROJECT_SOURCE_DIR}/hello/include
  )
set(CMAKE_EXE_LINKER_FLAGS "")