#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <emmintrin.h> // SSE2
#include <immintrin.h> // AVX/AVX2
#include <omp.h>       // OpenMP

// 内存对齐宏，优化内存访问
#define ALIGN __attribute__((aligned(32)))
#define MAX_VECTOR_SIZE 1000000

// 使用SIMD指令的整数向量点积 (SSE2)
long long dot_product_int_simd(int *a, int *b, int length)
{
    __m128i sum_vec = _mm_setzero_si128(); // 初始化向量寄存器为0
    long long sum = 0;
    int i;

    // 使用SIMD指令并行处理4个整数
    for (i = 0; i <= length - 4; i += 4)
    {
        __m128i a_vec = _mm_loadu_si128((__m128i *)&a[i]);
        __m128i b_vec = _mm_loadu_si128((__m128i *)&b[i]);

        // SSE中没有直接的整数乘法累加，使用专用指令完成
        __m128i mul_lo = _mm_mullo_epi32(a_vec, b_vec);
        sum_vec = _mm_add_epi32(sum_vec, mul_lo);
    }

    // 将向量寄存器中的4个整数相加
    int result[4] ALIGN;
    _mm_store_si128((__m128i *)result, sum_vec);

    for (int j = 0; j < 4; j++)
    {
        sum += result[j];
    }

    // 处理剩余的元素
    for (; i < length; i++)
    {
        sum += a[i] * b[i];
    }

    return sum;
}

// 使用SIMD指令的浮点向量点积 (AVX)
double dot_product_double_simd(double *a, double *b, int length)
{
    __m256d sum_vec = _mm256_setzero_pd(); // 初始化向量寄存器为0
    double sum = 0.0;
    int i;

    // 使用AVX指令并行处理4个双精度浮点数
    for (i = 0; i <= length - 4; i += 4)
    {
        __m256d a_vec = _mm256_loadu_pd(&a[i]);
        __m256d b_vec = _mm256_loadu_pd(&b[i]);

        // 使用fused multiply-add来提高精度和性能
        sum_vec = _mm256_fmadd_pd(a_vec, b_vec, sum_vec);
    }

    // 将向量寄存器中的4个浮点数相加
    double result[4] ALIGN;
    _mm256_store_pd(result, sum_vec);

    for (int j = 0; j < 4; j++)
    {
        sum += result[j];
    }

    // 处理剩余的元素
    for (; i < length; i++)
    {
        sum += a[i] * b[i];
    }

    return sum;
}

// 多线程优化的整数向量点积
long long dot_product_int_parallel(int *a, int *b, int length)
{
    long long sum = 0;

#pragma omp parallel reduction(+ : sum)
    {
#pragma omp for schedule(static)
        for (int i = 0; i < length; i++)
        {
            sum += (long long)a[i] * b[i];
        }
    }

    return sum;
}

// 多线程优化的浮点向量点积
double dot_product_double_parallel(double *a, double *b, int length)
{
    double sum = 0.0;

#pragma omp parallel reduction(+ : sum)
    {
#pragma omp for schedule(static)
        for (int i = 0; i < length; i++)
        {
            sum += a[i] * b[i];
        }
    }

    return sum;
}

// 循环展开优化的整数向量点积
long long dot_product_int_unrolled(int *a, int *b, int length)
{
    long long sum = 0;
    int i;

    // 8倍循环展开，减少循环开销
    for (i = 0; i <= length - 8; i += 8)
    {
        sum += (long long)a[i] * b[i] +
               (long long)a[i + 1] * b[i + 1] +
               (long long)a[i + 2] * b[i + 2] +
               (long long)a[i + 3] * b[i + 3] +
               (long long)a[i + 4] * b[i + 4] +
               (long long)a[i + 5] * b[i + 5] +
               (long long)a[i + 6] * b[i + 6] +
               (long long)a[i + 7] * b[i + 7];
    }

    // 处理剩余的元素
    for (; i < length; i++)
    {
        sum += (long long)a[i] * b[i];
    }

    return sum;
}

// 循环展开优化的浮点向量点积
double dot_product_double_unrolled(double *a, double *b, int length)
{
    double sum = 0.0;
    int i;

    // 8倍循环展开，减少循环开销
    for (i = 0; i <= length - 8; i += 8)
    {
        sum += a[i] * b[i] +
               a[i + 1] * b[i + 1] +
               a[i + 2] * b[i + 2] +
               a[i + 3] * b[i + 3] +
               a[i + 4] * b[i + 4] +
               a[i + 5] * b[i + 5] +
               a[i + 6] * b[i + 6] +
               a[i + 7] * b[i + 7];
    }

    // 处理剩余的元素
    for (; i < length; i++)
    {
        sum += a[i] * b[i];
    }

    return sum;
}

