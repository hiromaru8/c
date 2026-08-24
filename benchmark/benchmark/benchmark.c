/**
 * @file benchmark.c
 * @brief TinyAES の AES-CTR 暗号化性能を測定するベンチマークプログラム
 *
 * @details
 * TinyAES DLL を動的にロードし、AES-CTR 暗号化の性能を
 * 複数のデータサイズについて測定する。
 *
 * ベンチマークでは、Windows の高精度パフォーマンスカウンタ
 * (QueryPerformanceCounter()) を使用して実行時間を計測し、
 * 総実行時間、平均実行時間、およびスループットを算出する。
 *
 * 測定対象となるデータは VirtualAlloc() により確保し、
 * touch_pages() を用いて事前に全ページへアクセスすることで、
 * デマンドページングによるページフォールトの影響を低減する。
 *
 * AES-256 の固定キーを使用し、IV は BCryptGenRandom() により
 * 各反復ごとにランダム生成する。
 *
 * 測定結果は標準出力へ表示するとともに、
 * CSV ファイル (tiny_aes.csv) に保存する。
 *
 * コマンドライン引数により、ベンチマークの実行回数を指定できる。
 *
 * @note
 * 本プログラムは Windows 専用であり、以下の Windows API を使用する。
 * - QueryPerformanceCounter()
 * - QueryPerformanceFrequency()
 * - VirtualAlloc()
 * - VirtualFree()
 * - BCryptGenRandom()
 *
 * @see tinyaes_loader.h
 */
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <bcrypt.h>
#include <ntstatus.h>

#include "tinyaes_loader.h"

#define DLL_NAME "libtinyAES256_CTR.dll"

/**
 * GROVAL VARIABLES
 */
static LARGE_INTEGER freq;  // 高精度パフォーマンスカウンタの周波数を格納する変数


/**
 * @brief ベンチマーク結果を格納する構造体
 */
typedef struct BenchmarkResult {
    size_t  data_size;      // ベンチマーク対象データサイズ（Byte）
    int     iterations;     // ベンチマーク実行回数

    int64_t elapsed_ticks;  // 総実行時間（パフォーマンスカウンタのティック数）

    double  total_time;     // 総実行時間（秒）
    double  average_time;   // 平均実行時間（秒）
    double  throughput;     // スループット（MiBit/s）
} BenchmarkResult;

/**
 * @brief ベンチマーク結果を計算する。
 *
 * @details
 * 計測した経過ティック数から総実行時間、
 * 平均実行時間、およびスループットを算出し、
 * BenchmarkResult に格納する。
 */
static void calculate_result(
    BenchmarkResult *result,
    size_t data_size,
    int iterations,
    int64_t elapsed)
{
    result->data_size     = data_size;
    result->iterations    = iterations;
    result->elapsed_ticks = elapsed;

    result->total_time =
        (double)elapsed / (double)freq.QuadPart;

    result->average_time =
        result->total_time / iterations;

    result->throughput =
        ((double)data_size * 8.0 * iterations) /
        (1024.0 * 1024.0) /
        result->total_time;
}

/**
 * @brief ベンチマーク結果を標準出力へ表示する。
 * @param[in] result ベンチマーク結果
 */
static void print_result(const BenchmarkResult *result)
{
    printf("  elapsed    : %lld ticks\n", result->elapsed_ticks);
    printf("  Total Time : %.8f sec\n", result->total_time);
    printf("  Average    : %.8f sec\n", result->average_time);
    printf("  Throughput : %.3f MiBit/s\n\n",
           result->throughput);
}

/**
 * @brief 現在の高精度パフォーマンスカウンタ値を取得する。
 *
 * @details
 * QueryPerformanceCounter() を呼び出し、
 * 高分解能タイマの現在値を返す。
 *
 * @return
 * 現在のパフォーマンスカウンタ値。
 */
static LARGE_INTEGER get_counter(void)
{
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return c;
}

/**
 * @brief メモリ領域の全ページへアクセスする。
 *
 * @details
 * 指定したメモリ領域の各ページへ書き込みを行い、
 * デマンドページングによる初回ページフォールトを
 * ベンチマーク開始前に発生させる。
 *
 * これにより、測定中に発生するページフォールトの影響を
 * 低減できる。
 *
 * Windows の GetSystemInfo() を使用してページサイズを取得する。
 *
 * @param[in,out] p
 * 対象となるメモリ領域。
 *
 * @param[in] bytes
 * メモリ領域のサイズ（Byte）。
 *
 * @note
 * Windows 専用実装。
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
    // bytes がページサイズの倍数でない場合でも、最後のページへ書き込みする
    if (bytes)
        p[bytes - 1] = 0;
}


/**
 * @brief ベンチマーク結果を CSV ファイルへ出力する。
 *
 * @details
 * 指定した CSV ファイルへ測定結果を追記する。
 * ファイルが存在しない場合、または空ファイルの場合は
 * ヘッダ行を出力してからデータを書き込む。
 *
 * @param[in] filename
 * 出力する CSV ファイル名。
 *
 * @param[in] result
 * ベンチマーク結果。
 *
 */
