#!/bin/bash

# 设置默认参数
NUM_CASES=10000
DATA_TYPE="int"
VECTOR_LENGTH=10000
USE_RANDOM_LENGTH=false
OUTPUT_FILE="temp_test_input.txt"

# 检查路径并设置默认值
TESTCASE_GEN="../src/testcase_generator"
ORIGINAL_C="../src/c_vector_dot_product"
OPTIMIZED_C="../src/optimal_c_vector_product"

# 检查程序是否存在
if [ ! -f "$TESTCASE_GEN" ]; then
    echo "Error: testcase_generator not found at $TESTCASE_GEN" >&2
    echo "Building testcase_generator..." >&2
    (cd ../src && make testcase_generator) >&2
    if [ ! -f "$TESTCASE_GEN" ]; then
        echo "Failed to build testcase_generator" >&2
        exit 1
    fi
fi

if [ ! -f "$ORIGINAL_C" ]; then
    echo "Error: c_vector_dot_product not found at $ORIGINAL_C" >&2
    echo "Building c_vector_dot_product..." >&2
    (cd ../src && make c_vector_dot_product) >&2
    if [ ! -f "$ORIGINAL_C" ]; then
        echo "Failed to build c_vector_dot_product" >&2
        exit 1
    fi
fi

if [ ! -f "$OPTIMIZED_C" ]; then
    echo "Error: optimal_c_vector_product not found at $OPTIMIZED_C" >&2
    echo "Building optimal_c_vector_product..." >&2
    echo "注意: 优化版本需要特殊编译选项以启用SIMD和OpenMP" >&2
    (cd ../src && gcc -O3 -march=native -fopenmp -mavx2 -mfma optimal_c_vector_product.c -o optimal_c_vector_product) >&2
    if [ ! -f "$OPTIMIZED_C" ]; then
        echo "Failed to build optimal_c_vector_product" >&2
        exit 1
    fi
fi

# 解析命令行参数
while getopts "n:t:l:r" opt; do
    case $opt in
        n) NUM_CASES=$OPTARG ;;
        t) DATA_TYPE=$OPTARG ;;
        l) VECTOR_LENGTH=$OPTARG ;;
        r) USE_RANDOM_LENGTH=true ;;
        \?) echo "Invalid option: -$OPTARG" >&2; exit 1 ;;
    esac
done

# 准备生成测试用例的命令
CMD="$TESTCASE_GEN -n $NUM_CASES -t $DATA_TYPE -l $VECTOR_LENGTH"
if [ "$USE_RANDOM_LENGTH" = true ]; then
    CMD="$CMD -r"
fi
CMD="$CMD -o $OUTPUT_FILE"

# 生成测试用例（不输出过程信息）
$CMD > /dev/null 2>&1

# 检查测试用例是否成功生成
if [ ! -f "$OUTPUT_FILE" ]; then
    echo "错误: 无法生成测试输入文件" >&2
    exit 1
fi

# 创建两个临时文件的副本
ORIGINAL_INPUT="original_input.txt"
OPTIMIZED_INPUT="optimized_input.txt"

# 复制输入文件
cp "$OUTPUT_FILE" "$ORIGINAL_INPUT" > /dev/null 2>&1
cp "$OUTPUT_FILE" "$OPTIMIZED_INPUT" > /dev/null 2>&1

# 测试原始C程序（不输出过程信息）
ORIGINAL_RESULT=$($ORIGINAL_C < "$ORIGINAL_INPUT" 2>/dev/null)

# 测试优化的C程序（不输出过程信息）
OPTIMIZED_RESULT=$($OPTIMIZED_C < "$OPTIMIZED_INPUT" 2>/dev/null)

# 如果优化程序没有输出结果，尝试重新编译并测试
if ! echo "$OPTIMIZED_RESULT" | grep -q "Int dot product result\|Double dot product result"; then
    (cd ../src && gcc -O3 -march=native -fopenmp -mavx2 -mfma optimal_c_vector_product.c -o optimal_c_vector_product) > /dev/null 2>&1
    OPTIMIZED_RESULT=$($OPTIMIZED_C < "$OPTIMIZED_INPUT" 2>/dev/null)
fi

# 从原始C程序输出中提取执行时间
ORIGINAL_TIME=$(echo "$ORIGINAL_RESULT" | grep -o "Program execution time: [0-9]\+ microseconds" | grep -o "[0-9]\+")
if [ -z "$ORIGINAL_TIME" ]; then
    # 尝试其他格式
    ORIGINAL_TIME=$(echo "$ORIGINAL_RESULT" | grep -o "执行时间: [0-9]\+ 微秒" | grep -o "[0-9]\+")
    if [ -z "$ORIGINAL_TIME" ]; then
        ORIGINAL_TIME=$(echo "$ORIGINAL_RESULT" | tail -n 1 | grep -o "[0-9]\+")
    fi
fi

# 从优化C程序输出中提取执行时间
OPTIMIZED_TIME=$(echo "$OPTIMIZED_RESULT" | grep -o "Program execution time: [0-9]\+ microseconds" | grep -o "[0-9]\+")
if [ -z "$OPTIMIZED_TIME" ]; then
    # 尝试其他格式
    OPTIMIZED_TIME=$(echo "$OPTIMIZED_RESULT" | grep -o "执行时间: [0-9]\+ 微秒" | grep -o "[0-9]\+")
    if [ -z "$OPTIMIZED_TIME" ]; then
        OPTIMIZED_TIME=$(echo "$OPTIMIZED_RESULT" | tail -n 1 | grep -o "[0-9]\+")
    fi
fi

# 检查是否成功提取时间
if [ -z "$ORIGINAL_TIME" ] || [ -z "$OPTIMIZED_TIME" ]; then
    echo "警告: 无法提取执行时间" >&2
    exit 1
fi

# 计算平均执行时间
ORIGINAL_AVG=$(echo "scale=2; $ORIGINAL_TIME / $NUM_CASES" | bc)
OPTIMIZED_AVG=$(echo "scale=2; $OPTIMIZED_TIME / $NUM_CASES" | bc)

# 计算性能提升
SPEEDUP=$(echo "scale=2; $ORIGINAL_TIME / $OPTIMIZED_TIME" | bc)

# 只输出性能比较结果，添加参数信息
echo "=========================================="
echo "测试配置:"
echo "- 测试案例数量: $NUM_CASES"
echo "- 数据类型: $DATA_TYPE"
echo "- 向量长度: $VECTOR_LENGTH"
echo "- 随机长度: $USE_RANDOM_LENGTH"
echo "性能比较结果:"
echo "原始C程序总执行时间: $ORIGINAL_TIME us (平均每个测试案例: $ORIGINAL_AVG us)"
echo "优化C程序总执行时间: $OPTIMIZED_TIME us (平均每个测试案例: $OPTIMIZED_AVG us)"
echo "性能提升: ${SPEEDUP}x (值越大说明优化效果越好)"
echo "=========================================="

# 清理临时文件
rm -f "$ORIGINAL_INPUT" "$OPTIMIZED_INPUT" "$OUTPUT_FILE" > /dev/null 2>&1

exit 0
