#include <windows.h>
#include <direct.h>
#include <conio.h>
#include "../algorithm/sha256.h"
#include "../common/error.h"
#include "../common/common.h"
#include "../common/algorithm_registry.h"
#include "../common/dynamic_menu.h"

// 상수 정의
#define MIN_CHOICE 0
#define MAX_CHOICE 2
#define MAX_TEXT_INPUT 1024
#define MAX_FILE_PATH 256

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
    printf("│       암호화 알고리즘 테스트       │\n");
    printf("├────────────────────────────────────┤\n");
    printf("│         1. 자동 테스트             │\n");
    printf("│         2. 수동 테스트             │\n");
    printf("│         0. 종료                    │\n");
    printf("└────────────────────────────────────┘\n");
}

// 진행 상태 표시 함수
static void print_progress(const char* message) {
    printf("\n[진행] %s\n", message);
}

// main 함수
int main(int argc, char** argv) {
    // 미사용 매개변수 경고 제거
    (void)argc;
    (void)argv;
    
    int result = SUCCESS;
    
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
        return 1;
    }
    
    // 모든 알고리즘 등록
    register_all_algorithms();
    
    while (1) {
        print_main_menu();
        int choice = get_menu_choice(0, 2, "메뉴 선택: ");
        
        if (choice == 0) {
            printf("프로그램을 종료합니다.\n");
            break;
        }
        else if (choice == 1) {
            // 자동 테스트 모드
            print_algorithm_selection_menu();
            int algo_choice = get_menu_choice(0, get_algorithm_count(), "알고리즘 선택: ");
            
            if (algo_choice == 0) continue;
            
            print_test_type_menu(algo_choice - 1);
            int test_type = get_menu_choice(0, 
                get_algorithm_by_id(algo_choice - 1)->test_type_count, 
                "테스트 유형 선택: ");
                
            if (test_type == 0) continue;
            
            result = run_auto_test(algo_choice - 1, test_type - 1);
            if (result != SUCCESS) {
                print_error(result);
            }
        }
        else if (choice == 2) {
            // 수동 테스트 모드
            print_algorithm_selection_menu();
            int algo_choice = get_menu_choice(0, get_algorithm_count(), "알고리즘 선택: ");
            
            if (algo_choice == 0) continue;
            
            print_input_type_menu(algo_choice - 1);
            int input_type = get_menu_choice(0, 
                get_algorithm_by_id(algo_choice - 1)->input_type_count, 
                "입력 유형 선택: ");
                
            if (input_type == 0) continue;
            
            result = run_manual_test(algo_choice - 1, input_type - 1);
            if (result != SUCCESS) {
                print_error(result);
            }
        }
    }
    
    return 0;
}