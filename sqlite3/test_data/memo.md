
# command memo
このメモの実行プログラムは[SQLite公式](https://sqlite.org/download.html)から入手。

Precompiled Binaries for Windows

``` cmd
sqlite3.exe test.db < init.sql
sqlite3.exe test.db < insert.sql
sqlite3.exe test.db < select.sql
```

``` powershell
Get-Content .\init.sql | sqlite3.exe sample.db
```
