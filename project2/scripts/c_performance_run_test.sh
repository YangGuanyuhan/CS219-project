#!/bin/bash

# 检查路径并设置默认值
TESTCASE_GEN="../src/testcase_generator"
VECTOR_DOT="../src/c_vector_dot_product"

# 检查程序是否存在
if [ ! -f "$TESTCASE_GEN" ]; then
    echo "Error: testcase_generator not found at $TESTCASE_GEN"
    echo "Building testcase_generator..."
    (cd ../src && make testcase_generator)
    if [ ! -f "$TESTCASE_GEN" ]; then
        echo "Failed to build testcase_generator"
        exit 1
    fi
fi

if [ ! -f "$VECTOR_DOT" ]; then
    echo "Error: c_vector_dot_product not found at $VECTOR_DOT"
    echo "Building c_vector_dot_product..."
    (cd ../src && make c_vector_dot_product)
    if [ ! -f "$VECTOR_DOT" ]; then
        echo "Failed to build c_vector_dot_product"
        exit 1
    fi
fi

# 设置默认参数
NUM_CASES=10000
DATA_TYPE="int"
VECTOR_LENGTH=1000
USE_RANDOM_LENGTH=false
OUTPUT_FILE="temp_test_input.txt"

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

# 准备命令
CMD="$TESTCASE_GEN -n $NUM_CASES -t $DATA_TYPE -l $VECTOR_LENGTH"
if [ "$USE_RANDOM_LENGTH" = true ]; then
    CMD="$CMD -r"
fi
CMD="$CMD -o $OUTPUT_FILE"

# 生成测试用例
echo "Generating test cases: $NUM_CASES cases of type $DATA_TYPE with vector length $VECTOR_LENGTH"
$CMD

# 检查测试用例是否成功生成
if [ ! -f "$OUTPUT_FILE" ]; then
    echo "Error: Failed to generate test input file"
    exit 1
fi

# 运行测试并提取时间
echo "Running c_vector_dot_product with generated test cases..."
RESULT=$($VECTOR_DOT < $OUTPUT_FILE)

# 提取最后一行（包含执行时间）
EXEC_TIME=$(echo "$RESULT" | tail -n 1)

# 输出执行时间
echo "=========================================="
echo "programming language: C"
echo "Test configuration:"
echo "- Number of cases: $NUM_CASES"
echo "- Data type: $DATA_TYPE"
echo "- Vector length: $VECTOR_LENGTH"
echo "- Random length: $USE_RANDOM_LENGTH"
echo "=========================================="
echo "$EXEC_TIME"
echo "=========================================="
# Calculate average time per test case
if [[ $EXEC_TIME =~ ([0-9]+) ]]; then
    TOTAL_TIME=${BASH_REMATCH[1]}
    AVG_TIME=$(echo "scale=2; $TOTAL_TIME / $NUM_CASES" | bc)
    echo "Average time per test case: $AVG_TIME us"
else
    echo "Error: Failed to parse execution time"
    exit 1
fi

echo "=========================================="

# 清理临时文件
# rm -f $OUTPUT_FILE

exit 0