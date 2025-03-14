#include <windows.h>
#include <direct.h>
#include <conio.h>
#include "../include/common.h"
#include "../include/sha256.h"
#include "../include/error.h"

// 상수 정의
#define MIN_CHOICE 0
#define MAX_CHOICE 2
#define MIN_INPUT_TYPE 1
#define MAX_INPUT_TYPE 2
#define MAX_TEXT_INPUT 1024
#define MAX_FILE_PATH 256
#define MIN_AUTO_TEST_TYPE 1
#define MAX_AUTO_TEST_TYPE 3

// 입력 버퍼 정리 함수
static void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// 메뉴 선택 함수
static int get_menu_choice(int min, int max, const char *prompt) {
    int choice;
    printf("%s", prompt);
    
    while (1) {
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("오류: 숫자를 입력하세요.\n%s", prompt);
            continue;
        }
        clear_input_buffer();
        
        if (choice < min || choice > max) {
            print_error(ERR_INVALID_INPUT);
            printf("%s", prompt);
            continue;
        }
        break;
    }
    return choice;
}

// 작업 디렉토리 설정 함수
static int set_working_directory(void) {
    char cwd[MAX_PATH_LENGTH];
    
    // 현재 디렉토리 확인
    if (_getcwd(cwd, sizeof(cwd)) == NULL) {
        if (errno == EACCES) {
            return ERR_SYSTEM_PERMISSION;
        }
        return ERR_SYSTEM_DIR;
    }

    // bin 디렉토리 확인 및 변경
    char *last_dir = strrchr(cwd, '\\');
    if (last_dir && strcmp(last_dir, "\\bin") == 0) {
        if (_chdir("..") != 0) {
            if (errno == EACCES) {
                return ERR_SYSTEM_PERMISSION;
            }
            return ERR_SYSTEM_DIR;
        }
        
        // 변경된 디렉토리 확인
        if (_getcwd(cwd, sizeof(cwd)) == NULL) {
            return ERR_SYSTEM_DIR;
        }
        printf("작업 디렉토리 변경: %s\n\n", cwd);
    }

    return SUCCESS;
}

// 메뉴 출력 함수
static void print_main_menu(void) {
    printf("\n┌────────────────────────────────────┐\n");
    printf("│       SHA-256 테스트 프로그램      │\n");
    printf("├────────────────────────────────────┤\n");
    printf("│         1. 자동 테스트             │\n");
    printf("│         2. 수동 테스트             │\n");
    printf("│         0. 종료                    │\n");
    printf("└────────────────────────────────────┘\n");
}

static void print_input_type_menu(void) {
    printf("\n┌────────────────────────────────────┐\n");
    printf("│           입력 유형 선택           │\n");
    printf("├────────────────────────────────────┤\n");
    printf("│    1. 텍스트 직접 입력             │\n");
    printf("│    2. 16진수 문자열 입력           │\n");
    printf("└────────────────────────────────────┘\n");
}

// 테스트 유형 선택 메뉴 추가
static void print_auto_test_menu(void) {
    printf("\n┌────────────────────────────────────┐\n");
    printf("│          테스트 유형 선택          │\n");
    printf("├────────────────────────────────────┤\n");
    printf("│    1. ShortMsg (짧은 메시지)       │\n");
    printf("│    2. LongMsg (긴 메시지)          │\n");
    printf("│    3. Monte (몬테 카를로)          │\n");
    printf("└────────────────────────────────────┘\n");
}

// 진행 상태 표시 함수
static void print_progress(const char* message) {
    printf("\n[진행] %s\n", message);
}

