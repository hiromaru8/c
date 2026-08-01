/**
 * @file tinyaes_loader.c
 * @brief TinyAES DLL の動的ロードモジュール
 *
 * @details
 * Windows の LoadLibrary() および GetProcAddress() を利用して
 * TinyAES DLL をロードし、DLL が提供する API を取得する。
 *
 * DLL のロードおよび関数アドレス取得の詳細を本モジュールへ隠蔽することで、
 * 呼び出し側は Windows API を意識することなく TinyAES を利用できる。
 *
 * @note
 * Windows 専用実装。
 */

#include "tinyaes_loader.h"

#include <stdio.h>
#include <stdlib.h>

#include <windows.h>



/**
 * @brief AES 初期化関数の型
 *
 * TinyAES DLL が公開する AES_init_ctx_iv() の関数型。
 */
typedef void (*AES_INIT)(
    AES_ctx_tiny *,
    const uint8_t *,
    const uint8_t *);

/**
 * @brief AES-CTR 暗号化関数の型
 *
 * TinyAES DLL が公開する AES_CTR_xcrypt_buffer() の関数型。
 */
typedef void (*AES_XCRYPT)(
    AES_ctx_tiny *,
    uint8_t *,
    size_t);

/**
 * @brief TinyAES DLL ハンドル
 *
 * DLL モジュールハンドルおよび取得した関数ポインタを保持する。
 *
 * @details
 * 呼び出し側から Windows API を隠蔽するため、
 * 本構造体は本ファイル内のみで定義する。
 */
struct TinyAES
{
    HMODULE     module; // DLL モジュールハンドル
    AES_INIT    init;   // AES 初期化関数ポインタ
    AES_XCRYPT  xcrypt; // AES-CTR 暗号化関数ポインタ
};

/**
 * @brief AES 初期化関数を取得する。
 *
 * GetProcAddress() が返す FARPROC を
 * AES_INIT 型へ変換して返す。
 *
 * @param[in] module
 * TinyAES DLL のモジュールハンドル
 *
 * @return
 * 成功時は AES_INIT、
 * 失敗時は NULL。
 */
static AES_INIT load_aes_init(HMODULE module)
{
    /*
     * GetProcAddress() が返す FARPROC を
     * AES_INIT 型へ変換する。
     *
     * FARPROC を直接関数ポインタへキャストすると
     * コンパイラによっては警告が発生するため、
     * union を介して型変換を行う。
     */
    union {
        FARPROC fp;
        AES_INIT fn;
    } u;

    u.fp = GetProcAddress(module, "AES_init_ctx_iv");

    if (!u.fp) {
        fprintf(stderr,
            "GetProcAddress(AES_init_ctx_iv) failed (%lu)\n",
            GetLastError());
    }

    return u.fn;

}

/**
 * @brief AES-CTR 暗号化関数を取得する。
 *
 * GetProcAddress() が返す FARPROC を
 * AES_XCRYPT 型へ変換して返す。
 *
 * @param[in] module
 * TinyAES DLL のモジュールハンドル
 *
 * @return
 * 成功時は AES_XCRYPT、
 * 失敗時は NULL。
 */
static AES_XCRYPT load_aes_xcrypt(HMODULE module)
{
    /*
     * GetProcAddress() が返す FARPROC を
     * AES_XCRYPT 型へ変換する。
     *
     * FARPROC を直接関数ポインタへキャストすると
     * コンパイラによっては警告が発生するため、
     * union を介して型変換を行う。
     */
    union {
        FARPROC fp;
        AES_XCRYPT fn;
    } u;

    u.fp = GetProcAddress(module, "AES_CTR_xcrypt_buffer");

    if (!u.fp) {
        fprintf(stderr,
            "GetProcAddress(AES_CTR_xcrypt_buffer) failed (%lu)\n",
            GetLastError());
    }

    return u.fn;
}

/**
 * @brief TinyAES DLL をロードする。
 *
 * DLL をロードし、必要な関数アドレスを取得する。
 *
 * @param[in] dll_path
 * ロードする DLL のパス
 *
 * @return
 * 成功時は TinyAES ハンドル、
 * 失敗時は NULL。
 *
 * @note
 * エラー発生時は取得済みリソースをすべて解放する。
 */
TinyAES *tinyaes_open(const char *dll_path)
{
    TinyAES *aes = NULL;

    /**
     * TinyAES ハンドルを動的に確保する。
     * calloc() を使用することで、メンバを 0 に初期化し、エラー処理を単純化する。
     */
    aes = calloc(1, sizeof(*aes));
    if (!aes)
        return NULL;

    /* TinyAES DLL をロードする。 */
    aes->module = LoadLibraryA(dll_path);
    if (!aes->module)
        goto cleanup;

    /**
     * DLL が公開する API のアドレスを取得する。
     * 取得できない場合は、DLL のバージョン不一致や
     * DLL の破損などが考えられるため、ロード失敗として扱う。
     */
    aes->init = load_aes_init(aes->module);
    if (!aes->init)
        goto cleanup;

    aes->xcrypt = load_aes_xcrypt(aes->module);
    if (!aes->xcrypt)
        goto cleanup;

    return aes;

cleanup:
    /**
     * エラー発生時は、ここまでに確保したリソースをすべて解放する。
     * cleanup に集約することで、解放漏れを防ぎ保守性を向上させる。
     */
    if (aes) {

        if (aes->module)
            FreeLibrary(aes->module);

        free(aes);
    }

    return NULL;
}

/**
 * @brief TinyAES DLL をアンロードする。
 *
 * tinyaes_open() で取得したハンドルを解放する。
 *
 * @param[in] aes
 * TinyAES ハンドル。
 * NULL の場合は何もしない。
 */
void tinyaes_close(TinyAES *aes)
{
    if (!aes)
        return;

    if (aes->module)
        FreeLibrary(aes->module);

    free(aes);
}



void tinyaes_init(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    const uint8_t *key,
    const uint8_t *iv)
{
    aes->init(ctx, key, iv);
}

void tinyaes_xcrypt(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    uint8_t *buf,
    size_t length)
{
    aes->xcrypt(ctx, buf, length);
}
