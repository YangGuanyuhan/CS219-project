#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
// 定义错误代码
#define DOT_PRODUCT_SUCCESS 0
#define DOT_PRODUCT_NULL_PTR_ERROR 1
#define DOT_PRODUCT_INVALID_SIZE_ERROR 2

// 函数声明
// int类型向量点乘函数
int dotProductInt(const int *vecA, const int *vecB, int size, int *result);
// long long类型向量点乘函数
int dotProductLongLong(const long long *vecA, const long long *vecB, int size, long long *result);
// short类型向量点乘函数
int dotProductShort(const short *vecA, const short *vecB, int size, short *result);
// char类型向量点乘函数
int dotProductChar(const char *vecA, const char *vecB, int size, char *result);
// float类型向量点乘函数
int dotProductFloat(const float *vecA, const float *vecB, int size, float *result);
// double类型向量点乘函数
int dotProductDouble(const double *vecA, const double *vecB, int size, double *result);
// 错误打印函数
void printError(int errorCode);

// 函数实现
// int类型向量点乘函数
int dotProductInt(const int *vecA, const int *vecB, int size, int *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// float类型向量点乘函数
int dotProductFloat(const float *vecA, const float *vecB, int size, float *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0.0f;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// double类型向量点乘函数
int dotProductDouble(const double *vecA, const double *vecB, int size, double *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0.0;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// long long类型向量点乘函数
int dotProductLongLong(const long long *vecA, const long long *vecB, int size, long long *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// short类型向量点乘函数
int dotProductShort(const short *vecA, const short *vecB, int size, short *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// char类型向量点乘函数
int dotProductChar(const char *vecA, const char *vecB, int size, char *result)
{
    // 错误检查
    if (vecA == NULL || vecB == NULL || result == NULL)
    {
        return DOT_PRODUCT_NULL_PTR_ERROR;
    }

    if (size <= 0)
    {
        return DOT_PRODUCT_INVALID_SIZE_ERROR;
    }

    // 计算点乘
    *result = 0;
    for (int i = 0; i < size; i++)
    {
        *result += vecA[i] * vecB[i];
    }

    return DOT_PRODUCT_SUCCESS;
}

// 错误打印函数
void printError(int errorCode)
{
    switch (errorCode)
    {
    case DOT_PRODUCT_NULL_PTR_ERROR:
        printf("Error: Null pointer provided\n");
        break;
    case DOT_PRODUCT_INVALID_SIZE_ERROR:
        printf("Error: Invalid vector size\n");
        break;
    default:
        printf("Unknown error: %d\n", errorCode);
    }
}

// Conditionally compile main function
#ifndef TESTING
int main()
{
    struct timeval start, end;
    long elapsed_time_us;

    gettimeofday(&start, NULL);
    int n; // 测试用例个数
    scanf("%d", &n);

    for (int testCase = 0; testCase < n; testCase++)
    {
        char dataType[10]; // 数据类型
        int size;          // 向量大小

        scanf("%s %d", dataType, &size);

        // 检查size是否有效
        if (size <= 0)
        {
            printf("Error: Invalid vector size\n");
            continue;
        }

        // 根据数据类型处理
        if (strcmp(dataType, "int") == 0)
        {
            // 为int类型分配内存
            int *vecA = (int *)malloc(size * sizeof(int));
            int *vecB = (int *)malloc(size * sizeof(int));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%d", &vecA[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%d", &vecB[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            int result;
            int status = dotProductInt(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Int dot product result: %d\n", result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else if (strcmp(dataType, "long") == 0)
        {
            // 为long long类型分配内存
            long long *vecA = (long long *)malloc(size * sizeof(long long));
            long long *vecB = (long long *)malloc(size * sizeof(long long));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%lld", &vecA[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%lld", &vecB[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            long long result;
            int status = dotProductLongLong(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Long Long dot product result: %lld\n", result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else if (strcmp(dataType, "short") == 0)
        {
            // 为short类型分配内存
            short *vecA = (short *)malloc(size * sizeof(short));
            short *vecB = (short *)malloc(size * sizeof(short));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%hd", &vecA[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%hd", &vecB[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            short result;
            int status = dotProductShort(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Short dot product result: %hd\n", result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else if (strcmp(dataType, "char") == 0)
        {
            // 为char类型分配内存
            char *vecA = (char *)malloc(size * sizeof(char));
            char *vecB = (char *)malloc(size * sizeof(char));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                // 读取整数然后转换为char
                int temp;
                scanf("%d", &temp);
                vecA[i] = (char)temp;
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                // 读取整数然后转换为char
                int temp;
                scanf("%d", &temp);
                vecB[i] = (char)temp;
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            char result;
            int status = dotProductChar(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Char dot product result: %d\n", (int)result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else if (strcmp(dataType, "float") == 0)
        {
            // 为float类型分配内存
            float *vecA = (float *)malloc(size * sizeof(float));
            float *vecB = (float *)malloc(size * sizeof(float));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%f", &vecA[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%f", &vecB[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            float result;
            int status = dotProductFloat(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Float dot product result: %f\n", result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else if (strcmp(dataType, "double") == 0)
        {
            // 为double类型分配内存
            double *vecA = (double *)malloc(size * sizeof(double));
            double *vecB = (double *)malloc(size * sizeof(double));

            if (vecA == NULL || vecB == NULL)
            {
                printf("Error: Memory allocation failed\n");
                if (vecA)
                    free(vecA);
                if (vecB)
                    free(vecB);
                continue;
            }

            // 读取向量A
            char c;
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%lf", &vecA[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 读取向量B
            scanf(" %c", &c); // 读取'['
            for (int i = 0; i < size; i++)
            {
                scanf("%lf", &vecB[i]);
                if (i < size - 1)
                {
                    scanf("%c", &c); // 读取','
                }
            }
            scanf("%c", &c); // 读取']'

            // 计算点乘
            double result;
            int status = dotProductDouble(vecA, vecB, size, &result);

            // 输出结果
            if (status == DOT_PRODUCT_SUCCESS)
            {
                printf("Double dot product result: %f\n", result);
            }
            else
            {
                printError(status);
            }

            // 释放内存
            free(vecA);
            free(vecB);
        }
        else
        {
            printf("Error: Unsupported data type: %s\n", dataType);
        }
    }

    gettimeofday(&end, NULL);
    // 以us的单位输出
    elapsed_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);

    printf("Program execution time: %ld microseconds\n", elapsed_time_us);

    return 0;
}
#endif
