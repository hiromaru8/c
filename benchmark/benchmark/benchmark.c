#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <bcrypt.h>
#include <ntstatus.h>

#include "tinyaes_loader.h"

#define ITERATIONS 1000



//-----------------------------------------------------------
// 高精度タイマ
//-----------------------------------------------------------
static LARGE_INTEGER freq;

static LARGE_INTEGER get_counter(void)
{
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return c;
}

//-----------------------------------------------------------
// ページフォールト防止
//-----------------------------------------------------------
/**
 * 仮想メモリ領域の全ページへ書き込み、初回アクセス時のページフォールトを
 * あらかじめ発生させる。
 *
 * 多くのOSでは、確保したメモリに対応する物理ページは初回アクセス時に
 * 割り当てられる（デマンドページング）。リアルタイム性が重要な処理の前に
 * 本関数を呼び出すことで、実行中のページフォールトによる遅延を低減できる。
 *
 * この実装は Windows 専用であり、ページサイズの取得には GetSystemInfo()
 * を使用する。
 *
 * 必要なヘッダ: windows.h
 */
static void touch_pages(uint8_t *p, size_t bytes)
{
    // システムのページサイズを取得する
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t page = si.dwPageSize;

    // printf("Page Size : %zu bytes\n", page);
    // 各ページへ書き込み、初回アクセス時のページフォールトを事前に発生させる
    for (size_t i = 0; i < bytes; i += page)
        p[i] = 0;
    // bytes がページサイズの倍数でない場合でも、最後のページへアクセスする
    if (bytes)
        p[bytes - 1] = 0;
}


//-----------------------------------------------------------
// CSV出力
//-----------------------------------------------------------
static void write_csv(const char *filename,
                      size_t data_size,
                      int iterations,
                      double total_time,
                      double throughput)
{
    int write_header = 0;

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        write_header = 1;
    } else {
        char buf[128];
        if (!fgets(buf, sizeof(buf), fp))
            write_header = 1;
        fclose(fp);
    }

    fp = fopen(filename, write_header ? "w" : "a");
    if (!fp) {
        fprintf(stderr, "Cannot open %s\n", filename);
        return;
    }

    if (write_header) {
        fprintf(fp,
            "DataSize(Bytes),Iterations,TotalTime(sec),Average(sec),Throughput(MiBit/s)\n");
    }

    fprintf(fp,
            "%zu,%d,%.8f,%.8f,%.3f\n",
            data_size,
            iterations,
            total_time,
            total_time / iterations,
            throughput);

    fclose(fp);
}

//-----------------------------------------------------------
// tiny_aes ベンチマーク
//-----------------------------------------------------------
static int measure_speed_tiny(const char *dll_path,
                                 size_t data_size,
                                 int iterations)
{
    int ret = 1;
    uint8_t *data = NULL;
    // int locked = 0;     // VirtualLock() でメモリをロックしたかどうかのフラグ
    NTSTATUS status = STATUS_SUCCESS;
    TinyAES *aes = tinyaes_open(dll_path);
    if (!aes) {
        return 1;
    }


    AES_ctx_tiny ctx;
    // ベンチマークのため固定キーを使用
    const uint8_t key[32] = {0};
    // IVは毎回ランダムに生成するため、ここでは初期化しておく
    uint8_t iv[16];
    

    // ベンチマーク用のデータバッファをページ境界で確保する。
    // VirtualAlloc() を使用することでページ単位の管理が可能となり、
    // touch_pages() による事前アクセスや VirtualLock() と組み合わせることで
    // ページフォールトやページアウトによる測定誤差を低減できる。
    data = VirtualAlloc(NULL,
                        data_size,
                        MEM_RESERVE | MEM_COMMIT,
                        PAGE_READWRITE);

    if (!data) {
        fprintf(stderr,
                "VirtualAlloc failed (%lu)\n",
                GetLastError());
        goto cleanup;
    }

    // 各ページへ事前にアクセスし、初回アクセス時のページフォールトをベンチマーク開始前に発生させる
    touch_pages(data, data_size);

    // VirtualLock()はサイズが大きいと失敗する場合があるため、コメントアウトしている。
    //// ページアウトによる遅延を抑えるため、確保したメモリをロックする
    // if (!VirtualLock(data, data_size)) {
    //     fprintf(stderr,
    //         "VirtualLock failed (%lu)\n",
    //         GetLastError());
    //     goto cleanup;
    // }
    // locked = 1;


    // =====　測定開始　=====
    LARGE_INTEGER start = get_counter();

    // AES-CTR暗号化をiterations回実行
    for (int i = 0; i < iterations; i++) {
        // ivは乱数とする
        status = BCryptGenRandom(
            NULL,
            iv,
            sizeof(iv),
            BCRYPT_USE_SYSTEM_PREFERRED_RNG);
        if (status != STATUS_SUCCESS)
        {
            printf("BCryptGenRandom failed\n");
            goto cleanup;
        }
        // printf("  Iteration %d: iv = ", i + 1);
        // for (int j = 0; j < sizeof(iv); j++) {
        //     printf("%02X", iv[j]);
        // }
        tinyaes_init(aes, &ctx, key, iv);
        tinyaes_xcrypt(aes, &ctx, data, data_size);
    }

    // =====　測定終了　=====
    LARGE_INTEGER end = get_counter();
    int64_t elapsed = end.QuadPart - start.QuadPart;
    double total_time = (double)elapsed / (double)freq.QuadPart;

    double throughput =
        ((double)data_size * 8.0 * iterations) /
        (1024.0 * 1024.0) /
        total_time;

    printf("  elapsed    : %lld ticks\n", elapsed);
    printf("  Total Time : %.8f sec\n", total_time);
    printf("  Average    : %.8f sec\n", total_time / iterations);
    printf("  Throughput : %.3f MiBit/s\n\n", throughput);

    write_csv("tiny_aes.csv",
              data_size,
              iterations,
              total_time,
              throughput);


    ret = 0;
    goto cleanup;

cleanup:
    
    // VirtualLock()を使用してメモリをロックしていた場合は、アンロックする
    // if (locked)
    //     VirtualUnlock(data, data_size);

    if (data) 
        VirtualFree(data, 0, MEM_RELEASE);

    if (aes)
        tinyaes_close(aes);

    return ret;

}

//-----------------------------------------------------------
// main
//-----------------------------------------------------------
int main(int argc, char *argv[])
{
    int iterations = 1000;
    if (argc >= 2) {
        iterations = atoi(argv[1]);
    }

    if (iterations <= 0) {
        fprintf(stderr,
                "Usage: %s [iterations]\n",
                argv[0]);
        return 1;
    }

    const size_t sizes[] = {
        16ULL,
        32ULL,
        256ULL,
        512ULL,
        1024ULL,
        1024ULL * 1024ULL,
        10ULL * 1024ULL * 1024ULL
    };

    printf("======================\n");
    printf("tiny-AES CTR Benchmark\n");
    printf("======================\n");
    if (!QueryPerformanceFrequency(&freq)) {
        fprintf(stderr, "High resolution timer is not supported.\n");
        return 1;
    }
    printf("Frequency : %lld Hz\n", freq.QuadPart);
    printf("Iterations : %d\n", iterations);


    for (int i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++) {
        printf("Data Size : %zu bytes\n", sizes[i]);
        if (measure_speed_tiny("libtiny_aes.dll", sizes[i], iterations) != 0) {
            fprintf(stderr, "Failed to measure speed for size %zu\n", sizes[i]);
            return 1;
        }
    }

    return 0;
}