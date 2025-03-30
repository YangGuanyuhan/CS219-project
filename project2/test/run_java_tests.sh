#!/bin/bash

# 定义Java文件路径
JAVA_SRC="../src/JavaVectorDotProduct.java"
JAVA_CLASS="../src/JavaVectorDotProduct.class"

# 确保Java程序存在
if [ ! -f "$JAVA_SRC" ]; then
    echo "Error: Java source file not found at $JAVA_SRC"
    echo "Creating Java source file from template..."
    
    # 复制Java代码到正确位置
    cp -f ../src/VectorDotProduct.java $JAVA_SRC
    
    if [ ! -f "$JAVA_SRC" ]; then
        echo "Failed to create Java source file"
        exit 1
    fi
fi

# 编译Java程序
echo "Compiling Java program..."
javac $JAVA_SRC

# 检查编译是否成功
if [ ! -f "$JAVA_CLASS" ]; then
    echo "Error: Java compilation failed"
    exit 1
fi

# 设置Java程序的目录为工作目录
cd ../src

# 运行Java测试
echo "Running Java tests..."

echo "1. Running with test input..."
java JavaVectorDotProduct < ../test/test_input.txt

echo -e "\n2. Running with error input..."
java JavaVectorDotProduct < ../test/test_error_input.txt

# 返回到测试目录
cd ../test

# 清理
echo -e "\nCleaning up..."
rm -f java_perf_test.txt

echo "Java tests completed!"
