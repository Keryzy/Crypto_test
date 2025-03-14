#ifndef SHA256_H
#define SHA256_H

#include <openssl/evp.h>  // OpenSSL 헤더 추가
#include <stdint.h>
#include <stddef.h>
#include "../common/common.h"  // TestType 정의 가져옴

// SHA-256 다이제스트 길이
#define SHA256_DIGEST_LENGTH 32

// 메뉴 및 UI 관련 상수
#define MIN_CHOICE 0
#define MAX_CHOICE 2
#define MIN_INPUT_TYPE 1
#define MAX_INPUT_TYPE 2

// 테스트 유형 관련 상수 추가
#define MIN_AUTO_TEST_TYPE 1
#define MAX_AUTO_TEST_TYPE 3

// 테스트 유형 정의
typedef enum {
    TEST_SHORT_MSG = 0,
    TEST_LONG_MSG = 1,
    TEST_MONTE_CARLO = 2
} TestType;

// 기본 해시 함수
int sha256_hash(const uint8_t* message, size_t message_len, uint8_t* digest);

// 스트리밍 해시 함수
int sha256_init(void** ctx);
int sha256_update(void* ctx, const unsigned char* data, size_t data_len);
int sha256_final(void* ctx, unsigned char* digest);

// 자동 테스트 함수들
int sha256_run_auto_test(int test_type_id);
int sha256_test(const char* input_file, const char* output_file);
int sha256_monte(const char* input_file, const char* output_file);

// 수동 테스트 함수
int sha256_run_manual_test(int input_type_id);

// 알고리즘 등록 함수
void register_sha256_algorithm(void);

// 테스트 유형 감지 함수
TestType sha256_detect_test_type(const char* filename);

// 디버깅 정보 출력 함수
void print_debug_info(const uint8_t* msg, size_t msg_len, const uint8_t* hash);

#endif // SHA256_H