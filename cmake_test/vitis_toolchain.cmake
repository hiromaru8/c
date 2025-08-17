# ターゲット用Cコンパイラ
set(CMAKE_SYSTEM_NAME Linux)   # ターゲットOSに応じて変更 (BareMetalなら Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Vitis付属GCCのパス
set(VITIS_GCC_PATH "D:/xilinx/Vitis/2023.2/gnu/aarch32/nt/gcc-arm-linux-gnueabi/bin")

# Cコンパイラ
set(CMAKE_C_COMPILER "${VITIS_GCC_PATH}/arm-linux-gnueabihf-gcc.exe")
# C++コンパイラ
set(CMAKE_CXX_COMPILER "${VITIS_GCC_PATH}/arm-linux-gnueabihf-g++.exe")

# 必要に応じてリンカ、アーカイバ
set(CMAKE_AR "${VITIS_GCC_PATH}/arm-linux-gnueabihf-ar.exe")
set(CMAKE_LINKER "${VITIS_GCC_PATH}/arm-linux-gnueabihf-ld.exe")
set(CMAKE_RANLIB "${VITIS_GCC_PATH}/arm-linux-gnueabihf-ranlib.exe")

# ターゲット固有フラグ
# set(CMAKE_C_FLAGS "--specs=nosys.specs -mcpu=cortex-a9 -O2")
# set(CMAKE_CXX_FLAGS "--specs=nosys.specs -mcpu=cortex-a9 -O2")
