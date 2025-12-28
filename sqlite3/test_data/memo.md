
# command memo

https://sqlite.org/download.html

``` cmd
sqlite3.exe test.db < init.sql
sqlite3.exe test.db < insert.sql
sqlite3.exe test.db < select.sql
```

``` powershell
Get-Content .\init.sql | sqlite3.exe sample.db
```
