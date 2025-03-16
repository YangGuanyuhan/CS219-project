#include <stdio.h> // 添加标准库头文件

int main() // 修正 main() 语法
{
    int a = 5; // 变量声明放到函数体内
    int b = 2;
    double result;
    result = (double)a / b; // 强制转换为 double 类型
   
    printf("%f\n", result); // 使用 %d 格式化整数输出

    return 0;
}
