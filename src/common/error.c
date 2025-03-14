#include "../../include/common.h"
#include "../../include/error.h"

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
            
        // 시스템 관련 에러 (100번대)
        case ERR_SYSTEM_DIR:
            return "디렉토리 경로 오류";
        case ERR_SYSTEM_PERMISSION:
            return "권한 없음";
        case ERR_SYSTEM_CONSOLE:
            return "콘솔 출력 설정 실패";
        case ERR_SYSTEM_MEMORY:
            return "시스템 메모리 오류";
            
        // 파일 관련 에러 (110번대)
        case ERR_FILE_OPEN:
            return "파일을 열 수 없습니다";
        case ERR_FILE_READ:
            return "파일을 읽을 수 없습니다";
        case ERR_FILE_WRITE:
            return "파일을 쓸 수 없습니다";
        case ERR_FILE_NOT_FOUND:
            return "파일을 찾을 수 없습니다";
        case ERR_FILE_CREATE:
            return "파일을 생성할 수 없습니다";
            
        // 입력/형식 관련 에러 (120번대)
        case ERR_INVALID_FORMAT:
            return "잘못된 형식입니다";
        case ERR_INVALID_INPUT:
            return "유효하지 않은 입력입니다";
        case ERR_MEMORY_ALLOC:
            return "메모리 할당에 실패했습니다";
            
        // HEX 변환 관련 에러 (130번대)
        case ERR_HEX_INVALID_CHAR:
            return "유효하지 않은 16진수 문자가 포함되어 있습니다";
        case ERR_HEX_BUFFER_OVERFLOW:
            return "16진수 문자열이 버퍼 크기를 초과합니다";
        case ERR_HEX_NULL_INPUT:
            return "NULL 입력이 전달되었습니다";
            
        // 암호화 관련 에러 (140번대)
        case ERR_CRYPTO_OPERATION:
            return "암호화 연산에 실패했습니다";
        case ERR_CRYPTO_INIT:
            return "암호화 라이브러리 초기화에 실패했습니다";
            
        // SHA-256 관련 에러 (150번대)
        case ERR_SHA256_INIT:
            return "SHA-256 초기화에 실패했습니다";
        case ERR_SHA256_UPDATE:
            return "SHA-256 업데이트 연산에 실패했습니다";
        case ERR_SHA256_FINAL:
            return "SHA-256 최종 연산에 실패했습니다";
        case ERR_SHA256_HASH:
            return "SHA-256 해시 연산에 실패했습니다";
            
        // 테스트 관련 에러 (160번대)
        case ERR_TEST_FAILED:
            return "테스트에 실패했습니다";
            
        default:
            return "알 수 없는 오류가 발생했습니다";
    }
}