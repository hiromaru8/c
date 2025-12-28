#ifndef AES_256CTR
#define AES_256CTR

#include <stdint.h>
#include <stddef.h>

#ifdef _WIN32
  #ifdef TINYAES_EXPORTS
    #define TINYAES_API __declspec(dllexport)
  #else
    #define TINYAES_API __declspec(dllimport)
  #endif
#else
  #define TINYAES_API
#endif

#define AES_BLOCKLEN 16

typedef struct {
    uint8_t RoundKey[240]; // AES-256用最大サイズ
    uint8_t Iv[AES_BLOCKLEN];
} AES_ctx;

#ifdef __cplusplus
extern "C" {
#endif

// 初期化（AES256-CTR）
TINYAES_API void AES_init_ctx_iv(AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);

// CTRモードで暗号化/復号（同関数でOK）
TINYAES_API void AES_CTR_xcrypt_buffer(AES_ctx* ctx, uint8_t* buf, size_t length);


#ifdef __cplusplus
    }
#endif

#endif
