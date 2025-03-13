#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
// 테스트 유형 정의
typedef enum {
    TEST_SHORT_MSG,   // 짧은 메시지 테스트
    TEST_LONG_MSG,    // 긴 메시지 테스트
    TEST_MONTE_CARLO  // 몬테 카를로 테스트
} TestType;

// SHA-256 함수 선언
int sha256_hash(const uint8_t* message, size_t message_len, uint8_t* digest);
int sha256_test(const char* input_file, const char* output_file);
int sha256_monte(const char* input_file, const char* output_file);
TestType sha256_detect_test_type(const char* filename);

// 디버깅용 함수 선언
void print_debug_info(const uint8_t* msg, size_t msg_len, const uint8_t* hash);

// 파일 스트림 기반 테스트 함수
int sha256_run_test_file(FILE* fp, FILE* out_fp);

int sha256_monte_carlo_test_file(FILE* fp, FILE* out_fp);

#endif // SHA256_H