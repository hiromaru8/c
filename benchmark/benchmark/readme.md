# 概要

## 1. 将来的な変更ポイント

## 1.1 プログラム構造について

### 1.1.1 measure_speed_tiny()

`測定結果は標準出力へ表示するとともに、CSV ファイルへ追記する。`

**「性能測定」だけでなく「表示」と「CSV出力」まで行っている**ため、責務が増えているので、今後リファクタリングするなら、

* measure_speed_tiny() … 測定のみ
* print_result() … 標準出力
* write_csv() … CSV保存

という3つに分ける。

### 2. 処理の拡張

* [tiny-AES-c](https://github.com/kokke/tiny-AES-C)には、ほかの鍵長や暗号利用モードがあり、対応を増やす。
* `tiny-AES-c`以外のAIS-NI、AES-GCMなど追加
* 対象とする測定対象を増やす（今回はIV、鍵拡大を含めているが）
