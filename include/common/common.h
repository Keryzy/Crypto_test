#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  // uint8_t 정의
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

// 오류 코드 (error.h로 이동할 수 있는 부분)
#define SUCCESS 0

// 상수 정의
#define MAX_PATH_LENGTH 256
#define MAX_LINE_LENGTH 1024
#define MAX_HASH_LENGTH 128

// 16진수 변환 함수
int hex_to_bytes(const char* hex_string, uint8_t* bytes, size_t max_len);
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_string);

// 파일 유틸리티 함수
bool file_exists(const char* path);
int create_directory(const char* path);  // 반환형을 int로 통일
const char* replace_extension(const char* filename, const char* old_ext, const char* new_ext);

// 테스트 결과 비교 함수
int compare_test_results(const char* output_file, const char* expected_file);

#endif // COMMON_H