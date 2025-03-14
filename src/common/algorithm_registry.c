#include "../../include/common/algorithm_registry.h"
#include "../../include/common/common.h"
#include "../../include/common/error.h"
#include <string.h>
#include <stdio.h>

// 등록된 알고리즘을 저장할 배열
static AlgorithmConfig g_algorithms[MAX_ALGORITHMS];
static int g_algorithm_count = 0;

// 알고리즘 등록 함수
int register_algorithm(const AlgorithmConfig* config) {
    if (g_algorithm_count >= MAX_ALGORITHMS) {
        return ERR_MAX_ALGORITHMS;
    }
    
    // 알고리즘 설정 복사
    memcpy(&g_algorithms[g_algorithm_count], config, sizeof(AlgorithmConfig));
    g_algorithm_count++;
    
    return SUCCESS;
}

// 알고리즘 ID로 가져오기
AlgorithmConfig* get_algorithm_by_id(int id) {
    if (id < 0 || id >= g_algorithm_count) {
        return NULL;
    }
    
    return &g_algorithms[id];
}

// 알고리즘 이름으로 가져오기
AlgorithmConfig* get_algorithm_by_name(const char* name) {
    for (int i = 0; i < g_algorithm_count; i++) {
        if (strcmp(g_algorithms[i].name, name) == 0) {
            return &g_algorithms[i];
        }
    }
    
    return NULL;
}

// 알고리즘 수 가져오기
int get_algorithm_count(void) {
    return g_algorithm_count;
}

// 헤더 파일에서 선언한 함수(각 알고리즘 모듈에서 구현됨)
extern void register_sha256_algorithm(void);
// extern void register_aes128_algorithm(void);
// extern void register_cmac128_algorithm(void);
// 추가 알고리즘 등록 함수들...

// 모든 알고리즘 등록
void register_all_algorithms(void) {
    // 알고리즘 모듈별 등록 함수 호출
    register_sha256_algorithm();
    // register_aes128_algorithm();
    // register_cmac128_algorithm();
    // ... 다른 알고리즘 등록
    
    printf("총 %d개 알고리즘 등록됨\n", g_algorithm_count);
} 