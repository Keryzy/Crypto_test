#include "../../include/common.h"
#include <direct.h> // Windows _mkdir 함수용
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

/**
 * 16진수 문자열을 바이트 배열로 변환
 */
int hex_to_bytes(const char* hex_string, uint8_t* bytes, size_t max_len) {
    if (hex_string == NULL) {
        printf("ERROR: Null hex string\n");
        return -3; // 널 포인터 처리
    }
    
    printf("DEBUG: hex_to_bytes called with hex_string length=%zu, max_len=%zu\n", 
           strlen(hex_string), max_len);
    
    // 문자열에 유효하지 않은 문자가 있는지 먼저 확인
    for (size_t i = 0; i < strlen(hex_string); i++) {
        char ch = hex_string[i];
        if (!((ch >= '0' && ch <= '9') || 
              (ch >= 'a' && ch <= 'f') || 
              (ch >= 'A' && ch <= 'F'))) {
            printf("ERROR: Invalid hex character at position %zu: '%c' (ASCII: %d)\n", 
                   i, isprint(ch) ? ch : '.', (unsigned char)ch);
            
            // 앞뒤 컨텍스트 출력
            size_t start = (i > 10) ? i - 10 : 0;
            size_t end = (i + 10 < strlen(hex_string)) ? i + 10 : strlen(hex_string);
            printf("Context: \"");
            for (size_t j = start; j < end; j++) {
                char c = hex_string[j];
                printf("%c", isprint(c) ? c : '.');
            }
            printf("\"\n");
            
            return -1;
        }
    }
    
    size_t hex_len = strlen(hex_string);
    size_t byte_len = hex_len / 2;
    
    // 홀수 길이 처리
    if (hex_len % 2 != 0) {
        printf("WARNING: Odd length hex string (%zu), padding with leading zero\n", hex_len);
        // 새 버퍼에 원본 데이터 복사하되 앞에 0 추가
        char* padded_hex = (char*)malloc(hex_len + 2);
        if (!padded_hex) {
            printf("ERROR: Memory allocation failed\n");
            return -4;
        }
        padded_hex[0] = '0';
        strcpy(padded_hex + 1, hex_string);
        
        // 새 버퍼로 재귀 호출
        int result = hex_to_bytes(padded_hex, bytes, max_len);
        free(padded_hex);
        return result;
    }
    
    if (byte_len > max_len) {
        printf("ERROR: Converted byte length (%zu) exceeds max_len (%zu)\n", byte_len, max_len);
        return -2; // 버퍼 오버플로우 방지
    }
    
    // 2자리 16진수씩 처리
    for (size_t i = 0; i < byte_len; i++) {
        char high = hex_string[i*2];
        char low = hex_string[i*2 + 1];
        
        uint8_t high_val = 0, low_val = 0;
        
        // 상위 4비트
        if (high >= '0' && high <= '9') high_val = high - '0';
        else if (high >= 'a' && high <= 'f') high_val = high - 'a' + 10;
        else if (high >= 'A' && high <= 'F') high_val = high - 'A' + 10;
        
        // 하위 4비트
        if (low >= '0' && low <= '9') low_val = low - '0';
        else if (low >= 'a' && low <= 'f') low_val = low - 'a' + 10;
        else if (low >= 'A' && low <= 'F') low_val = low - 'A' + 10;
        
        bytes[i] = (high_val << 4) | low_val;
    }
    
    printf("DEBUG: hex_to_bytes successfully converted %zu bytes\n", byte_len);
    return byte_len;
}

/**
 * 바이트 배열을 16진수 문자열로 변환
 */
void bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_str, size_t hex_len) {
    if (bytes == NULL || hex_str == NULL || hex_len < len * 2 + 1) {
        return;
    }
    
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + (i * 2), "%02x", bytes[i]);
    }
    
    hex_str[len * 2] = '\0';
}

/**
 * 파일 존재 여부 확인
 */
bool file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

/**
 * 디렉토리 생성 (경로에 여러 단계가 포함된 경우도 처리)
 */
int create_directory(const char* path) {
    char tmp[MAX_FILENAME_LENGTH];
    char* p = NULL;
    size_t len;
    
    // 경로가 비어있으면 실패
    if (path == NULL || *path == '\0') {
        return ERR_INVALID_INPUT;
    }
    
    // 임시 버퍼에 경로 복사
    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    len = strlen(tmp);
    
    // 끝에 슬래시 제거
    if (tmp[len - 1] == '/' || tmp[len - 1] == '\\') {
        tmp[len - 1] = '\0';
    }
    
    // 경로를 '\'로 표준화
    for (p = tmp; *p; p++) {
        if (*p == '/') *p = '\\';
    }
    
    // 경로가 이미 존재하는지 확인
    if (file_exists(tmp)) {
        return SUCCESS;
    }
    
    // 부모 디렉토리 생성
    for (p = tmp + 1; *p; p++) {
        if (*p == '\\') {
            *p = '\0';  // 현재 위치까지의 경로 분리
            
            if (!file_exists(tmp)) {
                int result = _mkdir(tmp);
                if (result != 0 && errno != EEXIST) {
                    return ERR_FILE_WRITE;
                }
            }
            
            *p = '\\';  // 원래 문자로 복원
        }
    }
    
    // 최종 디렉토리 생성
    if (!file_exists(tmp)) {
        int result = _mkdir(tmp);
        if (result != 0 && errno != EEXIST) {
            return ERR_FILE_WRITE;
        }
    }
    
    return SUCCESS;
}

