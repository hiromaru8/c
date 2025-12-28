// aes_private.c (コンパイルしてライブラリ化、ヘッダ未公開)
#include "aes_public.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h> /* POSIX の場合のロック */
#include <unistd.h>
#include <sys/mman.h> /* mlock/munlock */

#if defined(_WIN32)
#include <windows.h>
#endif

/* 内部でのみ見える構造体定義 */
struct AES_CTX {
    size_t key_len;
    // AES ラウンドキー等。アラインしておくと良い（AES-NI 用）
    alignas(16) uint8_t round_key[240]; // 最大サイズ（AES-256 のキー展開用）
    pthread_mutex_t lock; // 必要ならロック
    // 他のフラグ等
};

/* セキュアゼロ関数（移植的に） */
static void secure_zero(void *p, size_t n) {
#if defined(HAVE_EXPLICIT_BZERO)
    explicit_bzero(p, n);
#elif defined(_WIN32)
    SecureZeroMemory(p, n);
#else
    volatile unsigned char *q = (volatile unsigned char*)p;
    while (n--) *q++ = 0;
#endif
}

/* メモリをロック（スワップ(ディスクへ書き込み)防止） */
static int secure_mlock(void *p, size_t n) {
#if defined(_WIN32)
    return VirtualLock(p, n) ? 0 : -1;
#else
    return mlock(p, n) == 0 ? 0 : -1;
#endif
}

static int secure_munlock(void *p, size_t n) {
#if defined(_WIN32)
    return VirtualUnlock(p, n) ? 0 : -1;
#else
    return munlock(p, n) == 0 ? 0 : -1;
#endif
}

/* ライブラリ初期化（必要な場合） */
int aes_library_init(void) {
    
    // CPU feature check (AES-NI) などをここでやる

    return 0;
}
void aes_library_cleanup(void) {
    // 特に何もしない
    
}

/* 鍵からコンテキストを作る例（鍵は呼び出し側で管理） */
AES_CTX* aes_create_from_key(const uint8_t *key, size_t key_len) {
    if (!(key && (key_len==16 || key_len==24 || key_len==32))) return NULL;

    AES_CTX *ctx = (AES_CTX*)malloc(sizeof(AES_CTX));
    if (!ctx) return NULL;
    memset(ctx, 0, sizeof(*ctx));
    ctx->key_len = key_len;
    pthread_mutex_init(&ctx->lock, NULL);

    /* ロックしてスワップさせない（失敗しても致命的ではないが推奨） */
    if (secure_mlock(ctx, sizeof(*ctx)) != 0) {
        /* ロック失敗時はログ等。環境によっては権限で失敗する */
    }

    /* ここで鍵展開（ラウンドキー生成）を行う。鍵バイト列から round_key を作る。
       実装は AES のキー展開アルゴリズム。鍵バッファはゼロ化して復元されないようにする */
    aes_key_expand(key, key_len, ctx->round_key);

    /* 呼び出し側の key を直ちに消去（呼び出し側がやるべきだが念のため） -- 注意：呼び出し側管理が好ましい */
    // secure_zero((void*)key, key_len); // ※呼び出し側ライフサイクルによる。ここで消すと呼び出し側が困ることがある

    return ctx;
}

void aes_destroy(AES_CTX *ctx) {
    if (!ctx) return;
    pthread_mutex_lock(&ctx->lock);

    /* round_key 等をゼロ化してからメモリアンロック/解放 */
    secure_zero(ctx->round_key, sizeof(ctx->round_key));
    secure_munlock(ctx, sizeof(*ctx));

    pthread_mutex_unlock(&ctx->lock);
    pthread_mutex_destroy(&ctx->lock);

    /* そして free */
    secure_zero(ctx, sizeof(*ctx)); // 最後の念押し
    free(ctx);
}

/* encrypt/decrypt のサンプル（詳細な実装は省略） */
int aes_encrypt_cbc(AES_CTX *ctx, const uint8_t *iv,
                    const uint8_t *plaintext, uint8_t *ciphertext, size_t len) {
    if (!ctx || (len % 16) != 0) return -1;
    pthread_mutex_lock(&ctx->lock);
    // IV コピー、ブロック単位で暗号化（AESブロック関数を使う）
    // AESブロック暗号化関数は ctx->round_key を使う（秘密保持）
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

int aes_decrypt_cbc(AES_CTX *ctx, const uint8_t *iv,
                    const uint8_t *ciphertext, uint8_t *plaintext, size_t len) {
    if (!ctx || (len % 16) != 0) return -1;
    pthread_mutex_lock(&ctx->lock);
    // 復号処理
    pthread_mutex_unlock(&ctx->lock);
    return 0;
}

/* 実際の AES ラウンド/キー展開の実装はここに入れる（省略） */

