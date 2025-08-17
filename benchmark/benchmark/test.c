#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

#define AES_MAXNR       14
#define AES_BLOCK_SIZE  16
#define ITERATIONS      100

// ---- tiny_aes ----
typedef struct AES_ctx_tiny {
    uint8_t RoundKey[240];
    uint8_t Iv[16];
} AES_ctx_tiny;
typedef void (*AES_INIT)(AES_ctx_tiny*, const uint8_t*, const uint8_t*);
typedef void (*AES_XCRYPT)(AES_ctx_tiny*, uint8_t*, size_t);

// ---- T-table AES ----
struct aes_key_st {
    unsigned int rd_key[4 * (AES_MAXNR + 1)];
    uint8_t Iv[AES_BLOCK_SIZE];
};
typedef struct aes_key_st AES_ctx_ttable;
typedef void (*AES_INIT_1)(AES_ctx_ttable*, const uint8_t*, const uint8_t*);
typedef void (*AES_XCRYPT_1)(AES_ctx_ttable*, uint8_t*, size_t);

// ---- timing ----
static double get_time_sec(void) {
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER counter;
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
}

// 全ページを事前に「触って」コミットを完了させる
static void touch_pages(uint8_t *p, size_t bytes) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    size_t page = si.dwPageSize;
    for (size_t i = 0; i < bytes; i += page) {
        p[i] = 0;
    }
    if (bytes) {
        p[bytes - 1] = 0; // 末尾も念のため触る
    }
}

// CSV 出力用の関数
static void write_csv(const char *filename, size_t data_size, double total_time, double throughput) {
    int write_header = 0;

    // すでにヘッダがあるかチェック
    FILE *fcheck = fopen(filename, "r");
    if (!fcheck) {
        write_header = 1; // ファイルがない場合はヘッダを書き込む
    } else {
        char buf[128];
        if (!fgets(buf, sizeof(buf), fcheck)) {
            write_header = 1; // 空ファイルならヘッダを書き込む
        }
        fclose(fcheck);
    }

    FILE *f = fopen(filename, write_header ? "w" : "a");
    if (!f) {
        fprintf(stderr, "Cannot open CSV file: %s\n", filename);
        return;
    }

    if (write_header) {
        fprintf(f, "DataSize(Bytes),iterations,TotalTime(sec),Average(sec),Throughput(MiBit/s)\n");
    }
    fprintf(f, "%zu,%d,%.8f,%.3f,%.3f\n", data_size, ITERATIONS, total_time, total_time / ITERATIONS, throughput);
    fclose(f);
}
// --- tiny aes 計測 ---
static double measure_speed_tiny(const char *dll_path, size_t data_size) {
    HMODULE h = LoadLibraryA(dll_path);
    if (!h) {
        fprintf(stderr, "LoadLibrary failed: %s (error=%lu)\n", dll_path, GetLastError());
        return -1.0;
    }

    AES_INIT  aes_init  = (AES_INIT)GetProcAddress(h, "AES_init_ctx_iv");
    AES_XCRYPT aes_xcrypt = (AES_XCRYPT)GetProcAddress(h, "AES_CTR_xcrypt_buffer");
    if (!aes_init || !aes_xcrypt) {
        fprintf(stderr, "GetProcAddress failed in %s\n", dll_path);
        FreeLibrary(h);
        return -1.0;
    }

    AES_ctx_tiny ctx;
    uint8_t key[32] = {0};
    uint8_t iv[16]  = {0};

    // VirtualAlloc: 予約+コミット
    uint8_t *data = (uint8_t*)VirtualAlloc(NULL, data_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!data) {
        fprintf(stderr, "VirtualAlloc failed (error=%lu)\n", GetLastError());
        FreeLibrary(h);
        return -1.0;
    }

    // 事前に全ページへアクセスしてページフォールトを排除
    touch_pages(data, data_size);

    printf("  sizeof(data): %zu bytes\n", data_size);

    double start = get_time_sec();
    for (int i = 0; i < ITERATIONS; i++) {
        aes_init(&ctx, key, iv);
        aes_xcrypt(&ctx, data, data_size);
    }
    double end = get_time_sec();

    double total_time = end - start;
    double mib_per_sec = (data_size  * 8 * (double)ITERATIONS) / (1024.0 * 1024.0) / total_time;

    printf("    Total time : %.8f sec\n", total_time);
    printf("    Average    : %.8f sec\n", total_time / ITERATIONS);
    printf("    Throughput : %.3f MiBit/s\n", mib_per_sec);
    write_csv("tiny_aes.csv", data_size, total_time, mib_per_sec);

    VirtualFree(data, 0, MEM_RELEASE);
    FreeLibrary(h);
    return total_time;
}