/**
 * 파일에서 MD 값만 추출하여 배열에 저장
 * @return 추출된 MD 값의 개수
 */
int extract_md_values(const char* filename, char md_values[][MAX_LINE_LENGTH], int max_values) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }
    
    int count = 0;
    char line[MAX_LINE_LENGTH];
    
    while (fgets(line, sizeof(line), file) && count < max_values) {
        // 개행 문자 제거
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }
        if (len > 0 && line[len - 1] == '\r') {
            line[len - 1] = '\0';
            len--;
        }
        
        // MD 값 추출
        if (strncmp(line, "MD =", 4) == 0 || strncmp(line, "MD=", 3) == 0) {
            strcpy(md_values[count], line);
            count++;
        }
    }
    
    fclose(file);
    return count;
}

/**
 * 두 파일의 MD 값만 비교
 * @return 일치하면 0, 불일치하면 1, 오류 발생 시 -1
 */
int compare_test_results(const char* output_file, const char* expected_file) {
    FILE* out_file = fopen(output_file, "r");
    if (out_file == NULL) {
        printf("출력 파일을 열 수 없습니다: %s\n", output_file);
        return -1;
    }
    
    FILE* exp_file = fopen(expected_file, "r");
    if (exp_file == NULL) {
        printf("정답 파일을 열 수 없습니다: %s\n", expected_file);
        fclose(out_file);
        return -1;
    }
    
    char out_line[MAX_LINE_LENGTH];
    char exp_line[MAX_LINE_LENGTH];
    int md_count = 0;
    int mismatch_count = 0;
    
    // 각 파일에서 라인별로 읽어가며 MD 값 비교
    while (fgets(out_line, sizeof(out_line), out_file) != NULL) {
        // 줄바꿈 문자 제거
        size_t len = strlen(out_line);
        if (len > 0 && out_line[len - 1] == '\n') {
            out_line[len - 1] = '\0';
        }
        
        // MD 값이 있는 줄만 처리
        if (strncmp(out_line, "MD =", 4) == 0 || strncmp(out_line, "MD=", 3) == 0) {
            // 정답 파일에서 다음 MD 값 찾기
            bool found_md = false;
            while (fgets(exp_line, sizeof(exp_line), exp_file) != NULL) {
                // 줄바꿈 문자 제거
                len = strlen(exp_line);
                if (len > 0 && exp_line[len - 1] == '\n') {
                    exp_line[len - 1] = '\0';
                }
                
                if (strncmp(exp_line, "MD =", 4) == 0 || strncmp(exp_line, "MD=", 3) == 0) {
                    found_md = true;
                    break;
                }
            }
            
            if (!found_md) {
                printf("정답 파일에서 MD 값을 더 이상 찾을 수 없습니다.\n");
                mismatch_count++;
                break;
            }
            
            md_count++;
            
            // MD 값 비교
            if (strcmp(out_line, exp_line) != 0) {
                printf("MD 값 불일치 (테스트 #%d):\n", md_count);
                printf("  계산값: %s\n", out_line);
                printf("  정답값: %s\n", exp_line);
                mismatch_count++;
            }
        }
    }
    
    // 정답 파일에 더 많은 MD 값이 있는지 확인
    bool more_md = false;
    while (fgets(exp_line, sizeof(exp_line), exp_file) != NULL) {
        if (strncmp(exp_line, "MD =", 4) == 0 || strncmp(exp_line, "MD=", 3) == 0) {
            more_md = true;
            mismatch_count++;
            break;
        }
    }
    
    fclose(out_file);
    fclose(exp_file);
    
    if (more_md) {
        printf("정답 파일에 MD 값이 더 많습니다.\n");
    }
    
    if (md_count == 0) {
        printf("MD 값을 찾을 수 없습니다.\n");
        return -1;
    }
    
    printf("총 %d개의 MD 값 비교 완료, 불일치: %d개\n", md_count, mismatch_count);
    return mismatch_count;
}

const char* get_filename_from_path(const char* path) {
    const char* filename = strrchr(path, '\\');
    if (filename == NULL) {
        filename = strrchr(path, '/');
    }
    return filename ? filename + 1 : path;
}