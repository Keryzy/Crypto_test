#include "../../include/common.h"

// 파일에서 한 줄을 안전하게 읽는 함수
int safe_get_line(char* buffer, size_t buffer_size, FILE* file) {
    if (buffer == NULL || file == NULL) {
        return ERR_INVALID_INPUT;
    }

    if (fgets(buffer, buffer_size, file) == NULL) {
        if (feof(file)) {
            buffer[0] = '\0';
            return SUCCESS; // 파일 끝
        } else {
            return ERR_FILE_READ;
        }
    }

    // 개행 문자 제거
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return SUCCESS;
}

// .req/.rsp 파일에서 키-값 쌍 파싱
int parse_key_value(char* line, char** key, char** value) {
    if (line == NULL || key == NULL || value == NULL) {
        return ERR_INVALID_INPUT;
    }

    *key = NULL;
    *value = NULL;

    // 빈 줄 또는 주석 무시
    if (line[0] == '\0' || line[0] == '#') {
        return SUCCESS;
    }

    // 키-값 구분자 찾기
    char* delimiter = strchr(line, '=');
    if (delimiter == NULL) {
        return ERR_INVALID_FORMAT;
    }

    // 키 추출 및 공백 제거
    *key = line;
    *delimiter = '\0';
    delimiter++;

    // 값의 시작 위치 (공백 건너뛰기)
    while (*delimiter == ' ') {
        delimiter++;
    }
    *value = delimiter;

    // 키의 끝 공백 제거
    char* end = *key + strlen(*key) - 1;
    while (end > *key && *end == ' ') {
        *end = '\0';
        end--;
    }

    return SUCCESS;
}