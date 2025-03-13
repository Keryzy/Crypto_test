#include "../include/common.h"
#include "../include/sha256.h"
#include <windows.h>

void print_usage(const char* program_name) {
    printf("사용법: %s <입력 파일> <출력 파일>\n", program_name);
    printf("입력 파일: SHA-256 테스트 벡터 파일 (.req 또는 .rsp)\n");
    printf("출력 파일: 결과를 저장할 파일\n");
}

int main(int argc, char* argv[]) {
    // 버퍼링 없이 출력하도록 설정
    setvbuf(stdout, NULL, _IONBF, 0);
    
    // Windows 콘솔 한글 출력을 위한 설정
    SetConsoleOutputCP(CP_UTF8);
    
    // OpenSSL 초기화
    OpenSSL_add_all_algorithms();
    
    // 인자 확인
    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];
    
    printf("\n========= SHA-256 테스트 프로그램 =========\n");
    printf("입력 파일: %s\n", input_file);
    printf("출력 파일: %s\n", output_file);
    
    // 테스트 유형 감지 및 출력
    TestType test_type = sha256_detect_test_type(input_file);
    const char* type_str;
    
    switch (test_type) {
        case TEST_SHORT_MSG:
            type_str = "짧은 메시지";
            break;
        case TEST_LONG_MSG:
            type_str = "긴 메시지";
            break;
        case TEST_MONTE_CARLO:
            type_str = "몬테 카를로";
            break;
        default:
            type_str = "알 수 없음";
            break;
    }
    
    printf("감지된 테스트 유형: %s\n", type_str);
    printf("=========================================\n\n");
    
    int result;
    
    // 출력 파일의 디렉토리 생성 (파일명 제외한 경로만 추출)
    char output_dir[MAX_PATH_LENGTH] = {0};
    strcpy(output_dir, output_file);
    char* last_slash = strrchr(output_dir, '/');
    if (last_slash) {
        *last_slash = '\0';
        create_directory(output_dir);
    } else {
        last_slash = strrchr(output_dir, '\\');
        if (last_slash) {
            *last_slash = '\0';
            create_directory(output_dir);
        }
    }
    
    // 테스트 유형에 따라 적절한 함수 호출
    if (test_type == TEST_MONTE_CARLO) {
        result = sha256_monte(input_file, output_file);
    } else {
        result = sha256_test(input_file, output_file);
    }
    
    // 결과 출력
    if (result == SUCCESS) {
        printf("\n✓ 테스트 성공!\n");
    } else {
        printf("\n! 오류 발생: %d\n", result);
    }
    
    // OpenSSL 정리
    EVP_cleanup();
    
    return (result == SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}