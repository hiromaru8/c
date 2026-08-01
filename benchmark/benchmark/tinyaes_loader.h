#ifndef TINYAES_LOADER_H
#define TINYAES_LOADER_H

#include <stddef.h>
#include <stdint.h>

typedef struct TinyAES TinyAES;

/**
 * @brief TinyAES DLL をロードし、利用可能な状態にする。
 *
 * @details
 * 指定された DLL をロードし、暗号化に必要な関数
 * (AES_init_ctx_iv() および AES_CTR_xcrypt_buffer())
 * のアドレスを取得する。
 *
 * DLL のロードまたは関数アドレスの取得に失敗した場合は、
 * 確保済みのリソースを解放して NULL を返す。
 *
 * 正常終了時に返されるハンドルは、
 * 使用後に必ず tinyaes_close() を呼び出して解放すること。
 *
 * @param[in] dll_path
 * ロードする TinyAES DLL のパス。
 *
 * @return
 * - 成功 : TinyAES ハンドル
 * - 失敗 : NULL
 *
 * @note
 * 本関数は Windows の LoadLibrary() および GetProcAddress()
 * を使用する。
 *
 * @sa tinyaes_close()
 */
TinyAES *tinyaes_open(const char *dll_path);


/**
 * @brief TinyAES DLL をアンロードする。
 *
 * @details
 * tinyaes_open() により取得した TinyAES ハンドルを解放し、
 * DLL をアンロードする。
 *
 * NULL が指定された場合は何も行わないため、
 * 呼び出し前に NULL チェックを行う必要はない。
 *
 * 本関数を呼び出した後は、指定した TinyAES ハンドルを
 * 使用してはならない。
 *
 * @param[in] aes
 * tinyaes_open() が返した TinyAES ハンドル。
 *
 * @sa tinyaes_open()
 */
void tinyaes_close(TinyAES *aes);

/**
 * @brief TinyAES のコンテキスト
 *
 * TinyAES ライブラリが要求するコンテキスト構造体。
 * DLL の公開 API と互換性を保つため、本モジュール内でも同じ定義を使用する。
 */
typedef struct AES_ctx_tiny {
    uint8_t RoundKey[240];
    uint8_t Iv[16];
} AES_ctx_tiny;

void tinyaes_init(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    const uint8_t *key,
    const uint8_t *iv);

void tinyaes_xcrypt(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    uint8_t *buf,
    size_t length);


#endif