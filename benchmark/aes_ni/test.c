#include <wmmintrin.h>
#include <emmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define AES256_ROUNDS 14

// ---------- AES-256 key expansion ----------
void aes256_key_expansion(const __m128i key[2], __m128i round_keys[AES256_ROUNDS+1]) {
    __m128i temp1, temp2;
    int i;

    round_keys[0] = key[0];
    round_keys[1] = key[1];

    temp1 = key[0];
    temp2 = key[1];

    for (i = 0; i < 7; i++) {
        __m128i t;

        switch(i) {
            case 0: t = _mm_aeskeygenassist_si128(temp2, 0x01); break;
            case 1: t = _mm_aeskeygenassist_si128(temp2, 0x02); break;
            case 2: t = _mm_aeskeygenassist_si128(temp2, 0x04); break;
            case 3: t = _mm_aeskeygenassist_si128(temp2, 0x08); break;
            case 4: t = _mm_aeskeygenassist_si128(temp2, 0x10); break;
            case 5: t = _mm_aeskeygenassist_si128(temp2, 0x20); break;
            case 6: t = _mm_aeskeygenassist_si128(temp2, 0x40); break;
        }

        temp1 = _mm_xor_si128(temp1, _mm_slli_si128(temp1, 4));
        temp1 = _mm_xor_si128(temp1, _mm_slli_si128(temp1, 4));
        temp1 = _mm_xor_si128(temp1, _mm_slli_si128(temp1, 4));
        temp1 = _mm_xor_si128(temp1, _mm_shuffle_epi32(t, 0xff));

        round_keys[2*i+2] = temp1;
        temp2 = _mm_xor_si128(temp1, _mm_aeskeygenassist_si128(temp1, 0x00));
        round_keys[2*i+3] = temp2;
    }
}

// ---------- AES-256 block encryption ----------
__m128i aes256_encrypt_block(__m128i plaintext, __m128i round_keys[AES256_ROUNDS+1]) {
    int i;
    __m128i m = plaintext;
    for(i=0;i<AES256_ROUNDS-1;i++) {
        m = _mm_aesenc_si128(m, round_keys[i]);
    }
    m = _mm_aesenclast_si128(m, round_keys[AES256_ROUNDS-1]);
    return m;
}

// ---------- AES-256 CTR mode ----------
void aes256_ctr_encrypt(uint8_t *dst, const uint8_t *src, size_t len,
                        __m128i round_keys[AES256_ROUNDS+1], uint64_t nonce) {
    uint64_t counter = 0;
    for(size_t i=0;i<len;i+=16) {
        __m128i block = _mm_set_epi64x(nonce, counter++);
        __m128i keystream = aes256_encrypt_block(block, round_keys);

        size_t block_size = (i+16 <= len) ? 16 : (len-i);
        uint8_t buf[16];
        _mm_storeu_si128((__m128i*)buf, keystream);

        for(size_t j=0;j<block_size;j++) {
            dst[i+j] = src[i+j] ^ buf[j];
        }
    }
}

// ---------- 高精度タイマー付きベンチ ----------
double measure_throughput(uint8_t *plain, uint8_t *cipher, size_t size,
                          __m128i round_keys[AES256_ROUNDS+1], uint64_t nonce) {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    aes256_ctr_encrypt(cipher, plain, size, round_keys, nonce);

    QueryPerformanceCounter(&end);
    double seconds = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    return (size*8 / (1024.0*1024.0)) / seconds; // MiB/s
}

// ---------- メイン ----------
int main() {
    const size_t sizes[] = { 1024ULL, 1024ULL*1024ULL, 10ULL*1024ULL*1024ULL }; // 1KB, 1MB, 10MB
    uint8_t *plain, *cipher;
    __m128i key[2] = {_mm_set_epi32(0,1,2,3), _mm_set_epi32(4,5,6,7)};
    __m128i round_keys[AES256_ROUNDS+1];

    aes256_key_expansion(key, round_keys);

    for(int idx=0; idx<3; idx++) {
        size_t size = sizes[idx];
        plain = malloc(size);
        cipher = malloc(size);
        memset(plain, 0x55, size);

        double mib_per_sec = measure_throughput(plain, cipher, size, round_keys, 0x12345678abcdef00ULL);

        printf("Size: %zu bytes, Throughput: %.2f MiBit/s\n", size, mib_per_sec);

        free(plain);
        free(cipher);
    }

    return 0;
}
