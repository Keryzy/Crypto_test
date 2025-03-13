#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

// 성공 코드
#define SUCCESS 0

// 기본 에러 코드
#define ERR_FILE_OPEN           100
#define ERR_FILE_READ           101
#define ERR_FILE_WRITE          102
#define ERR_FILE_NOT_FOUND      103
#define ERR_INVALID_FORMAT      104
#define ERR_CRYPTO_OPERATION    105
#define ERR_INVALID_INPUT       106
#define ERR_TEST_FAILED         107
#define ERR_FILE_OPEN_FAILED    108
#define ERR_FILE_CREATE_FAILED  109
#define ERR_MEMORY_ALLOCATION_FAILED 110

// OpenSSL SHA-256 관련 에러 코드
#define ERR_SHA256_INIT_FAILED      301
#define ERR_SHA256_UPDATE_FAILED    302
#define ERR_SHA256_FINAL_FAILED     303
#define ERR_SHA256_HASH_FAILED      304

// 에러 메시지 함수
void print_error(int error_code);

// 에러 메시지 반환 함수
const char* get_error_message(int error_code);

#endif // ERROR_H
