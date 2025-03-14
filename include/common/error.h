#ifndef ERROR_H
#define ERROR_H

#include <stdbool.h>

// 성공 코드
#define SUCCESS 0

// 시스템 관련 에러 코드 (100번대)
#define ERR_SYSTEM_DIR          100
#define ERR_SYSTEM_PERMISSION   101
#define ERR_SYSTEM_CONSOLE      102
#define ERR_SYSTEM_MEMORY       103

// 파일 관련 에러 코드 (110번대)
#define ERR_FILE_OPEN           110    // 파일 열기 실패
#define ERR_FILE_READ           111    // 파일 읽기 실패
#define ERR_FILE_WRITE          112    // 파일 쓰기 실패
#define ERR_FILE_NOT_FOUND      113    // 파일을 찾을 수 없음
#define ERR_FILE_CREATE         114    // 파일 생성 실패

// 입력/형식 관련 에러 코드 (120번대)
#define ERR_INVALID_FORMAT      120    // 잘못된 형식
#define ERR_INVALID_INPUT       121    // 유효하지 않은 입력
#define ERR_MEMORY_ALLOC        122    // 메모리 할당 실패

// HEX 변환 관련 에러 코드 (130번대)
#define ERR_HEX_INVALID_CHAR    130    // 유효하지 않은 16진수 문자
#define ERR_HEX_BUFFER_OVERFLOW 131    // 16진수 버퍼 초과
#define ERR_HEX_NULL_INPUT      132    // NULL 입력 오류

// 암호화 관련 에러 코드 (140번대)
#define ERR_CRYPTO_OPERATION    140    // 암호화 연산 실패
#define ERR_CRYPTO_INIT         141    // 암호화 초기화 실패

// SHA-256 관련 에러 코드 (150번대)
#define ERR_SHA256_INIT         150    // SHA-256 초기화 실패
#define ERR_SHA256_UPDATE       151    // SHA-256 업데이트 실패
#define ERR_SHA256_FINAL        152    // SHA-256 최종 연산 실패
#define ERR_SHA256_HASH         153    // SHA-256 해시 실패

// 테스트 관련 에러 코드 (160번대)
#define ERR_TEST_FAILED         160    // 테스트 실패
#define ERR_INVALID_TEST_TYPE   161    // 유효하지 않은 테스트 유형
#define ERR_INVALID_INPUT_TYPE  162    // 유효하지 않은 입력 유형

// 알고리즘 관련 에러 코드 (170번대)
#define ERR_INVALID_ALGORITHM   170    // 유효하지 않은 알고리즘
#define ERR_UNKNOWN_ALGORITHM_TYPE 171 // 알 수 없는 알고리즘 유형
#define ERR_MAX_ALGORITHMS      172    // 최대 알고리즘 수 초과
#define ERR_NOT_IMPLEMENTED     173    // 구현되지 않은 기능

// 에러 메시지 함수
void print_error(int error_code);

// 에러 메시지 반환 함수
const char* get_error_message(int error_code);

#endif // ERROR_H
