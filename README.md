# project1 report

contributor

1. Yang Guanyuhan

2. deepseek

## 1. file structure

```
christy@christywindowscomputer:~/CS219/project1$ tree
.
├── Makefile
├── README.md
├── build
│   └── calculator.o
├── calculator
├── inc
│   └── calculator.h
└── src
    └── calculator.c

3 directories, 6 files
```

结合实验课学的makefile以及在一般项目中的开发规范完成文件结构的架构，添加readme文档也就是本次报告，学习使用git来完成版本管理，ddl结束后同步push到我的github

([YangGuanyuhan github](https://github.com/YangGuanyuhan))

## 2. ai工具使用说明

将文档发送给deepseek以获取对文档的解释和补充，从deepseek思考过程中找到需要完成的核心部分。以及请deepseek提出一个合理的架构使得代码的方法具有高的复用性和健壮性。

deepseek提出需要特别注意的点在代码中

- 大数处理
- 例如，在Linux shell中，如果用户输入的是**，shell会将其扩展为当前目录下的所有文件，导致参数数量错误。因此，用户必须转义*，比如用引号括起来或者用反斜杠。
- 1. 解析命令行参数，确定运行模式（命令行参数模式或交互模式）。

2. 在命令行参数模式下，处理三个参数（两个数和一个运算符），执行计算，处理错误。
  3. 在交互模式下，循环读取输入行，解析每行的表达式，计算结果，直到用户输入quit。
4. 错误处理，包括除以零、无效输入、无法解析的数字等。
  5. 支持大数运算，可能需要不同的数据类型或处理方式。
6. 扩展功能，如支持sqrt等函数。

## 3.代码的整体架构

![屏幕截图 2025-03-15 170340](C:\Users\15235\Pictures\Screenshots\屏幕截图 2025-03-15 170340.png)

## 4.代码部分的结构描述

#### 1，函数的struct，union，enum

在.h文件中使用struc的区分不同的代码类型，简化代码

数字类型存储

```c
// 数字类型枚举,用于区分整数、浮点数和科学计数法
typedef enum
{
    INT,
    FLOAT,
    SCIENCE
} NumType;

```

使用结构体存储数字，里面包括数字类型和数字的具体数值，c语言中的数字只有整数和浮点数，所以可以用union来减少内存开销，但是对于特别的科学计数法，专门使用struct来进行存储和计算。

```c
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
```

#### 2.代码的主体以及方法

###### 1.main方法

根据输出参数判断是进入普通模式，交互模式，还是处理错误

如果**参数格式错误**，输出错误信息，返回错误值并且告知正确用法

```、
fprintf(stderr, "usage: %s [number1 operator number2]\n", argv[0]);
        return 1;
```

###### 2.process_command_line

处理命令行输入并执行运算

向ai学习了处理不能使用的参数，消除编译器警告，表明开发者意图

```
(void)argc;
```

通过输入调用函数parse_number是否能够正常转换为数字

###### 3.parse_number

处理整数、浮点数和科学记数法的输入。

关键点

1. 使用long long 避免溢出，超出这个范围改为使用科学计数法
2. 通过检查 `errno` 是否为 `ERANGE`（范围错误）来捕获数值溢出
3. 通过检查 `*endptr == '\0'` 确保整个字符串都被解析
4. 使用 `num->type` 字段标记解析后的数据类型
5. 使用 `Number` 结构体的联合体设计来高效存储不同类型的数值

###### 4.handle_operation

1. **类型灵活性**：自动检查是否会数字过大过小，能够自动进入科学计数法，如果整数相乘会溢出，转换为浮点数，所有除法都返回浮点数

2. **数值规范化**：

   while (fabs(result.sci.mantissa) >= 10.0) {

     result.sci.mantissa /= 10.0;

     result.sci.exponent++;

   }

3. **精度处理**：

   小数在显示前检查是否可以表示为整数

   科学计数法在合适范围内转回常规表示

4. **多重情况的除以零的检查**

   ```c
   if (op == '/')
       {
           if ((num2.type == INT && num2.int_val == 0) ||
               (num2.type == FLOAT && num2.float_val == 0.0) ||
               (num2.type == SCIENCE && num2.sci.mantissa == 0.0))
           {
               printf("numbers cant divide 0\n");
               return;
           }
       }
   ```

5. **数据类型提升**：

   混合类型运算时自动提升到更高精度类型，整数溢出时自动转换为浮点数

###### 5.is_mult_overflow

###### 6.convert_to_scientific

对于0值，函数直接设置尾数为0而不进行规范化（避免无限循环）

###### 7.print_result

对于浮点数类型的结果最终判断是否是整数，如果是的话，最终按照整数打印

对于科学计数法，判断精度

当 exponent 在 -50 ~ 50 之间时：计算出实际值 val = mantissa × 10^exponent。
如果 val 是整数，按 整数格式 %lld 打印。
否则，按浮点数格式 %.8f 打印。
当 exponent 过大或过小时：
直接按 科学计数法格式 %.8fe%d 打印，例如 1.23456789e25。

###### 8.handle_sqrt

1. 使用指针运算（`input + 5`）跳过"sqrt("前缀

2. 根据指数的奇偶性采用不同处理策略：

```c
if (num.sci.exponent % 2 == 0) {

  // 偶数指数直接处理

  result.sci.exponent = num.sci.exponent / 2;

} else {

  // 奇数指数特殊处理，确保精确计算

  double mantissa = num.sci.mantissa * 10;

  int exponent = num.sci.exponent - 1;

  // ...

}
```

避免了浮点运算中的精度损失

3. 边界条件处理
   验证参数能否正确解析为数值
   专门处理负数开方的错误情况
   对超大数值(>1e100)自动转换为科学计数法

###### 9.interactive_mode

去掉输入中的换行符,避免出现错误

## 5.程序的使用方法

1. **计算表达式模式** (`argc == 4`)

   ./calculator 5 + 3

   - 执行 `process_command_line(argc, argv)`
   - 直接处理命令行提供的两个数字和一个运算符

2. **交互式模式** (`argc == 1`)

   ./calculator

   - 启动 `interactive_mode()`
   - 进入交互式界面，用户可以一直输入表达式进行计算

3. **以sqrt启动交互模式** (`argc == 2 && strcmp(argv[1], "sqrt") == 0`)

   ./calculator sqrt

   - 也进入交互模式，可能是为了兼容性考虑

4. **单次平方根计算** (`argc == 2 && strncmp(argv[1], "sqrt(", 5) == 0`)

   ./calculator "sqrt(16)"

   - 执行 `handle_sqrt(argv[1])`
   - 直接计算括号内数字的平方根并输出

5. **直接退出** (`argc == 2 && strcmp(argv[1], "quit") == 0`)

   ./calculator quit

   - 执行 `return 0`，正常退出程序

## 6.测试样例

使用cunit测试框架完成测试，详见makefile

## 7.总结

实现了文档中的功能，完成了很多的边界检测，超级无敌有效提高了代码的茁壮。

也在里面学习到了指针以及边界处理输入处理的很多知识





