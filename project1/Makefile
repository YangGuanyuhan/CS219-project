CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -Iinc
LDFLAGS = -lm
SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
TARGET = calculator

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

run: all
	./$(TARGET)

test: all
	@echo "测试基本运算:"
	./$(TARGET) 5 + 3
	./$(TARGET) 10 - 4
	./$(TARGET) 6 "*" 7
	./$(TARGET)  2 / 3
	./$(TARGET) 3.14 / 0
	./$(TARGET) a x 2
	./$(TARGET) 987654321 x 987654321
	./$(TARGET) 987654321.0 + 0.123456789
	./$(TARGET) "sqrt(8)"
	@echo "测试科学计数法:"
	./$(TARGET) 1.0e10 "*" 1.0e10
	./$(TARGET) 1.0e200 "*" 1.0e200 