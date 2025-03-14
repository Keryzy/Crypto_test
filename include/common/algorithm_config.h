#ifndef ALGORITHM_CONFIG_H
#define ALGORITHM_CONFIG_H

#include <stddef.h>
#include "common.h"

// 최대 길이 상수
#define MAX_ALGO_NAME 50
#define MAX_ALGO_DESC 200
#define MAX_TEST_TYPES 10
#define MAX_TYPE_NAME 50
#define MAX_INPUT_TYPES 5
#define MAX_ALGORITHMS 10


// 알고리즘 유형 열거형
typedef enum {
    ALGO_TYPE_HASH,      // 해시 알고리즘
    ALGO_TYPE_CIPHER,    // 암호화 알고리즘
    ALGO_TYPE_SIGNATURE  // 서명 알고리즘
} AlgorithmType;

// 알고리즘 설정 구조체
typedef struct {
    char name[MAX_ALGO_NAME];                 // 알고리즘 이름 (예: "SHA-256")
    char description[MAX_ALGO_DESC];          // 알고리즘 설명
    AlgorithmType type;                       // 알고리즘 유형
    
    // 테스트 유형 관련
    char test_types[MAX_TEST_TYPES][MAX_TYPE_NAME];  // 지원하는 테스트 유형 배열
    int test_type_count;                             // 지원하는 테스트 유형 수
    
    // 입력 유형 관련
    char input_types[MAX_INPUT_TYPES][MAX_TYPE_NAME]; // 지원하는 입력 유형 배열
    int input_type_count;                             // 지원하는 입력 유형 수
    
    // 알고리즘 함수 포인터 (유형별)
    union {
        // 해시 알고리즘용
        struct {
            int (*hash_function)(const unsigned char* input, size_t input_len, unsigned char* output);
            int (*hash_init)(void** ctx);
            int (*hash_update)(void* ctx, const unsigned char* input, size_t input_len);
            int (*hash_final)(void* ctx, unsigned char* output);
        } hash;
        
        // 암호화 알고리즘용
        struct {
            int (*encrypt)(const unsigned char* input, size_t input_len, 
                          const unsigned char* key, unsigned char* output);
            int (*decrypt)(const unsigned char* input, size_t input_len, 
                          const unsigned char* key, unsigned char* output);
        } cipher;
        
        // 서명 알고리즘용
        struct {
            int (*sign)(const unsigned char* input, size_t input_len, 
                       const unsigned char* key, unsigned char* signature);
            int (*verify)(const unsigned char* input, size_t input_len, 
                         const unsigned char* key, const unsigned char* signature);
        } signature;
    } ops;
    
    // 테스트 벡터 파일 경로
    char test_vector_path[MAX_PATH_LENGTH];
} AlgorithmConfig;

#endif /* ALGORITHM_CONFIG_H */ 