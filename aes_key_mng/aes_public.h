// aes_public.h
#ifndef AES_PUBLIC_H
#define AES_PUBLIC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 不完全型（opaque）宣言 — レイアウトは公開しない */
typedef struct AES_CTX AES_CTX;

/* ライブラリ初期化/終了（必要なら） */
int aes_library_init(void);
void aes_library_cleanup(void);

/* コンテキスト作成／破棄
   key: 鍵バイト列（呼び出し側は作成後すぐに上書き/消去推奨）
   key_len: バイト長（16/24/32） */
AES_CTX* aes_create_from_key(const uint8_t *key, size_t key_len);
void aes_destroy(AES_CTX *ctx);

/* 暗号化/復号（例：AES-CBC、パディングは呼び出し側で管理するか別関数で）
   iv は 16 バイト（呼び出し側が用意）
   戻り値: 0=成功, 非0=エラー */
int aes_encrypt_cbc(AES_CTX *ctx, const uint8_t *iv,
                    const uint8_t *plaintext, uint8_t *ciphertext, size_t len);
int aes_decrypt_cbc(AES_CTX *ctx, const uint8_t *iv,
                    const uint8_t *ciphertext, uint8_t *plaintext, size_t len);

/* 双方向で使う場合は復号も同様に。鍵を返す関数は絶対に作らないでください。 */

#ifdef __cplusplus
}
#endif

#endif // AES_PUBLIC_H