static void write_csv(
    const char *filename,
    const BenchmarkResult *result)
{
    int write_header = 0;

    /**
     * CSV ファイルが存在するか確認する。また、存在する場合はヘッダ行があるか確認する。
     */
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        // fopen() が失敗した場合、ファイルが存在しないと判断し、ヘッダ行を書き込む
        write_header = 1;
    } else {
        char buf[128];
        // fgets() でファイルの先頭行を読み込み、ヘッダ行が存在するか確認する
        if (!fgets(buf, sizeof(buf), fp))
            write_header = 1;
        fclose(fp);
    }

    /**
     * CSV ファイルへ測定結果を書き込む。
     * header が必要な場合はヘッダ行を書き込む（上書きモード）。
     * header が不要な場合はデータ行のみを書き込む（追記モード）。
     */
    fp = fopen(filename, write_header ? "w" : "a");
    if (!fp) {
        fprintf(stderr, "Cannot open %s\n", filename);
        return;
    }
    // ヘッダ行を書き込む
    if (write_header) {
        fprintf(fp,
            "DataSize(Bytes),Iterations,TotalTime(sec),Average(sec),Throughput(MiBit/s)\n");
    }
    // データ行を書き込む
    fprintf(fp,
            "%zu,%d,%.8f,%.8f,%.3f\n",
            result->data_size,
            result->iterations,
            result->total_time,
            result->average_time,
            result->throughput);

    fclose(fp);
}

/**
 * @brief TinyAES の AES-CTR 暗号化性能を測定する。
 *
 * @details
 * 指定サイズのデータに対して AES-CTR 暗号化を
 * 指定回数繰り返し実行し、
 * 総実行時間およびスループットを測定する。
 *
 * @param[in] aes
 * TinyAES ハンドル。
 *
 * @param[in] data_size
 * ベンチマーク対象データサイズ（Byte）。
 *
 * @param[in] iterations
 * 暗号化の実行回数。
 *
 * @retval 0
 * 正常終了。
 *
 * @retval 1
 * エラー。
 *
 * @note
 * ベンチマーク用に AES-256 の固定キーを使用し、
 * IV は各反復ごとに BCryptGenRandom() により生成する。
 */
static int measure_speed_tiny(
    TinyAES *aes,
    size_t data_size,
    int iterations,
    BenchmarkResult *result)
{
    int ret = 1;                    // 戻り値 0:成功, 1:失敗
    uint8_t *data = NULL;           // ベンチマーク用のデータバッファ
    // int locked = 0;              // VirtualLock() でメモリをロックしたかどうかのフラグ
    NTSTATUS status;                // BCryptGenRandom() の戻り値を受け取る変数
    AES_ctx_tiny ctx;               // TinyAES のコンテキスト構造体
    const uint8_t key[32] = {0};    // ベンチマークのため固定キーを使用
    uint8_t iv[16];                 // IVは毎回ランダムに生成するため、ここでは初期化しておく
    

    /**
     * ベンチマーク用のデータバッファをページ境界で確保する。
     *   VirtualAlloc() を使用することでページ単位の管理が可能となり、
     *   touch_pages() による事前アクセスや VirtualLock() と組み合わせることで
     *   ページフォールトやページアウトによる測定誤差を低減できる。
     */
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

    // // 各ページへ書き込みを行い、初回アクセス時のページフォールトをベンチマーク開始前に発生させる
    touch_pages(data, data_size);

    /**
     * VirtualLock()はサイズが大きいと失敗する場合があるため、コメントアウトしている。
     * ページアウトによる遅延を抑えるため、確保したメモリをロックする
     */
    // if (!VirtualLock(data, data_size)) {
    //     fprintf(stderr,
    //         "VirtualLock failed (%lu)\n",
    //         GetLastError());
    //     goto cleanup;
    // }
    // locked = 1;

    // /* AES-CTR 暗号化の初期化 */
    // tinyaes_init(aes, &ctx, key, iv);

    /**
     * ==========　測定開始　=========
     * 
     */
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
            fprintf(stderr,
            "BCryptGenRandom failed (0x%08X)\n",
            (unsigned)status);
            goto cleanup;
        }
        // printf("  Iteration %d: iv = ", i + 1);
        // for (int j = 0; j < sizeof(iv); j++) {
        //     printf("%02X", iv[j]);
        // }

        /* AES-CTR 暗号化の初期化 */
        tinyaes_init(aes, &ctx, key, iv);
        // memcpy(ctx.Iv, iv, sizeof(iv));  // IVを更新する

        /* AES-CTR 暗号化を実行 */
        tinyaes_xcrypt(aes, &ctx, data, data_size);
        // printf("  data[0] = %02X, data[%zu] = %02X\n", data[0], data_size - 1, data[data_size - 1]);
    }
    /**
     * ==========　測定終了　=========
     */ 
    LARGE_INTEGER end = get_counter();


    /**
     * 測定結果を計算する。
     * elapsed : 総実行時間（パフォーマンスカウンタのティック数）
     */
    int64_t elapsed = end.QuadPart - start.QuadPart;
    calculate_result(result, data_size, iterations, elapsed);



    ret = 0;

