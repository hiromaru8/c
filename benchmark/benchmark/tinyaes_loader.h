#ifndef TINYAES_LOADER_H
#define TINYAES_LOADER_H

#include <stddef.h>
#include <stdint.h>


/**
 * @brief TinyAES ハンドル型
 *
 * @details
 * TinyAES DLL のモジュールハンドルおよび取得済み API 関数ポインタを
 * 内部に保持する不透明型（opaque type）。
 *
 * 呼び出し側は構造体の内容を直接参照せず、
 * tinyaes_open() および tinyaes_close() を使用して管理する。
 */
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
 * @brief TinyAES 暗号コンテキスト
 *
 * @details
 * TinyAES ライブラリが使用する AES コンテキスト。
 *
 * DLL 側で定義されている AES_ctx 構造体とのバイナリ互換性を
 * 維持するため、同一レイアウトで定義している。
 *
 * @note
 * 構造体メンバは TinyAES DLL の仕様に依存するため、
 * 変更する場合は DLL 側との互換性を確認すること。
 */
typedef struct AES_ctx_tiny {
    uint8_t RoundKey[240];
    uint8_t Iv[16];
} AES_ctx_tiny;

/**
 * @brief AES-CTR 暗号化用コンテキストを初期化する。
 *
 * @details
 * 指定された鍵と IV を使用して TinyAES の暗号化コンテキストを
 * 初期化する。
 *
 * @param[in] aes
 * TinyAES ハンドル。
 *
 * @param[out] ctx
 * 初期化対象の AES コンテキスト。
 *
 * @param[in] key
 * AES 鍵。
 *
 * @param[in] iv
 * 初期化ベクトル。
 *
 * @note
 * key および iv のサイズは TinyAES DLL の仕様に従うこと。
 */
void tinyaes_init(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    const uint8_t *key,
    const uint8_t *iv);

/**
 * @brief AES-CTR 暗号化／復号を実行する。
 *
 * @details
 * TinyAES の AES_CTR_xcrypt_buffer() を呼び出し、
 * 指定されたバッファを暗号化または復号する。
 *
 * AES-CTR モードでは暗号化と復号は同一処理である。
 *
 * @param[in] aes
 * TinyAES ハンドル。
 *
 * @param[in,out] ctx
 * AES コンテキスト。
 *
 * @param[in,out] buf
 * 暗号化または復号対象のデータバッファ。
 *
 * @param[in] length
 * 処理するデータサイズ（byte）。
 *
 * @note
 * 処理後、buf の内容は暗号文または復号文に置換される。
 */
void tinyaes_xcrypt(
    TinyAES *aes,
    AES_ctx_tiny *ctx,
    uint8_t *buf,
    size_t length);


#endif