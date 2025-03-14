#include "../../include/common/algorithm_registry.h"
#include "../../include/algorithm/sha256.h"
#include <string.h>

// SHA-256 알고리즘 등록 함수
void register_sha256_algorithm(void) {
    REGISTER_HASH_ALGO(
        "SHA-256",                    // 알고리즘 이름
        "보안 해시 알고리즘 256비트", // 설명
        sha256_hash,                  // 해시 함수
        sha256_init,                  // 초기화 함수
        sha256_update,                // 업데이트 함수
        sha256_final,                 // 마무리 함수
        "./test_vectors/SHA_256",     // 테스트 벡터 경로
        "ShortMsg",                   // 테스트 유형들...
        "LongMsg",
        "Monte"
    );
} 