// 수동 테스트 실행 함수
static int run_manual_test() {
    print_input_type_menu();
    int input_type = get_menu_choice(MIN_INPUT_TYPE, MAX_INPUT_TYPE, "\n선택: ");
    
    uint8_t digest[SHA256_DIGEST_LENGTH];
    int result = SUCCESS;
    
    if (input_type == 1) {  // 텍스트 직접 입력
        char text[MAX_TEXT_INPUT];
        
        printf("\n텍스트를 입력하세요 (한글, 특수문자, 공백 모두 지원, 최대 %d 문자):\n", MAX_TEXT_INPUT-1);
        if (fgets(text, sizeof(text), stdin) == NULL) {
            return ERR_INVALID_INPUT;
        }
        
        // 개행 문자 제거
        size_t len = strlen(text);
        if (len > 0 && text[len-1] == '\n') {
            text[len-1] = '\0';
            len--;
        }
        
        // 입력 정보 출력 (인코딩 확인용)
        printf("\n[입력된 텍스트] %s\n", text);
        printf("[입력 길이] %zu 바이트\n", len);
        
        // 텍스트 바이트 값 출력 (UTF-8 확인용)
        printf("[바이트 값] ");
        for (size_t i = 0; i < len && i < 32; i++) {
            printf("%02x ", (unsigned char)text[i]);
        }
        if (len > 32) printf("...");
        printf("\n");
        
        // 유니코드 문자 확인 (편의 기능)
        printf("[문자 코드] ");
        for (size_t i = 0; i < len;) {
            if ((text[i] & 0x80) == 0) {
                // ASCII
                printf("U+%04X ", (unsigned char)text[i]);
                i += 1;
            } else if ((text[i] & 0xE0) == 0xC0) {
                // 2바이트 UTF-8
                uint32_t code = ((text[i] & 0x1F) << 6) | (text[i+1] & 0x3F);
                printf("U+%04X ", code);
                i += 2;
            } else if ((text[i] & 0xF0) == 0xE0) {
                // 3바이트 UTF-8 (한글 포함)
                uint32_t code = ((text[i] & 0x0F) << 12) | 
                                ((text[i+1] & 0x3F) << 6) | 
                                (text[i+2] & 0x3F);
                printf("U+%04X ", code);
                i += 3;
            } else {
                // 4바이트 이상 UTF-8
                printf("?? ");
                i += 1;
            }
            
            if (i > 10) {
                printf("...");
                break;
            }
        }
        printf("\n");
        
        // 해시 계산
        result = sha256_hash((const uint8_t*)text, len, digest);
    } else {  // 16진수 문자열 입력
        char hex_string[MAX_TEXT_INPUT];
        
        printf("\n16진수 문자열을 입력하세요 (예: 68656c6c6f = 'hello'):\n");
        if (fgets(hex_string, sizeof(hex_string), stdin) == NULL) {
            return ERR_INVALID_INPUT;
        }
        
        // 개행 문자 제거
        size_t len = strlen(hex_string);
        if (len > 0 && hex_string[len-1] == '\n') {
            hex_string[len-1] = '\0';
        }
        
        // 공백 제거 (사용자 편의성 향상)
        char* src = hex_string;
        char* dst = hex_string;
        while (*src) {
            if (!isspace((unsigned char)*src)) {
                *dst++ = *src;
            }
            src++;
        }
        *dst = '\0';
        
        printf("\n[16진수 문자열] %s\n", hex_string);
        printf("[16진수 문자열 길이] %zu 문자\n", strlen(hex_string));
        
        // 16진수 문자열을 바이트로 변환 후 해시
        size_t byte_len = (strlen(hex_string) + 1) / 2; // 홀수 길이도 처리
        uint8_t* message = (uint8_t*)malloc(byte_len);
        
        if (!message) {
            return ERR_MEMORY_ALLOC;
        }
        
        int bytes_converted = hex_to_bytes(hex_string, message, byte_len);
        
        if (bytes_converted < 0) {
            free(message);
            return ERR_INVALID_FORMAT;
        }
        
        printf("[변환된 바이트 수] %d 바이트\n", bytes_converted);
        
        // 실제 변환된 바이트 수로 해시 계산
        result = sha256_hash(message, bytes_converted, digest);
        free(message);
    }
    
    if (result != SUCCESS) {
        return result;
    }
    
    // 해시 결과 출력
    printf("\n[SHA-256 해시 결과]\n");
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", digest[i]);
    }
    printf("\n");
    
    return SUCCESS;
}