cleanup:
    
    /**
     * VirtualLock()を使用してメモリをロックしていた場合は、アンロックする
     */
    // if (locked)
    //     VirtualUnlock(data, data_size);

    /**
     * VirtualAlloc()で確保したメモリを解放する
     */
    if (data) 
        VirtualFree(data, 0, MEM_RELEASE);

    return ret;

}

/**
 * @brief TinyAES ベンチマークプログラムのエントリポイント。
 *
 * @details
 * コマンドライン引数からベンチマーク回数を取得し、
 * TinyAES DLL をロードして複数サイズのデータについて
 * AES-CTR 暗号化性能を測定する。
 *
 * @param[in] argc
 * コマンドライン引数の個数。
 *
 * @param[in] argv
 * コマンドライン引数。
 * argv[1] にベンチマーク回数を指定できる。
 *
 * @retval 0
 * 正常終了。
 *
 * @retval 1
 * エラー終了。
 */
int main(int argc, char *argv[])
{
    int ret = 0;
    int iterations = 1000;
    BenchmarkResult result;
 
    /**
     * コマンドライン引数からベンチマークのiterations回数を取得する。
     */
    if (argc >= 2) {
        char *end; // 変換できなかった文字のポインタを受け取るための変数
        
        // 文字列を整数に変換する。変換できなかった場合はエラーとする。
        long v = strtol(argv[1], &end, 10);
        if (*end != '\0' || v <= 0 || v > INT_MAX) {
            fprintf(stderr,
                "Usage: %s [iterations]\n",
                argv[0]);
            ret = 1;
            return ret;
        }
        // 変換結果を iterations に代入する。
        iterations = (int)v;
    }

    /**
     * 測定対象のデータサイズを配列で定義する。
     */
    const size_t sizes[] = {
        16ULL,
        32ULL,
        256ULL,
        512ULL,
        1024ULL,
        1024ULL * 1024ULL,
        10ULL * 1024ULL * 1024ULL
    };

    /**
     * TinyAES DLL をロードする。
     */
    const char *dll_path = DLL_NAME;
    TinyAES *aes = tinyaes_open(dll_path);
    if (!aes) {
        ret = 1;
        goto cleanup;
    }

    /**
     * 高精度パフォーマンスカウンタの周波数を取得する。
     */
    if (!QueryPerformanceFrequency(&freq)) {
        fprintf(stderr, "High resolution timer is not supported.\n");
            ret = 1;
            goto cleanup;
    }

    /**
     * ベンチマークの情報を表示する。
     */
    printf("======================\n");
    printf("tiny-AES CTR Benchmark\n");
    printf("Frequency  : %lld Hz\n", freq.QuadPart);
    printf("Iterations : %d\n", iterations);
    printf("DLL Path   : %s\n", dll_path);
    printf("======================\n");

    /**
     * 各データサイズについてベンチマークを実行する。
     */
    for (int i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++) {
        printf("Data Size : %zu bytes\n", sizes[i]);
        if (measure_speed_tiny(aes, sizes[i], iterations, &result) != 0) {
            fprintf(stderr, "Failed to measure speed for size %zu\n", sizes[i]);
            ret = 1;
            goto cleanup;
        }
        print_result(&result);
        write_csv("tiny_aes.csv", &result);
    }
    
cleanup:
    /**
     * TinyAES DLL をアンロードする。
     */
    if (aes)
        tinyaes_close(aes);

    printf("Benchmark finished.\n");
    printf("Press Enter to exit...");
    getchar();
    return ret;
}