#include <openssl/sha.h>
#include "sha256.h"
#include "error.h"
#include "common.h"
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