// 自动选择最佳方法计算整数向量点积
long long dot_product_int_optimal(int *a, int *b, int length)
{
    // 根据向量长度选择最佳算法
    if (length >= 1000)
    {
#ifdef _OPENMP
        return dot_product_int_parallel(a, b, length);
#else
        return dot_product_int_simd(a, b, length);
#endif
    }
    else if (length >= 100)
    {
        return dot_product_int_simd(a, b, length);
    }
    else
    {
        return dot_product_int_unrolled(a, b, length);
    }
}

// 自动选择最佳方法计算浮点向量点积
double dot_product_double_optimal(double *a, double *b, int length)
{
    // 根据向量长度选择最佳算法
    if (length >= 1000)
    {
#ifdef _OPENMP
        return dot_product_double_parallel(a, b, length);
#else
        return dot_product_double_simd(a, b, length);
#endif
    }
    else if (length >= 100)
    {
        return dot_product_double_simd(a, b, length);
    }
    else
    {
        return dot_product_double_unrolled(a, b, length);
    }
}

// 添加解析辅助函数，处理包含方括号和逗号的输入
int read_vector_element(int *value)
{
    char c;
    int sign = 1;
    *value = 0;
    int have_digit = 0;

    // 跳过空白字符和方括号
    while ((c = getchar()) != EOF)
    {
        if (c == '-')
        {
            sign = -1;
            break;
        }
        else if (c >= '0' && c <= '9')
        {
            *value = c - '0';
            have_digit = 1;
            break;
        }
        else if (c == '[' || c == ',' || c == ' ' || c == '\n' || c == '\t')
        {
            continue;
        }
        else if (c == ']')
        {
            // 遇到结束方括号，继续处理下一个字符
            continue;
        }
        else
        {
            return 0; // 未知字符
        }
    }

    if (!have_digit && c == '-')
    {
        // 读取负数的数字部分
        if ((c = getchar()) == EOF || c < '0' || c > '9')
        {
            return 0;
        }
        *value = -(c - '0');
        have_digit = 1;
    }

    // 读取数字的剩余部分
    while ((c = getchar()) != EOF && c >= '0' && c <= '9')
    {
        *value = *value * 10 + (c - '0');
        have_digit = 1;
    }

    // 如果是负数且已经处理了数字部分
    if (sign == -1 && have_digit && *value > 0)
    {
        *value = -*value;
    }

    // 如果下一个字符是逗号或方括号，则表示成功读取了一个数字
    if (have_digit && (c == ',' || c == ']' || c == ' ' || c == '\n' || c == '\t'))
    {
        return 1;
    }

    return have_digit;
}

// 类似函数用于读取double类型
int read_vector_element_double(double *value)
{
    char c;
    int sign = 1;
    double integer_part = 0;
    double decimal_part = 0;
    double decimal_factor = 0.1;
    int have_digit = 0;
    int decimal_mode = 0;

    // 跳过空白字符和方括号
    while ((c = getchar()) != EOF)
    {
        if (c == '-')
        {
            sign = -1;
            break;
        }
        else if (c >= '0' && c <= '9')
        {
            integer_part = c - '0';
            have_digit = 1;
            break;
        }
        else if (c == '[' || c == ',' || c == ' ' || c == '\n' || c == '\t')
        {
            continue;
        }
        else if (c == ']')
        {
            continue;
        }
        else
        {
            return 0;
        }
    }

    if (!have_digit && c == '-')
    {
        if ((c = getchar()) == EOF || c < '0' || c > '9')
        {
            return 0;
        }
        integer_part = -(c - '0');
        have_digit = 1;
    }

    // 读取整数部分
    while ((c = getchar()) != EOF)
    {
        if (c >= '0' && c <= '9')
        {
            integer_part = integer_part * 10 + (c - '0');
            have_digit = 1;
        }
        else if (c == '.')
        {
            decimal_mode = 1;
            break;
        }
        else
        {
            break;
        }
    }

    // 读取小数部分
    if (decimal_mode)
    {
        while ((c = getchar()) != EOF && c >= '0' && c <= '9')
        {
            decimal_part += (c - '0') * decimal_factor;
            decimal_factor *= 0.1;
            have_digit = 1;
        }
    }

    *value = sign * (integer_part + decimal_part);

    if (have_digit && (c == ',' || c == ']' || c == ' ' || c == '\n' || c == '\t'))
    {
        return 1;
    }

    return have_digit;
}

