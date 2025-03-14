# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra -I./include -I./include/algorithm -I./include/common -Isrc
LDFLAGS = -lssl -lcrypto

# 대상 실행 파일
TARGET = crypto_test

# 소스 파일 구조
SOURCES = src/main.c \
          src/algorithm/sha256.c \
          src/algorithm/sha256_register.c \
          src/common/common.c \
          src/common/error.c \
          src/common/algorithm_registry.c \
          src/common/dynamic_menu.c
          
# 나중에 추가될 알고리즘들
# src/algorithm/aes_128.c \
# src/algorithm/aes_128_register.c \
# src/algorithm/cmac_128.c \
# src/algorithm/cmac_128_register.c \
# ...

# 기본 타겟
all: $(TARGET)

# 실행 파일 빌드
$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

# 빌드 파일 정리
clean:
	rm -f $(TARGET)

# 프로그램 실행
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run