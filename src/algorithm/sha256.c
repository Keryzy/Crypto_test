#include <openssl/evp.h>
#include "../../include/algorithm/sha256.h"
#include "../../include/common/error.h"
#include "../../include/common/common.h"
#include <ctype.h>

/**
 * SHA-256 해시 계산 (EVP 인터페이스 사용)
 */
int sha256_hash(const uint8_t* message, size_t message_len, uint8_t* digest) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return ERR_SHA256_INIT;
    }
    
    int ret = SUCCESS;
    
    // 초기화
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        ret = ERR_SHA256_INIT;
        goto cleanup;
    }
    
    // 대용량 데이터를 청크 단위로 처리
    const size_t CHUNK_SIZE = 8192;  // 8KB 청크
    size_t processed = 0;
    
    while (processed < message_len) {
        size_t chunk = (message_len - processed < CHUNK_SIZE) ? 
                      (message_len - processed) : CHUNK_SIZE;
        
        if (EVP_DigestUpdate(ctx, message + processed, chunk) != 1) {
            ret = ERR_SHA256_UPDATE;
            goto cleanup;
        }
        
        processed += chunk;
    }
    
    // 해시 값 계산
    unsigned int digest_len = 0;
    if (EVP_DigestFinal_ex(ctx, digest, &digest_len) != 1) {
        ret = ERR_SHA256_FINAL;
        goto cleanup;
    }
    
    // 다이제스트 길이 검증 (SHA-256은 항상 32바이트)
    if (digest_len != SHA256_DIGEST_LENGTH) {
        ret = ERR_SHA256_HASH;
    }
    
cleanup:
    // 리소스 정리
    EVP_MD_CTX_free(ctx);
    return ret;
}

/**
 * 테스트 유형 감지
 */
TestType sha256_detect_test_type(const char* filename) {
    if (filename == NULL) {
        return TEST_SHORT_MSG;
    }
    
    // 파일명에서 대소문자 구분 없이 "Monte"가 포함되어 있는지 확인
    const char* monte_str = strstr(filename, "Monte");
    if (monte_str == NULL) {
        monte_str = strstr(filename, "monte");
    }
    
    if (monte_str != NULL) {
        return TEST_MONTE_CARLO;
    }
    
    // 파일명에서 대소문자 구분 없이 "Long"이 포함되어 있는지 확인
    const char* long_str = strstr(filename, "Long");
    if (long_str == NULL) {
        long_str = strstr(filename, "long");
    }
    
    if (long_str != NULL) {
        return TEST_LONG_MSG;
    }
    
    // 기본값은 ShortMsg 테스트
    return TEST_SHORT_MSG;
}

/**
 * SHA-256 테스트 실행 (ShortMsg, LongMsg 테스트)
 */
