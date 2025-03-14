#ifndef DYNAMIC_MENU_H
#define DYNAMIC_MENU_H

#include "algorithm_config.h"

// 알고리즘 선택 메뉴 출력
void print_algorithm_selection_menu(void);

// 특정 알고리즘의 테스트 유형 메뉴 출력
void print_test_type_menu(int algo_id);

// 특정 알고리즘의 입력 유형 메뉴 출력
void print_input_type_menu(int algo_id);

// 자동 테스트 수행 함수
int run_auto_test(int algo_id, int test_type_id);

// 수동 테스트 수행 함수
int run_manual_test(int algo_id, int input_type_id);

// 알고리즘 유형별 테스트 함수 (신규 추가)
int run_hash_algorithm_test(AlgorithmConfig* algo, int test_type_id);
int run_cipher_algorithm_test(AlgorithmConfig* algo, int test_type_id);
int run_signature_algorithm_test(AlgorithmConfig* algo, int test_type_id);

#endif /* DYNAMIC_MENU_H */