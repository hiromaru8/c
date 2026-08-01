#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <bcrypt.h>
#include <ntstatus.h>
#define ITERATIONS 100

// ---- tiny_aes ----
typedef struct AES_ctx_tiny {
    uint8_t RoundKey[240];
    uint8_t Iv[16];
} AES_ctx_tiny;

typedef void (*AES_INIT)(AES_ctx_tiny*, const uint8_t*, const uint8_t*);
typedef void (*AES_XCRYPT)(AES_ctx_tiny*, uint8_t*, size_t);


static AES_INIT load_aes_init(HMODULE h)
{
    // AES_init_ctx_iv関数のアドレスを取得する
    // unionを使って、FARPROC型の関数ポインタをAES_INIT型の関数ポインタに変換する
    // FARPROC型の関数ポインタは、GetProcAddress関数が返す型であり、汎用的な関数ポインタ型である
    // AES_INIT型の関数ポインタは、AES_init_ctx_iv関数の型であり、引数と戻り値の型が一致する必要がある
    union {
        FARPROC fp;
        AES_INIT fn;
    } u;

    u.fp = GetProcAddress(h, "AES_init_ctx_iv");
    return u.fn;
}

static AES_XCRYPT load_aes_xcrypt(HMODULE h)
{
    // AES_CTR_xcrypt_buffer関数のアドレスを取得する
    // unionを使って、FARPROC型の関数ポインタをAES_XCRYPT型の関数ポインタに変換する
    // FARPROC型の関数ポインタは、GetProcAddress関数が返す型であり、汎用的な関数ポインタ型である
    // AES_XCRYPT型の関数ポインタは、AES_CTR_xcrypt_buffer関数の型であり、引数と戻り値の型が一致する必要がある
    union {
        FARPROC fp;
        AES_XCRYPT fn;
    } u;

    u.fp = GetProcAddress(h, "AES_CTR_xcrypt_buffer");
    return u.fn;
}

//-----------------------------------------------------------
// 高精度タイマ
//-----------------------------------------------------------
static double get_time_sec(void)
{
    // QueryPerformanceCounter() と QueryPerformanceFrequency() を使用して、高精度な時間計測を行う
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER counter;
    // QueryPerformanceFrequency() は、パフォーマンスカウンタの周波数を取得する関数であり、1秒あたりのカウント数を返す
    if (freq.QuadPart == 0)
        QueryPerformanceFrequency(&freq);
    // QueryPerformanceCounter() は、パフォーマンスカウンタの現在値を取得する関数であり、カウント数を返す
    QueryPerformanceCounter(&counter);
    // パフォーマンスカウンタの現在値を周波数で割ることで、秒単位の時間を計算する
    return (double)counter.QuadPart / (double)freq.QuadPart;
}
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
            ITERATIONS,
            total_time,
            total_time / ITERATIONS,
            throughput);

    fclose(fp);
}

//-----------------------------------------------------------
// tiny_aes ベンチマーク
//-----------------------------------------------------------
static double measure_speed_tiny(const char *dll_path,
                                 size_t data_size)
{
    HMODULE h = LoadLibraryA(dll_path);
    if (!h) {
        fprintf(stderr,
                "LoadLibrary failed (%lu)\n",
                GetLastError());
        return -1.0;
    }

    AES_INIT aes_init = load_aes_init(h);
    AES_XCRYPT aes_xcrypt = load_aes_xcrypt(h);

    if (!aes_init || !aes_xcrypt) {
        fprintf(stderr, "GetProcAddress failed\n");
        FreeLibrary(h);
        return -1.0;
    }

    AES_ctx_tiny ctx;

    uint8_t key[32] = {0};
    uint8_t iv[16]  = {0};
    unsigned char random[32];

    NTSTATUS status = BCryptGenRandom(
        NULL,
        random,
        sizeof(random),
        BCRYPT_USE_SYSTEM_PREFERRED_RNG
    );

    if (status != STATUS_SUCCESS) {
        printf("BCryptGenRandom failed\n");
        return -1.0;
    }
    
    // ベンチマーク用のデータバッファをページ境界で確保する。
    // VirtualAlloc() を使用することでページ単位の管理が可能となり、
    // touch_pages() による事前アクセスや VirtualLock() と組み合わせることで
    // ページフォールトやページアウトによる測定誤差を低減できる。
    uint8_t *data =
        (uint8_t *)VirtualAlloc(NULL,
                                data_size,
                                MEM_RESERVE | MEM_COMMIT,
                                PAGE_READWRITE);

    if (!data) {
        fprintf(stderr,
                "VirtualAlloc failed (%lu)\n",
                GetLastError());
        FreeLibrary(h);
        return -1.0;
    }

    // 各ページへ事前にアクセスし、初回アクセス時のページフォールトをベンチマーク開始前に発生させる
    touch_pages(data, data_size);

    // ページアウトによる遅延を抑えるため、確保したメモリをロックする
    VirtualLock(data, data_size);

    printf("Data Size : %zu bytes\n", data_size);

    // =====　測定開始　=====
    // double start = get_time_sec();
    if (!QueryPerformanceFrequency(&freq)) {
        fprintf(stderr, "High resolution timer is not supported.\n");
        return 1;
    }
    LARGE_INTEGER start = get_counter();

    // AES-CTR暗号化をITERATIONS回実行
    for (int i = 0; i < ITERATIONS; i++) {
        // ivは乱数とする
        if (BCryptGenRandom(
                NULL,
                iv,
                sizeof(iv),
                BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0)
        {
            printf("BCryptGenRandom failed\n");
            VirtualFree(data, 0, MEM_RELEASE);
            FreeLibrary(h);
            return -1.0;
        }
        // printf("  Iteration %d: iv = ", i + 1);
        // for (int j = 0; j < sizeof(iv); j++) {
        //     printf("%02X", iv[j]);
        // }
        aes_init(&ctx, key, iv);
        aes_xcrypt(&ctx, data, data_size);
    }

    // =====　測定終了　=====
    // double end = get_time_sec();
    LARGE_INTEGER end = get_counter();

    double total_time = (double)(end.QuadPart - start.QuadPart) / (double)freq.QuadPart;

    double throughput =
        ((double)data_size * 8.0 * ITERATIONS) /
        (1024.0 * 1024.0) /
        total_time;

    printf("  Total Time : %.8f sec\n", total_time);
    printf("  Average    : %.8f sec\n", total_time / ITERATIONS);
    printf("  Throughput : %.3f MiBit/s\n\n", throughput);

    write_csv("tiny_aes.csv",
              data_size,
              total_time,
              throughput);

    VirtualFree(data, 0, MEM_RELEASE);
    FreeLibrary(h);

    return total_time;
}

//-----------------------------------------------------------
// main
//-----------------------------------------------------------
int main(void)
{
    const size_t sizes[] = {
        256ULL,
        512ULL,
        1024ULL,
        1024ULL * 1024ULL,
        10ULL * 1024ULL * 1024ULL
    };

    printf("tiny-AES CTR Benchmark\n");
    printf("======================\n");

    for (int i = 0; i < (int)(sizeof(sizes) / sizeof(sizes[0])); i++) {
        measure_speed_tiny("libtiny_aes.dll", sizes[i]);
    }

    return 0;
}