// --- t-table aes 計測 ---
static double measure_speed_ttable(const char *dll_path, size_t data_size) {
    HMODULE h = LoadLibraryA(dll_path);
    if (!h) {
        fprintf(stderr, "LoadLibrary failed: %s (error=%lu)\n", dll_path, GetLastError());
        return -1.0;
    }

    AES_INIT_1  aes_init  = (AES_INIT_1)GetProcAddress(h, "AES_init_ctx_iv");
    AES_XCRYPT_1 aes_xcrypt = (AES_XCRYPT_1)GetProcAddress(h, "AES_CTR_xcrypt_buffer");
    if (!aes_init || !aes_xcrypt) {
        fprintf(stderr, "GetProcAddress failed in %s\n", dll_path);
        FreeLibrary(h);
        return -1.0;
    }

    AES_ctx_ttable ctx;
    uint8_t key[32] = {0};
    uint8_t iv[16]  = {0};

    uint8_t *data = (uint8_t*)VirtualAlloc(NULL, data_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!data) {
        fprintf(stderr, "VirtualAlloc failed (error=%lu)\n", GetLastError());
        FreeLibrary(h);
        return -1.0;
    }
    touch_pages(data, data_size);

    printf("  sizeof(data): %zu bytes\n", data_size);

    double start = get_time_sec();
    for (int i = 0; i < ITERATIONS; i++) {
        aes_init(&ctx, key, iv);
        aes_xcrypt(&ctx, data, data_size);
    }
    double end = get_time_sec();

    double total_time = end - start;
    double mib_per_sec = (data_size  * 8 * (double)ITERATIONS) / (1024.0 * 1024.0) / total_time;

    printf("    Total time : %.8f sec\n", total_time);
    printf("    Average    : %.8f sec\n", total_time / ITERATIONS);
    printf("    Throughput : %.3f MiBit/s\n", mib_per_sec);

    write_csv("Ttable_aes.csv", data_size, total_time, mib_per_sec);

    VirtualFree(data, 0, MEM_RELEASE);
    FreeLibrary(h);
    return total_time;
}


int main(void) {
    const size_t sizes[] = { 1024ULL, 1024ULL*1024ULL, 10ULL*1024ULL*1024ULL }; // 1KB, 1MB, 10MB

    printf("TINY AES Benchmark Results:-----------------------------------\n");
    for (int i = 0; i < (int)(sizeof(sizes)/sizeof(sizes[0])); i++) {
        // (void)measure_speed_tiny("../tiny_aes_dll/tiny_aes.dll", sizes[i]);
        (void)measure_speed_tiny("libtiny_aes.dll", sizes[i]);
    }

    printf("TTABLE AES Benchmark Results:---------------------------------\n");
    for (int i = 0; i < (int)(sizeof(sizes)/sizeof(sizes[0])); i++) {
        // (void)measure_speed_ttable("../ttable_aes_dll/ttable_aes_t.dll", sizes[i]);
        (void)measure_speed_ttable("libttable_aes_t.dll", sizes[i]);
    }


    return 0;
}
