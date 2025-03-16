#ifndef CALCULATOR_H // 起到头文件保护的功能 也可以用pragma once 代替
#define CALCULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

// 数字类型枚举,用于区分整数、浮点数和科学计数法
typedef enum
{
    INT,
    FLOAT,
    SCIENCE
} NumType;

// 数字结构体，使用联合体存储整数或浮点数
typedef struct
{
    NumType type;
    union
    {
        long long int_val;
        double float_val;
    };
    // 专门用于科学计数法的超大数
    struct
    {
        double mantissa; // 尾数
        int exponent;    // 指数
    } sci;
} Number;

// 函数声明
int parse_number(const char *str, Number *num);
int is_mult_overflow(long long a, long long b);
void handle_operation(Number num1, char op, Number num2);
void process_command_line(int argc, char *argv[]);
void interactive_mode(void);
void handle_sqrt(const char *input);
void print_result(Number result);
int is_scientific_notation(const char *str);
void convert_to_scientific(Number *num);

#endif /* CALCULATOR_H */