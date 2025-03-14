#include "../../include/common/dynamic_menu.h"
#include "../../include/common/algorithm_registry.h"
#include "../../include/common/common.h"
#include "../../include/common/error.h"
#include "../../include/algorithm/sha256.h"
#include <stdio.h>
#include <string.h>

// 알고리즘 선택 메뉴 출력
void print_algorithm_selection_menu(void) {
    printf("\n┌────────────────────────────────────┐\n");
    printf("│         알고리즘 선택 메뉴         │\n");
    printf("├────────────────────────────────────┤\n");
    
    for (int i = 0; i < get_algorithm_count(); i++) {
        AlgorithmConfig* algo = get_algorithm_by_id(i);
        printf("│    %d. %-27s │\n", i+1, algo->name);
    }
    
    printf("│    0. 이전 메뉴                   │\n");
    printf("└────────────────────────────────────┘\n");
}

// 특정 알고리즘의 테스트 유형 메뉴 출력
void print_test_type_menu(int algo_id) {
    AlgorithmConfig* algo = get_algorithm_by_id(algo_id);
    if (!algo) return;
    
    printf("\n┌────────────────────────────────────┐\n");
    printf("│   %s - 테스트 유형 선택    │\n", algo->name);
    printf("├────────────────────────────────────┤\n");
    
    for (int i = 0; i < algo->test_type_count; i++) {
        printf("│    %d. %-27s │\n", i+1, algo->test_types[i]);
    }
    
    printf("│    0. 이전 메뉴                   │\n");
    printf("└────────────────────────────────────┘\n");
}

// 특정 알고리즘의 입력 유형 메뉴 출력
void print_input_type_menu(int algo_id) {
    AlgorithmConfig* algo = get_algorithm_by_id(algo_id);
    if (!algo) return;
    
    printf("\n┌────────────────────────────────────┐\n");
    printf("│   %s - 입력 유형 선택    │\n", algo->name);
    printf("├────────────────────────────────────┤\n");
    
    for (int i = 0; i < algo->input_type_count; i++) {
        printf("│    %d. %-27s │\n", i+1, algo->input_types[i]);
    }
    
    printf("│    0. 이전 메뉴                   │\n");
    printf("└────────────────────────────────────┘\n");
}

// 자동 테스트 수행 함수
int run_auto_test(int algo_id, int test_type_id) {
    AlgorithmConfig* algo = get_algorithm_by_id(algo_id);
    if (!algo) return ERR_INVALID_ALGORITHM;
    
    printf("\n%s 알고리즘 - %s 테스트 시작...\n", 
           algo->name, algo->test_types[test_type_id]);
    
    // 알고리즘별 적절한 함수 호출
    if (strcmp(algo->name, "SHA-256") == 0) {
        return sha256_run_auto_test(test_type_id);
    }
    
    return ERR_NOT_IMPLEMENTED;
}

// 수동 테스트 수행 함수
int run_manual_test(int algo_id, int input_type_id) {
    AlgorithmConfig* algo = get_algorithm_by_id(algo_id);
    if (!algo) return ERR_INVALID_ALGORITHM;
    
    printf("\n%s 알고리즘 - %s 입력 방식으로 테스트 시작...\n", 
           algo->name, algo->input_types[input_type_id]);
    
    // 알고리즘별 적절한 함수 호출
    if (strcmp(algo->name, "SHA-256") == 0) {
        return sha256_run_manual_test(input_type_id);
    }
    
    return ERR_NOT_IMPLEMENTED;
} 