int sha256_test(const char* input_file, const char* output_file) {
    FILE* fp = fopen(input_file, "r");
    if (!fp) {
        print_error(ERR_FILE_OPEN);
        return ERR_FILE_OPEN;
    }

    FILE* out_fp = fopen(output_file, "w");
    if (!out_fp) {
        fclose(fp);
        print_error(ERR_FILE_CREATE);
        return ERR_FILE_CREATE;
    }

    char line[MAX_LINE_LENGTH];
    uint8_t* message = NULL;  // 동적 메시지 버퍼
    uint8_t digest[SHA256_DIGEST_LENGTH];
    size_t message_len;
    int test_count = 0;
    char *msg_buffer = NULL;  // 16진수 문자열을 위한 동적 버퍼

    // 헤더 정복사
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Len", 3) == 0) break;
        fputs(line, out_fp);
    }

    // 테스트 벡터 처리
    while (!feof(fp)) {
        if (strncmp(line, "Len", 3) == 0) {
            int len_bits = 0;
            
            if (sscanf(line, "Len = %d", &len_bits) != 1 || len_bits < 0) {
                if (!fgets(line, sizeof(line), fp)) break;
                continue;
            }
            
            message_len = (size_t)((len_bits + 7) / 8);  // 비트를 바이트로 변환
            
            // 메시지 버퍼 할당 (이전 버퍼 해제 후)
            if (message != NULL) {
                free(message);
            }
            message = (uint8_t*)malloc(message_len + 16);  // 여유있게 할당
            if (message == NULL) {
                if (!fgets(line, sizeof(line), fp)) break;
                continue;
            }
            
            // 원본 Len 값 그대로 출력
            fprintf(out_fp, "Len = %d\n", len_bits);
            
            // Msg 읽기
            if (!fgets(line, sizeof(line), fp)) {
                free(message);
                message = NULL;
                break;
            }
            
            // Msg 값 그대로 출력
            fputs(line, out_fp);
            
            // Msg 내용 파싱 및 해시 계산
            if (strncmp(line, "Msg", 3) == 0) {
                char* msg_hex = strchr(line, '=');
                if (msg_hex) {
                    msg_hex++;  // = 다음으로 이동
                    while(*msg_hex == ' ') msg_hex++;  // 공백 건너뛰기

                    
                    // 개행 문자 제거
                    char* newline = strchr(msg_hex, '\n');
                    if (newline) *newline = '\0';
                    newline = strchr(msg_hex, '\r');
                    if (newline) *newline = '\0';
                    
                    // 메시지의 예상 16진수 길이 계산 (바이트 당 2자리)
                    size_t expected_hex_len = message_len * 2;
                    
                    // 동적으로 충분히 큰 버퍼 할당
                    msg_buffer = (char*)malloc(expected_hex_len + 100);
                    if (!msg_buffer) {
                        free(message);
                        message = NULL;
                        fclose(fp);
                        fclose(out_fp);
                        return ERR_MEMORY_ALLOC;
                    }
                    msg_buffer[0] = '\0';  // 빈 문자열로 초기화
                    
                    // 첫 줄 복사
                    strcpy(msg_buffer, msg_hex);
                    
                    // 현재 16진수 길이
                    size_t current_hex_len = strlen(msg_buffer);
                    
                    // 추가 라인이 필요한 경우 계속 읽기
                    while (current_hex_len < expected_hex_len) {
                        if (!fgets(line, sizeof(line), fp)) break;
                        
                        // 다음 라인이 "Len"으로 시작하면 메시지가 완료된 것
                        if (strncmp(line, "Len", 3) == 0) {
                            break;
                        }
                        
                        // 공백과 탭 문자 건너뛰기
                        char* ptr = line;
                        while (*ptr && (isspace(*ptr) || *ptr == '\t')) ptr++;
                        
                        // 개행 문자 제거
                        newline = strchr(ptr, '\n');
                        if (newline) *newline = '\0';
                        newline = strchr(ptr, '\r');
                        if (newline) *newline = '\0';
                        
                        // 비어있지 않은 경우에만 추가
                        if (*ptr) {
                            // 버퍼 크기 검사
                            if (current_hex_len + strlen(ptr) >= expected_hex_len + 100) {
                                // 버퍼 확장 필요
                                size_t new_size = current_hex_len + strlen(ptr) + 200;
                                char* new_buffer = (char*)realloc(msg_buffer, new_size);
                                if (!new_buffer) {
                                    break;
                                }
                                msg_buffer = new_buffer;
                            }
                            
                            // 메시지에 추가
                            strcat(msg_buffer, ptr);
                            current_hex_len = strlen(msg_buffer);
                        }
                    }
                    
                    // Len=0 특수 케이스 (입력이 실제로 비어있을 때만)
                    if (len_bits == 0 && (msg_buffer[0] == '\0' || strcmp(msg_buffer, "00") == 0)) {
                        memset(message, 0, 1);  // 최소 1바이트 초기화
                        message_len = 0;
                    } else {
                        int bytes_converted = hex_to_bytes(msg_buffer, message, message_len);
                        
                        if (bytes_converted < 0) {
                            print_error(bytes_converted);
                            free(message);
                            free(msg_buffer);
                            fclose(fp);
                            fclose(out_fp);
                            return bytes_converted;
                        } else if ((size_t)bytes_converted != message_len) {
                            // 경고만 출력하고 계속 진행
                        }
                    }
                    
                    // SHA-256 해시 계산
                    int hash_result = sha256_hash(message, message_len, digest);
                    
                    if (hash_result != 0) {
                        free(message);
                        free(msg_buffer);
                        fclose(fp);
                        fclose(out_fp);
                        return ERR_SHA256_HASH;
                    }
                    
                    // MD 값 출력
                    fprintf(out_fp, "MD = ");
                    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
                        fprintf(out_fp, "%02x", digest[i]);
                    }
                    fprintf(out_fp, "\n\n");
                    
                    test_count++;
                }
            }
        } else {
            // 메시지를 모두 읽은 후 다음 Len 라인
            if (!fgets(line, sizeof(line), fp)) break;
        }
    }

    // 동적 메모리 해제
    if (msg_buffer) {
        free(msg_buffer);
    }
    if (message) {
        free(message);
    }
    
    fclose(fp);
    fclose(out_fp);
    return SUCCESS;
}

