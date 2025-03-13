#include "../../include/common.h"

/**
 * 에러 코드에 해당하는 에러 메시지를 출력
 */
void print_error(int error_code) {
    printf("\n오류: %s (코드: %d)\n", get_error_message(error_code), error_code);
}

/**
 * 에러 코드에 해당하는 메시지 반환
 */
const char* get_error_message(int error_code) {
    switch (error_code) {
        case SUCCESS:
            return "성공";
        case ERR_FILE_OPEN:
            return "파일을 열 수 없습니다";
        case ERR_FILE_READ:
            return "파일을 읽을 수 없습니다";
        case ERR_FILE_WRITE:
            return "파일을 쓸 수 없습니다";
        case ERR_FILE_NOT_FOUND:
            return "파일을 찾을 수 없습니다";
        case ERR_INVALID_FORMAT:
            return "잘못된 형식입니다";
        case ERR_CRYPTO_OPERATION:
            return "암호화 연산에 실패했습니다";
        case ERR_INVALID_INPUT:
            return "유효하지 않은 입력입니다";
        case ERR_TEST_FAILED:
            return "테스트에 실패했습니다";
        case ERR_FILE_OPEN_FAILED:
            return "파일을 열 수 없습니다";
        case ERR_FILE_CREATE_FAILED:
            return "파일을 생성할 수 없습니다";
        case ERR_SHA256_INIT_FAILED:
            return "SHA-256 초기화에 실패했습니다";
        case ERR_SHA256_UPDATE_FAILED:
            return "SHA-256 업데이트 연산에 실패했습니다";
        case ERR_SHA256_FINAL_FAILED:
            return "SHA-256 최종 연산에 실패했습니다";
        case ERR_SHA256_HASH_FAILED:
            return "SHA-256 해시 연산에 실패했습니다";
        default:
            return "알 수 없는 오류가 발생했습니다";
    }
}