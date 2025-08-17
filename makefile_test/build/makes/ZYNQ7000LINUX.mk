# ターゲット名とコンパイラ
CROSS_COMPILE_PATH 	:= /D/xilinx/Vitis/2023.2/gnu/aarch32/nt/gcc-arm-linux-gnueabi/bin/
CROSS_COMPILE		:= $(CROSS_COMPILE_PATH)arm-linux-gnueabihf-
CC 	:= $(CROSS_COMPILE)gcc.exe
CXX	:= $(CROSS_COMPILE)g++.exe



# ツールチェーンのincludeフォルダのパス
TOOLCHAIN_INCLUDE := /D/xilinx/Vitis/2023.2/gnu/aarch32/nt/gcc-arm-linux-gnueabi/cortexa9t2hf-neon-xilinx-linux-gnueabi/include


CFLAGS := $(CFLAGS) -mcpu=cortex-a9 -I $(TOOLCHAIN_INCLUDE) 



# ==================================
# Column
# ==================================
	# gcc-arm-linux-gnueabi
	# 概要: Linuxシステム用にコンパイルされたARMバイナリをターゲットとするGCCクロスコンパイラ。
	# 用途: LinuxカーネルやLinux上で動作するアプリケーションのコンパイル。
	# 特長:
	# ターゲット環境: Linuxオペレーティングシステム。
	# ABI: GNU EABI（Embedded Application Binary Interface）。
	# 標準ライブラリ: Linux標準のCライブラリ（通常はglibc）。
	# システムコール: Linuxカーネルのシステムコールを利用。
	# ファイルシステム: Linuxのファイルシステムと互換性。

	# gcc-arm-none-eabi
	# 概要: 組み込みシステム用にコンパイルされたARMバイナリをターゲットとするGCCクロスコンパイラ。
	# 用途: OSが存在しない（ベアメタル）環境や、RTOS（リアルタイムオペレーティングシステム）上で動作するアプリケーションのコンパイル。
	# 特長:
	# ターゲット環境: OSなし（ベアメタル）またはRTOS。
	# ABI: EABI（Embedded Application Binary Interface）。
	# 標準ライブラリ: 通常はnewlibなどの軽量な組み込み向けCライブラリ。
	# システムコール: OS非依存であり、直接ハードウェアを操作するコードが書ける。
	# ファイルシステム: 組み込みシステムの仕様に依存。


	# arm-linux-gnueabihf:  接頭辞は、ARMアーキテクチャ用のGNUツールチェーンを表しています
	# Aarch32:RMアーキテクチャの32ビットモードを指します。ARMアーキテクチャには、
	#         32ビットモード（Aarch32）と64ビットモード（Aarch64）の2つのモードがあります。
