
# サンプルコードの作成

## 要件

* CUIプログラムの作成
* Pythonから呼び出すためのDLLの作成

### CUIプログラムの入出力条件

* コマンドライン引数は下記とする。
  * DBファイルパスを指定できる。
  * テーブル名をしていできる。
  * PKを指定できる
    * 1つ以上のPKを指定できる
    * レコードすべてを対象にでき、インデックスは昇順。
  * カラムを指定できる
    * 1つ以上のカラムを指定できる
    * すべてのカラムを指定できる
  * 出力先を指定できる
* 出力
  * バイナリファイル
  * ログは標準出力
  * 終了コード
  
CUI 引数仕様（例）

``` powershell
sqlite_export.exe ^
  --db test.db ^
  --table tb1 ^
  --pk id_src,id_dest ^
  --pk-values "(1,10),(2,20),(5,99)" ^
  --columns data1,data2 ^
  --out out.bin
```

特殊指定

| 指定            | 意味       |
| ------------- | -------- |
| `--pk *`      | PK全件（昇順） |
| `--columns *` | 全カラム     |

## DLL

* 入力は基本的にCUIと同じ。ただ、ファイルパスはPython側で吸収する
* Python側へはDBからの取得結果を返す

## ソースの条件

* 言語はC言語
* コンパイラはMINGW
* プログラムは2種類
  * CUI用のEXE
  * PythonからDLLを呼び出す
* cmakeを使う

## SQLite3のデータベースルール

### テーブル構造

``` sql
CREATE TABLE tb1 (
    id_src  INTEGER,
    id_dest INTEGER,
    data1   BLOB,
    data2   BLOB,
    PRIMARY KEY (id_src, id_dest)
);
CREATE TABLE tb2 (
    id  INTEGER,
    data1   BLOB,
    data2   BLOB,
    PRIMARY KEY (id)
);
```

