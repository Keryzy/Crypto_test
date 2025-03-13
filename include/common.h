#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <errno.h>

#include "error.h"

// 기본 상수
#define MAX_LINE_LENGTH 16384
#define MAX_MESSAGE_LENGTH 65536
#define MAX_FILENAME_LENGTH 256
#define SHA256_DIGEST_LENGTH 32
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX_PATH_LENGTH 1024

// 유틸리티 함수
int hex_to_bytes(const char* hex_str, unsigned char* bytes, size_t max_len);
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str, size_t hex_len);

// 파일 관련 함수
bool file_exists(const char* filename);
int create_directory(const char* path);
int compare_test_results(const char* output_file, const char* expected_file);
const char* get_filename_from_path(const char* path);
int extract_md_values(const char* filename, char md_values[][MAX_LINE_LENGTH], int max_values);

#endif // COMMON_H