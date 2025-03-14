#include "../../include/common.h"
#include <direct.h> // Windows _mkdir 함수용
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>

/**
 * 16진수 문자열을 바이트 배열로 변환
 * @param hex_string 16진수 문자열
 * @param bytes 출력 바이트 배열
 * @param max_len 최대 바이트 길이
 * @param bytes_converted 변환된 바이트 수를 저장할 포인터 (NULL 가능)
 * @return 성공 시 SUCCESS, 실패 시 오류 코드
 */
int hex_to_bytes_ex(const char* hex_string, uint8_t* bytes, size_t max_len, size_t* bytes_converted) {
    if (hex_string == NULL) {
        return ERR_INVALID_INPUT;
    }
    
    // 문자열에 유효하지 않은 문자가 있는지 확인
    for (size_t i = 0; i < strlen(hex_string); i++) {
        char ch = hex_string[i];
        if (!((ch >= '0' && ch <= '9') || 
              (ch >= 'a' && ch <= 'f') || 
              (ch >= 'A' && ch <= 'F'))) {
            return ERR_INVALID_FORMAT;
        }
    }
    
    size_t hex_len = strlen(hex_string);
    size_t byte_len = hex_len / 2;
    
    // 홀수 길이 처리
    if (hex_len % 2 != 0) {
        char* padded_hex = (char*)malloc(hex_len + 2);
        if (!padded_hex) {
            return ERR_MEMORY_ALLOC;
        }
        padded_hex[0] = '0';
        strcpy(padded_hex + 1, hex_string);
        
        int result = hex_to_bytes_ex(padded_hex, bytes, max_len, bytes_converted);
        free(padded_hex);
        return result;
    }
    
    if (byte_len > max_len) {
        return ERR_INVALID_INPUT;
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
    
    // 변환된 바이트 수 반환
    if (bytes_converted != NULL) {
        *bytes_converted = byte_len;
    }
    
    return SUCCESS;
}

// 기존 함수는 호환성을 위해 유지하되, 새 함수를 호출하도록 수정
int hex_to_bytes(const char* hex_string, uint8_t* bytes, size_t max_len) {
    size_t bytes_converted = 0;
    int result = hex_to_bytes_ex(hex_string, bytes, max_len, &bytes_converted);
    
    if (result != SUCCESS) {
        return result; // 오류 코드 반환
    }
    
    return (int)bytes_converted; // 성공 시 변환된 바이트 수 반환
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
 * @return 추출된 MD 값의 개수, 오류 시 정의된 오류 코드
 */
int extract_md_values(const char* filename, char md_values[][MAX_LINE_LENGTH], int max_values) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return ERR_FILE_OPEN;
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
    return count; // 성공 시 추출된 MD 값 개수 반환
}

/**
 * 두 파일의 MD 값만 비교
 * @return 일치하면 0, 불일치하면 불일치 개수, 오류 발생 시 정의된 오류 코드
 */
int compare_test_results(const char* output_file, const char* expected_file) {
    FILE* out_file = fopen(output_file, "r");
    if (out_file == NULL) {
        return ERR_FILE_OPEN;
    }
    
    FILE* exp_file = fopen(expected_file, "r");
    if (exp_file == NULL) {
        fclose(out_file);
        return ERR_FILE_OPEN;
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
                mismatch_count++;
                break;
            }
            
            md_count++;
            
            // MD 값 비교
            if (strcmp(out_line, exp_line) != 0) {
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
    
    if (md_count == 0) {
        return ERR_INVALID_FORMAT; // MD 값을 찾을 수 없음
    }
    
    return mismatch_count; // 성공 시 불일치 개수 반환 (0이면 완전 일치)
}

const char* get_filename_from_path(const char* path) {
    const char* filename = strrchr(path, '\\');
    if (filename == NULL) {
        filename = strrchr(path, '/');
    }
    return filename ? filename + 1 : path;
}

// replace_extension 함수 구현 추가
char* replace_extension(const char* filename, const char* old_ext, const char* new_ext) {
    static char result[MAX_PATH_LENGTH];
    
    strcpy(result, filename);
    size_t filename_len = strlen(filename);
    size_t old_ext_len = strlen(old_ext);
    
    if (filename_len > old_ext_len && 
        strcmp(filename + filename_len - old_ext_len, old_ext) == 0) {
        strcpy(result + filename_len - old_ext_len, new_ext);
    }
    
    return result;
}