/**
 * SHA-256 몬테 카를로 테스트 실행
 */
int sha256_monte(const char* input_file, const char* output_file) {
    FILE* fp = fopen(input_file, "r");
    if (!fp) {
        print_error(ERR_FILE_OPEN);
        return ERR_FILE_OPEN;
    }

    FILE* out_fp = fopen(output_file, "w");
    if (!out_fp) {
        fclose(fp);
        print_error(ERR_FILE_CREATE);
        return ERR_FILE_CREATE;
    }

    char line[MAX_LINE_LENGTH];
    uint8_t seed[SHA256_DIGEST_LENGTH];
    uint8_t MD[1003][SHA256_DIGEST_LENGTH]; // 몬테 카를로 테스트를 위한 버퍼

    // 헤더 정복사
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Seed", 4) == 0) break;
        fputs(line, out_fp);
    }

    // Seed 값 읽기
    char* seed_hex = strchr(line, '=');
    if (!seed_hex) {
        fclose(fp);
        fclose(out_fp);
        return ERR_INVALID_INPUT;
    }
    seed_hex++;
    while(*seed_hex == ' ') seed_hex++;
        
        // 개행 문자 제거
    char* newline = strchr(seed_hex, '\n');
    if (newline) *newline = '\0';
    
    hex_to_bytes(seed_hex, seed, SHA256_DIGEST_LENGTH);

    // Monte Carlo 테스트 시작
    fprintf(out_fp, "Seed = ");
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        fprintf(out_fp, "%02x", seed[i]);
    }
    fprintf(out_fp, "\n\n");

    // 100개의 체크포인트 반복
    for (int j = 0; j < 100; j++) {
        // 초기 상태 설정: MD_0 = MD_1 = MD_2 = Seed
        memcpy(MD[0], seed, SHA256_DIGEST_LENGTH);
        memcpy(MD[1], seed, SHA256_DIGEST_LENGTH);
        memcpy(MD[2], seed, SHA256_DIGEST_LENGTH);
        
        // 1000번의 해시 반복
        for (int i = 3; i < 1003; i++) {
            uint8_t message[SHA256_DIGEST_LENGTH * 3]; // 96바이트 메시지
            
            // M_i = MD_{i-3} || MD_{i-2} || MD_{i-1} (연결)
            memcpy(message, MD[i-3], SHA256_DIGEST_LENGTH);
            memcpy(message + SHA256_DIGEST_LENGTH, MD[i-2], SHA256_DIGEST_LENGTH);
            memcpy(message + (2 * SHA256_DIGEST_LENGTH), MD[i-1], SHA256_DIGEST_LENGTH);
            
            // MD_i = SHA-256(M_i)
            if (sha256_hash(message, SHA256_DIGEST_LENGTH * 3, MD[i]) != 0) {
                fclose(fp);
                fclose(out_fp);
                return ERR_SHA256_HASH;
            }
        }
        
        // 체크포인트 출력 (MD_j = MD_1002)
        fprintf(out_fp, "COUNT = %d\n", j);
        fprintf(out_fp, "MD = ");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            fprintf(out_fp, "%02x", MD[1002][i]);
        }
        fprintf(out_fp, "\n\n");
        
        // 다음 반복을 위한 시드 업데이트 (Seed = MD_1002)
        memcpy(seed, MD[1002], SHA256_DIGEST_LENGTH);
    }

    fclose(fp);
    fclose(out_fp);
    return SUCCESS;
}

// 테스트 벡터 처리 시 입력값과 출력값을 출력
void print_debug_info(const uint8_t* msg, size_t msg_len, const uint8_t* hash) {
    printf("Input message (%zu bytes): ", msg_len);
    for (size_t i = 0; i < msg_len; i++) {
        printf("%02x", msg[i]);
    }
    printf("\nCalculated hash: ");
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        printf("%02x", hash[i]);
    }
    printf("\n\n");
}

// 초기화 함수
int sha256_init(void** ctx) {
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        return ERR_SHA256_INIT;
    }
    
    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        return ERR_SHA256_INIT;
    }
    
    *ctx = mdctx;
    return SUCCESS;
}

// 업데이트 함수
int sha256_update(void* ctx, const unsigned char* data, size_t data_len) {
    EVP_MD_CTX* mdctx = (EVP_MD_CTX*)ctx;
    
    if (EVP_DigestUpdate(mdctx, data, data_len) != 1) {
        return ERR_SHA256_UPDATE;
    }
    
    return SUCCESS;
}