// 자동 테스트 실행 함수
static int run_auto_test() {
    print_progress("자동 테스트를 실행합니다...");
    
    // 테스트 유형 선택
    print_auto_test_menu();
    int test_type = get_menu_choice(MIN_AUTO_TEST_TYPE, MAX_AUTO_TEST_TYPE, "\n선택: ");
    
    // 디렉토리 설정
    const char* req_dir = "test_vectors/request/SHA_256";
    const char* rsp_dir = "test_vectors/response/SHA_256";
    const char* exp_dir = "test_vectors/expected/SHA_256";  // 예상 결과 디렉토리
    
    // 테스트 파일 선택
    const char* test_file;
    switch (test_type) {
        case 1:
            test_file = "SHA256ShortMsg.req";
            printf("\n[ ShortMsg 테스트 실행 ]\n");
            break;
        case 2:
            test_file = "SHA256LongMsg.req";
            printf("\n[ LongMsg 테스트 실행 ]\n");
            break;
        case 3:
            test_file = "SHA256Monte.req";
            printf("\n[ Monte 테스트 실행 ]\n");
            break;
        default:
            return ERR_INVALID_INPUT;
    }
    
    // 출력 디렉토리 생성
    create_directory(rsp_dir);
    
    // 파일 경로 설정
    char input_path[MAX_PATH_LENGTH];
    char output_path[MAX_PATH_LENGTH];
    char expected_path[MAX_PATH_LENGTH];
    
    snprintf(input_path, MAX_PATH_LENGTH, "%s/%s", req_dir, test_file);
    
    const char* output_file = replace_extension(test_file, ".req", ".rsp");
    snprintf(output_path, MAX_PATH_LENGTH, "%s/%s", rsp_dir, output_file);
    snprintf(expected_path, MAX_PATH_LENGTH, "%s/%s", exp_dir, output_file);
    
    // 테스트 타입 감지
    TestType detected_type = sha256_detect_test_type(input_path);
    
    // 테스트 실행 (디버그 출력 없이)
    int result;
    
    // 디버그 출력 끄기
    int old_stdout = _dup(1);
    freopen("NUL", "w", stdout);
    
    if (detected_type == TEST_MONTE_CARLO) {
        result = sha256_monte(input_path, output_path);
    } else {
        result = sha256_test(input_path, output_path);
    }
    
    // 디버그 출력 복원
    fflush(stdout);
    _dup2(old_stdout, 1);
    close(old_stdout);
    
    // 실행 결과 확인
    if (result != SUCCESS) {
        print_error(result);
        return result;
    }
    
    // 예상 파일이 있는지 확인
    if (!file_exists(expected_path)) {
        printf("경고: 예상 결과 파일을 찾을 수 없습니다: %s\n", expected_path);
        printf("테스트 수행 완료: 결과 파일은 %s에 저장되었습니다.\n", output_path);
        return SUCCESS;
    }
    
    // 결과 비교
    FILE* out_file = fopen(output_path, "r");
    FILE* exp_file = fopen(expected_path, "r");
    
    if (!out_file || !exp_file) {
        if (out_file) fclose(out_file);
        if (exp_file) fclose(exp_file);
        print_error(ERR_FILE_OPEN);
        return ERR_FILE_OPEN;
    }
    
    char out_line[MAX_LINE_LENGTH];
    char exp_line[MAX_LINE_LENGTH];
    int md_count = 0;
    int mismatch_count = 0;
    char exp_md_value[MAX_LINE_LENGTH] = "";
    char out_md_value[MAX_LINE_LENGTH] = "";
    
    // 각 파일에서 라인별로 읽어가며 MD 값 비교
    while (fgets(out_line, sizeof(out_line), out_file) != NULL) {
        // 줄바꿈 문자 제거
        size_t len = strlen(out_line);
        if (len > 0 && out_line[len - 1] == '\n') {
            out_line[len - 1] = '\0';
        }
        
        // MD 값이 있는 줄만 처리
        if (strncmp(out_line, "MD = ", 5) == 0 || strncmp(out_line, "MD=", 3) == 0) {
            // MD 값 추출
            char* value_start = strchr(out_line, '=') + 1;
            // 앞쪽 공백 제거
            while (*value_start == ' ') value_start = value_start + 1;
            strcpy(out_md_value, value_start);
            
            // 정답 파일에서 다음 MD 값 찾기
            bool found_md = false;
            while (fgets(exp_line, sizeof(exp_line), exp_file) != NULL) {
                // 줄바꿈 문자 제거
                len = strlen(exp_line);
                if (len > 0 && exp_line[len - 1] == '\n') {
                    exp_line[len - 1] = '\0';
                }
                
                if (strncmp(exp_line, "MD = ", 5) == 0 || strncmp(exp_line, "MD=", 3) == 0) {
                    // MD 값 추출
                    char* exp_value_start = strchr(exp_line, '=') + 1;
                    // 앞쪽 공백 제거
                    while (*exp_value_start == ' ') exp_value_start = exp_value_start + 1;
                    strcpy(exp_md_value, exp_value_start);
                    found_md = true;
                    break;
                }
            }
            
            md_count++;
            
            // 결과 출력
            printf("\nCase %d:\n", md_count);
            printf("예상 MD: %s\n", found_md ? exp_md_value : "(없음)");
            printf("결과 MD: %s\n", out_md_value);
            
            // MD 값 비교
            if (!found_md || strcmp(out_md_value, exp_md_value) != 0) {
                printf("✕ 불일치\n");
                mismatch_count++;
            } else {
                printf("✓ 일치\n");
            }
        }
    }
    
    fclose(out_file);
    fclose(exp_file);
    
    // 최종 결과 출력
    printf("\n테스트 결과: %d개 중 %d개 일치, %d개 불일치\n", 
           md_count, md_count - mismatch_count, mismatch_count);
    
    return (mismatch_count == 0) ? SUCCESS : ERR_TEST_FAILED;
}