int main()
{
    int num_cases, vector_length;
    char data_type[10];

    // 添加调试输出
    fprintf(stderr, "启动优化版向量点积计算...\n");

    // 读取测试用例数和数据类型
    if (scanf("%d %s", &num_cases, data_type) != 2)
    {
        fprintf(stderr, "输入错误: 无法读取测试用例数量和数据类型\n");
        return 1;
    }

    // 调试输出
    fprintf(stderr, "读取到: %d 个测试用例, 数据类型 = %s\n", num_cases, data_type);

    // 使用动态内存分配，并确保内存对齐
    int *int_vector1 = NULL, *int_vector2 = NULL;
    double *double_vector1 = NULL, *double_vector2 = NULL;

    // 为数据类型分配内存
    if (strcmp(data_type, "int") == 0)
    {
        // 使用malloc替代posix_memalign以排除潜在问题
        int_vector1 = (int *)malloc(MAX_VECTOR_SIZE * sizeof(int));
        int_vector2 = (int *)malloc(MAX_VECTOR_SIZE * sizeof(int));

        if (int_vector1 == NULL || int_vector2 == NULL)
        {
            fprintf(stderr, "内存分配失败\n");
            return 1;
        }
    }
    else if (strcmp(data_type, "double") == 0)
    {
        // 使用malloc替代posix_memalign以排除潜在问题
        double_vector1 = (double *)malloc(MAX_VECTOR_SIZE * sizeof(double));
        double_vector2 = (double *)malloc(MAX_VECTOR_SIZE * sizeof(double));

        if (double_vector1 == NULL || double_vector2 == NULL)
        {
            fprintf(stderr, "内存分配失败\n");
            return 1;
        }
    }
    else
    {
        fprintf(stderr, "不支持的数据类型: %s\n", data_type);
        return 1;
    }

    // 初始化计时器
    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    // 处理每个测试用例
    for (int i = 0; i < num_cases; i++)
    {
        // 读取向量长度
        if (scanf("%d", &vector_length) != 1)
        {
            fprintf(stderr, "输入错误: 无法读取第 %d 个测试用例的向量长度\n", i + 1);
            return 1;
        }

        // 调试输出
        if (i < 5 || i == num_cases - 1)
        {
            fprintf(stderr, "测试用例 %d: 向量长度 = %d\n", i + 1, vector_length);
        }

        if (vector_length > MAX_VECTOR_SIZE)
        {
            fprintf(stderr, "错误: 向量长度超过最大限制\n");
            return 1;
        }

        // 根据数据类型读取数据
        if (strcmp(data_type, "int") == 0)
        {
            // 读取第一个向量的所有元素
            for (int j = 0; j < vector_length; j++)
            {
                if (!read_vector_element(&int_vector1[j]))
                {
                    fprintf(stderr, "输入错误: 无法读取测试用例 %d 的向量1的元素 %d\n", i + 1, j);
                    return 1;
                }
            }

            // 读取第二个向量的所有元素
            for (int j = 0; j < vector_length; j++)
            {
                if (!read_vector_element(&int_vector2[j]))
                {
                    fprintf(stderr, "输入错误: 无法读取测试用例 %d 的向量2的元素 %d\n", i + 1, j);
                    return 1;
                }
            }

            // 计算并输出点积
            long long result = dot_product_int_optimal(int_vector1, int_vector2, vector_length);
            printf("Int dot product result: %lld\n", result);
        }
        else if (strcmp(data_type, "double") == 0)
        {
            // 读取向量数据
            for (int j = 0; j < vector_length; j++)
            {
                if (!read_vector_element_double(&double_vector1[j]))
                {
                    fprintf(stderr, "输入错误: 无法读取测试用例 %d 的向量1的元素 %d\n", i + 1, j);
                    return 1;
                }
            }

            for (int j = 0; j < vector_length; j++)
            {
                if (!read_vector_element_double(&double_vector2[j]))
                {
                    fprintf(stderr, "输入错误: 无法读取测试用例 %d 的向量2的元素 %d\n", i + 1, j);
                    return 1;
                }
            }

            // 计算并输出点积
            double result = dot_product_double_optimal(double_vector1, double_vector2, vector_length);
            printf("Double dot product result: %.2f\n", result);
        }
    }

    // 计算并输出总执行时间
    gettimeofday(&end_time, NULL);
    long execution_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 +
                          (end_time.tv_usec - start_time.tv_usec);
    printf("Program execution time: %ld microseconds\n", execution_time);

    // 释放内存
    if (int_vector1)
        free(int_vector1);
    if (int_vector2)
        free(int_vector2);
    if (double_vector1)
        free(double_vector1);
    if (double_vector2)
        free(double_vector2);

    return 0;
}
