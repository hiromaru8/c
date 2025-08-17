# ==============================================================================
# MACRO
# ==============================================================================
# ------------------------------------------------
# 引数
# ------------------------------------------------
# 引数のデフォルト値を設定
TARGET_PLATFORM ?= ZYNQ7000LINUX
$(warning TARGET_PLATFORM = $(TARGET_PLATFORM))



# ------------------------------------------------
# コンパイラ設定
# ------------------------------------------------
# 共通の設定を含むファイルを取り込む
include makes/common.mk

# プラットフォームごとのコンパイラ設定
ifeq ($(TARGET_PLATFORM), ZYNQ7000LINUX)
    $(warning BUILD ZYNQ7000LINUX)
    include makes/$(TARGET_PLATFORM).mk
else ifeq ($(TARGET_PLATFORM), ZYNQ7000)
    $(warning error $(TARGET_PLATFORM))
	@exit 1
else
    $(warning error $(TARGET_PLATFORM))
	@exit 1
endif


# ------------------------------------------------
# ビルド対象のファイルとディレクトリ
# ------------------------------------------------
# ターゲットプログラム
TARGET_EXEC := hello_world

# ディレクトリ作成
OBJDIR := $(TARGET_PLATFORM)/obj_$(TARGET_EXEC)
BINDIR := $(TARGET_PLATFORM)
$(shell mkdir -p $(OBJDIR) $(BINDIR))


# sorce
MODDIR := ../hello
SRCDIR := $(MODDIR)/src
INCDIR := $(MODDIR)/include
# includeフォルダ追加
CFLAGS := $(CFLAGS) -I $(INCDIR)

# ソースファイルとオブジェクトファイルのリスト
SRCS := $(wildcard $(SRCDIR)/*.c) \
        $(wildcard $(SRCDIR)/*.cpp) \
        $(wildcard $(SRCDIR)/*.s) 
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))


# ==============================================================================
# TARGET
# ==============================================================================
# ------------------------------------------------
#  ビルド
# ------------------------------------------------
# ターゲット
$(BINDIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) -o $@ $^


# オブジェクトファイルのコンパイル (C言語)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# オブジェクトファイルのコンパイル (C++)
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


# ------------------------------------------------
# クリーンアップ
# ------------------------------------------------
# 削除対象： oファイル、実行ファイル
.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(BINDIR)/$(TARGET_EXEC)

# 削除対象： ディレクトリごと
.PHONY: allclean
allclean:
	rm -rf $(OBJDIR)  $(BINDIR)



# ------------------------------------------------
# Makefileのヘルプを表示する
# ------------------------------------------------
.PHONY: help
help:
	@echo "使い方: make [ターゲット]"
	@echo ""
	@echo "利用可能なターゲット:"
	@echo "  help           : このヘルプを表示する"
	@echo "  clean          : ビルド生成物を削除する(oファイル、実行ファイル)"
	@echo "  allclean       : ビルド生成物を削除する(ディレクトリごと)"

