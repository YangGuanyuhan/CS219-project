#!/bin/bash

# 设置默认参数
NUM_CASES=10
DATA_TYPE="int"
VECTOR_LENGTH=10000
USE_RANDOM_LENGTH=false
OUTPUT_FILE="temp_test_input.txt"

# 检查路径并设置默认值
TESTCASE_GEN="../src/testcase_generator"
C_VECTOR_DOT="../src/c_vector_dot_product"
JAVA_SRC="../src/JavaVectorDotProduct.java"

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

if [ ! -f "$C_VECTOR_DOT" ]; then
    echo "Error: c_vector_dot_product not found at $C_VECTOR_DOT"
    echo "Building c_vector_dot_product..."
    (cd ../src && make c_vector_dot_product)
    if [ ! -f "$C_VECTOR_DOT" ]; then
        echo "Failed to build c_vector_dot_product"
        exit 1
    fi
fi

# 检查Java源文件是否存在并编译
if [ ! -f "$JAVA_SRC" ]; then
    echo "Error: JavaVectorDotProduct.java not found at $JAVA_SRC"
    exit 1
else
    echo "Compiling Java program..."
    javac $JAVA_SRC
    if [ $? -ne 0 ]; then
        echo "Failed to compile Java program"
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

# 输出测试配置
echo "=========================================="
echo "性能比较测试"
echo "测试配置:"
echo "- 测试案例数量: $NUM_CASES"
echo "- 数据类型: $DATA_TYPE"
echo "- 向量长度: $VECTOR_LENGTH"
echo "- 随机长度: $USE_RANDOM_LENGTH"
echo "=========================================="

# 准备生成测试用例的命令
CMD="$TESTCASE_GEN -n $NUM_CASES -t $DATA_TYPE -l $VECTOR_LENGTH"
if [ "$USE_RANDOM_LENGTH" = true ]; then
    CMD="$CMD -r"
fi
CMD="$CMD -o $OUTPUT_FILE"

# 生成测试用例
echo "生成测试用例中: $NUM_CASES 个 $DATA_TYPE 类型, 向量长度 $VECTOR_LENGTH"
$CMD

# 检查测试用例是否成功生成
if [ ! -f "$OUTPUT_FILE" ]; then
    echo "错误: 无法生成测试输入文件"
    exit 1
fi

# 使用相同输入测试C程序
echo "使用相同输入测试C程序..."
C_RESULT=$($C_VECTOR_DOT < $OUTPUT_FILE)

# 使用相同输入测试Java程序
echo "使用相同输入测试Java程序..."
SCRIPT_DIR="$(pwd)"
JAVA_RESULT=$(cd ../src && java JavaVectorDotProduct < "${SCRIPT_DIR}/${OUTPUT_FILE}")

# 从C程序输出中提取执行时间
C_TIME=$(echo "$C_RESULT" | grep -o "Program execution time: [0-9]\+ microseconds" | grep -o "[0-9]\+")
if [ -z "$C_TIME" ]; then
    # 尝试其他格式
    C_TIME=$(echo "$C_RESULT" | grep -o "执行时间: [0-9]\+ 微秒" | grep -o "[0-9]\+")
    if [ -z "$C_TIME" ]; then
        C_TIME=$(echo "$C_RESULT" | tail -n 1 | grep -o "[0-9]\+")
    fi
fi

# 从Java程序输出中提取执行时间
JAVA_TIME=$(echo "$JAVA_RESULT" | grep -o "程序运行时间: [0-9]\+ 微秒" | grep -o "[0-9]\+")
if [ -z "$JAVA_TIME" ]; then
    # 尝试其他格式
    JAVA_TIME=$(echo "$JAVA_RESULT" | grep -o "Program execution time: [0-9]\+ microseconds" | grep -o "[0-9]\+")
    if [ -z "$JAVA_TIME" ]; then
        JAVA_TIME=$(echo "$JAVA_RESULT" | tail -n 1 | grep -o "[0-9]\+")
    fi
fi

# 检查是否成功提取时间
if [ -z "$C_TIME" ] || [ -z "$JAVA_TIME" ]; then
    echo "警告: 无法提取执行时间，显示原始输出结果"
    echo "C程序输出的最后几行:"
    echo "$C_RESULT" | tail -n 5
    echo ""
    echo "Java程序输出的最后几行:"
    echo "$JAVA_RESULT" | tail -n 5
    exit 1
fi

# 计算平均执行时间
C_AVG=$(echo "scale=2; $C_TIME / $NUM_CASES" | bc)
JAVA_AVG=$(echo "scale=2; $JAVA_TIME / $NUM_CASES" | bc)

# 计算差异
RATIO=$(echo "scale=2; $JAVA_TIME / $C_TIME" | bc)

# 输出比较结果
echo "=========================================="
echo "性能比较结果:"
echo "C总执行时间: $C_TIME us (平均每个测试案例: $C_AVG us)"
echo "Java总执行时间: $JAVA_TIME us (平均每个测试案例: $JAVA_AVG us)"
echo "Java/C时间比率: $RATIO (比值越大说明Java相对C越慢)"
echo "=========================================="

# 清理临时文件
# rm -f $OUTPUT_FILE

exit 0
