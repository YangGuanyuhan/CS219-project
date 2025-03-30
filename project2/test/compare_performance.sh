#!/bin/bash

# 定义测试参数
NUM_CASES=1000
VECTOR_LENGTH=100
DATA_TYPE="int"
TEST_FILE="perf_comparison_test.txt"

# 确保程序已编译
echo "Ensuring programs are compiled..."
if [ ! -f "../src/vector_dot_product" ]; then
    (cd ../src && make vector_dot_product)
fi

if [ ! -f "../src/JavaVectorDotProduct.class" ]; then
    javac ../src/JavaVectorDotProduct.java
fi

# 生成测试用例
echo "Generating test cases: $NUM_CASES cases of type $DATA_TYPE with vector length $VECTOR_LENGTH"
../src/testcase_generator -n $NUM_CASES -t $DATA_TYPE -l $VECTOR_LENGTH -o $TEST_FILE

# 运行C程序测试
echo -e "\nRunning C implementation test..."
C_OUTPUT=$(../src/vector_dot_product < $TEST_FILE)
C_TIME=$(echo "$C_OUTPUT" | grep "程序运行时间" | awk '{print $2}')

# 运行Java程序测试
echo -e "\nRunning Java implementation test..."
JAVA_OUTPUT=$(java -cp ../src JavaVectorDotProduct < $TEST_FILE)
JAVA_TIME=$(echo "$JAVA_OUTPUT" | grep "程序运行时间" | awk '{print $2}')

# 输出性能比较
echo -e "\n===== Performance Comparison ====="
echo "Test configuration:"
echo "- Number of cases: $NUM_CASES"
echo "- Data type: $DATA_TYPE" 
echo "- Vector length: $VECTOR_LENGTH"
echo "====================================="
echo "C implementation time: $C_TIME 微秒"
echo "Java implementation time: $JAVA_TIME 微秒"

# 计算比率
if [ -n "$C_TIME" ] && [ -n "$JAVA_TIME" ]; then
    RATIO=$(echo "scale=2; $JAVA_TIME / $C_TIME" | bc)
    echo "Java/C ratio: ${RATIO}x (Java is ${RATIO} times slower than C)"
else
    echo "Could not calculate ratio: execution times not found"
fi
echo "====================================="

# 清理
rm -f $TEST_FILE
echo "Performance comparison completed!"
