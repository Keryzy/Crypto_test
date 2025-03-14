#ifndef ALGORITHM_REGISTRY_H
#define ALGORITHM_REGISTRY_H

#include "algorithm_config.h"

// 알고리즘 등록 함수
int register_algorithm(const AlgorithmConfig* config);

// 알고리즘 가져오기 함수
AlgorithmConfig* get_algorithm_by_id(int id);
AlgorithmConfig* get_algorithm_by_name(const char* name);

// 알고리즘 수 가져오기
int get_algorithm_count(void);

// 모든 알고리즘 등록 함수 (각 알고리즘 모듈에서 호출됨)
void register_all_algorithms(void);

// SHA-256 알고리즘 등록 함수 (외부 선언)
void register_sha256_algorithm(void);

// 알고리즘 등록용 편의 매크로
#define REGISTER_HASH_ALGO(NAME, DESC, HASH_FUNC, HASH_INIT, HASH_UPDATE, HASH_FINAL, VECTOR_PATH, ...) do { \
    static const char* _test_types[] = {__VA_ARGS__}; \
    static const char* _input_types[] = {"텍스트", "16진수"}; \
    AlgorithmConfig config = { \
        .type = ALGO_TYPE_HASH, \
        .test_type_count = sizeof(_test_types)/sizeof(_test_types[0]), \
        .input_type_count = 2 \
    }; \
    strncpy(config.name, NAME, MAX_ALGO_NAME - 1); \
    strncpy(config.description, DESC, MAX_ALGO_DESC - 1); \
    strncpy(config.test_vector_path, VECTOR_PATH, MAX_PATH_LENGTH - 1); \
    for(int i = 0; i < config.test_type_count && i < MAX_TEST_TYPES; i++) \
        strncpy(config.test_types[i], _test_types[i], MAX_TYPE_NAME - 1); \
    for(int i = 0; i < config.input_type_count && i < MAX_INPUT_TYPES; i++) \
        strncpy(config.input_types[i], _input_types[i], MAX_TYPE_NAME - 1); \
    config.ops.hash.hash_function = HASH_FUNC; \
    config.ops.hash.hash_init = HASH_INIT; \
    config.ops.hash.hash_update = HASH_UPDATE; \
    config.ops.hash.hash_final = HASH_FINAL; \
    register_algorithm(&config); \
} while(0)

#endif /* ALGORITHM_REGISTRY_H */