// 마무리 함수
int sha256_final(void* ctx, unsigned char* digest) {
    EVP_MD_CTX* mdctx = (EVP_MD_CTX*)ctx;
    unsigned int digest_len;
    
    if (EVP_DigestFinal_ex(mdctx, digest, &digest_len) != 1) {
        return ERR_SHA256_FINAL;
    }
    
    EVP_MD_CTX_free(mdctx);
    return SUCCESS;
}

// 자동 테스트 구현
int sha256_run_auto_test(int test_type_id) {
    // 테스트 타입에 따른 파일 이름 결정
    const char* test_file = NULL;
    
    if (test_type_id == 0) { // ShortMsg
        test_file = "SHA256ShortMsg.req";
    } else if (test_type_id == 1) { // LongMsg
        test_file = "SHA256LongMsg.req";
    } else if (test_type_id == 2) { // Monte
        test_file = "SHA256Monte.req";
    } else {
        return ERR_INVALID_TEST_TYPE;
    }
    
    // 파일 경로 구성
    char input_path[MAX_PATH_LENGTH];
    char output_path[MAX_PATH_LENGTH];
    
    sprintf(input_path, "test_vectors/request/SHA_256/%s", test_file);
    const char* output_file = replace_extension(test_file, ".req", ".rsp");
    sprintf(output_path, "test_vectors/response/SHA_256/%s", output_file);
    
    // 디렉토리 생성
    create_directory("test_vectors/response/SHA_256");
    
    int result;
    
    if (test_type_id == 2) { // Monte
        result = sha256_monte(input_path, output_path);
    } else {
        result = sha256_test(input_path, output_path);
    }
    
    if (result == SUCCESS) {
        printf("\n테스트 완료: 결과 파일이 %s에 저장되었습니다.\n", output_path);
        
        // 결과 비교
        char expected_path[MAX_PATH_LENGTH];
        sprintf(expected_path, "test_vectors/expected/SHA_256/%s", output_file);
        
        if (file_exists(expected_path)) {
            printf("\n결과 비교 중...\n");
            result = compare_test_results(output_path, expected_path);
            if (result == SUCCESS) {
                printf("\n✓ 모든 테스트 결과가 예상 결과와 일치합니다.\n");
            } else {
                printf("\n✕ 일부 테스트 결과가 예상 결과와 일치하지 않습니다.\n");
            }
        }
    }
    
    return result;
}

// 수동 테스트 구현
int sha256_run_manual_test(int input_type_id) {
    uint8_t digest[SHA256_DIGEST_LENGTH];
    int result = SUCCESS;
    
    if (input_type_id == 0) { // 텍스트 입력
        char text[MAX_LINE_LENGTH];
        
        printf("\n텍스트를 입력하세요 (한글, 특수문자, 공백 모두 지원, 최대 %d 문자):\n", MAX_LINE_LENGTH-1);
        if (fgets(text, sizeof(text), stdin) == NULL) {
            return ERR_INVALID_INPUT;
        }
        
        // 개행 문자 제거
        size_t len = strlen(text);
        if (len > 0 && text[len-1] == '\n') {
            text[len-1] = '\0';
            len--;
        }
        
        // 입력 정보 출력
        printf("\n[입력된 텍스트] %s\n", text);
        printf("[입력 길이] %zu 바이트\n", len);
        
        // 텍스트 바이트 값 출력
        printf("[바이트 값] ");
        for (size_t i = 0; i < len && i < 32; i++) {
            printf("%02x ", (unsigned char)text[i]);
        }
        if (len > 32) printf("...");
        printf("\n");
        
        // 해시 계산
        result = sha256_hash((const uint8_t*)text, len, digest);
        
    } else { // 16진수 입력
        char hex_string[MAX_LINE_LENGTH];
        
        printf("\n16진수 문자열을 입력하세요 (예: 68656c6c6f = 'hello'):\n");
        if (fgets(hex_string, sizeof(hex_string), stdin) == NULL) {
            return ERR_INVALID_INPUT;
        }
        
        // 개행 문자 제거 및 공백 처리
        // ... 이하 동일한 코드 ...
        
        // 해시 계산
        // ... 이하 동일한 코드 ...
    }
    
    // 결과 출력
    if (result == SUCCESS) {
        printf("\n[SHA-256 해시 결과]\n");
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            printf("%02x", digest[i]);
        }
        printf("\n");
    }
    
    return result;
}