// main 함수
int main(void) {
    int result;
    
    // 한글 입출력을 위한 콘솔 설정 - 입출력 모두 설정
    SetConsoleCP(65001);      // 입력 코드 페이지 설정 (추가)
    SetConsoleOutputCP(65001);  // 출력 코드 페이지 설정 (기존)

    // 한글 표시를 위한 폰트 정보 출력 (디버깅용)
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    GetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    printf("현재 콘솔 폰트: %ls\n", cfi.FaceName);
    
    // 작업 디렉토리 설정
    result = set_working_directory();
    if (result != SUCCESS) {
        print_error(result);
        return result;
    }
    
    // OpenSSL 초기화
    OpenSSL_add_all_algorithms();
    
    system("cls");  // 화면 클리어
    printf("\n=== SHA-256 해시 계산 테스트 프로그램 ===\n");
    
    while (1) {
        print_main_menu();
        int choice = get_menu_choice(MIN_CHOICE, MAX_CHOICE, "\n선택: ");
        
        if (choice == 0) {
            print_progress("프로그램을 종료합니다.");
            break;
        }
        
        system("cls");  // 화면 클리어
        
        if (choice == 1) {  // 자동 테스트
            result = run_auto_test();
        } else {  // 수동 테스트
            result = run_manual_test();
        }
        
        if (result != SUCCESS) {
            print_error(result);
        }
        
        // 결과 확인을 위한 대기
        printf("\n계속하려면 아무 키나 누르세요...");
        fflush(stdout);
        while (_kbhit()) _getch();  // 입력 버퍼를 비움
        _getch();
        
        system("cls");
    }
    
    // OpenSSL 정리
    EVP_cleanup();
    
    return SUCCESS;
}