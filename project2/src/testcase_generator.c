#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

// 数据类型选项
#define TYPE_INT 0
#define TYPE_LONG 1
#define TYPE_SHORT 2
#define TYPE_CHAR 3
#define TYPE_FLOAT 4
#define TYPE_DOUBLE 5
#define TYPE_RANDOM 6

// 常量定义
#define MAX_VECTOR_LENGTH 1000000
#define DEFAULT_NUM_CASES 5
#define DEFAULT_VECTOR_LENGTH 10

// 函数原型
void generate_test_cases(int num_cases, int data_type, int vector_length, int random_length, FILE *output);
void generate_vector(int data_type, int length, FILE *output);
void print_help();
const char *get_type_string(int type);
int get_random_int(int min, int max);
float get_random_float(float min, float max);
double get_random_double(double min, double max);

int main(int argc, char *argv[])
{
    // 默认参数
    int num_cases = DEFAULT_NUM_CASES;
    int data_type = TYPE_RANDOM;
    int vector_length = DEFAULT_VECTOR_LENGTH;
    int random_length = 0;
    char *output_file = NULL;
    FILE *output = stdout;

    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "n:t:l:ro:h")) != -1)
    {
        switch (opt)
        {
        case 'n':
            num_cases = atoi(optarg);
            if (num_cases <= 0)
            {
                fprintf(stderr, "Error: Number of test cases must be positive\n");
                return 1;
            }
            break;
        case 't':
            if (strcmp(optarg, "int") == 0)
            {
                data_type = TYPE_INT;
            }
            else if (strcmp(optarg, "long") == 0)
            {
                data_type = TYPE_LONG;
            }
            else if (strcmp(optarg, "short") == 0)
            {
                data_type = TYPE_SHORT;
            }
            else if (strcmp(optarg, "char") == 0)
            {
                data_type = TYPE_CHAR;
            }
            else if (strcmp(optarg, "float") == 0)
            {
                data_type = TYPE_FLOAT;
            }
            else if (strcmp(optarg, "double") == 0)
            {
                data_type = TYPE_DOUBLE;
            }
            else if (strcmp(optarg, "random") == 0)
            {
                data_type = TYPE_RANDOM;
            }
            else
            {
                fprintf(stderr, "Error: Unknown data type '%s'\n", optarg);
                print_help();
                return 1;
            }
            break;
        case 'l':
            vector_length = atoi(optarg);
            if (vector_length <= 0 || vector_length > MAX_VECTOR_LENGTH)
            {
                fprintf(stderr, "Error: Vector length must be between 1 and %d\n", MAX_VECTOR_LENGTH);
                return 1;
            }
            break;
        case 'r':
            random_length = 1;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'h':
            print_help();
            return 0;
        default:
            print_help();
            return 1;
        }
    }

    // 初始化随机数生成器
    srand(time(NULL));

    // 打开输出文件（如果指定）
    if (output_file != NULL)
    {
        output = fopen(output_file, "w");
        if (output == NULL)
        {
            fprintf(stderr, "Error: Could not open output file '%s'\n", output_file);
            return 1;
        }
    }

    // 生成测试用例
    generate_test_cases(num_cases, data_type, vector_length, random_length, output);

    // 关闭输出文件（如果有）
    if (output_file != NULL)
    {
        fclose(output);
        printf("Test cases written to '%s'\n", output_file);
    }

    return 0;
}

void generate_test_cases(int num_cases, int data_type, int vector_length, int random_length, FILE *output)
{
    // 首先输出测试用例数量
    fprintf(output, "%d\n", num_cases);

    for (int i = 0; i < num_cases; i++)
    {
        // 为当前测试用例选择数据类型
        int current_type = data_type;
        if (current_type == TYPE_RANDOM)
        {
            current_type = get_random_int(TYPE_INT, TYPE_DOUBLE);
        }

        // 为当前测试用例选择向量长度
        int current_length = vector_length;
        if (random_length)
        {
            current_length = get_random_int(1, MAX_VECTOR_LENGTH);
        }

        // 输出数据类型和向量长度
        fprintf(output, "%s %d\n", get_type_string(current_type), current_length);

        // 生成第一个向量
        generate_vector(current_type, current_length, output);
        fprintf(output, " ");

        // 生成第二个向量
        generate_vector(current_type, current_length, output);
        fprintf(output, "\n");
    }
}

void generate_vector(int data_type, int length, FILE *output)
{
    fprintf(output, "[");

    for (int i = 0; i < length; i++)
    {
        switch (data_type)
        {
        case TYPE_INT:
            fprintf(output, "%d", get_random_int(-100, 100));
            break;
        case TYPE_LONG:
            fprintf(output, "%lld", (long long)get_random_int(-1000000, 1000000));
            break;
        case TYPE_SHORT:
            fprintf(output, "%d", get_random_int(-100, 100));
            break;
        case TYPE_CHAR:
            fprintf(output, "%d", get_random_int(0, 100));
            break;
        case TYPE_FLOAT:
            fprintf(output, "%f", get_random_float(-100.0, 100.0));
            break;
        case TYPE_DOUBLE:
            fprintf(output, "%lf", get_random_double(-100.0, 100.0));
            break;
        }

        if (i < length - 1)
        {
            fprintf(output, ",");
        }
    }

    fprintf(output, "]");
}

const char *get_type_string(int type)
{
    switch (type)
    {
    case TYPE_INT:
        return "int";
    case TYPE_LONG:
        return "long";
    case TYPE_SHORT:
        return "short";
    case TYPE_CHAR:
        return "char";
    case TYPE_FLOAT:
        return "float";
    case TYPE_DOUBLE:
        return "double";
    default:
        return "unknown";
    }
}

int get_random_int(int min, int max)
{
    return min + rand() % (max - min + 1);
}

float get_random_float(float min, float max)
{
    float scale = rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

double get_random_double(double min, double max)
{
    double scale = rand() / (double)RAND_MAX;
    return min + scale * (max - min);
}

void print_help()
{
    printf("Test Case Generator for Vector Dot Product\n");
    printf("Usage: testcase_generator [options]\n\n");
    printf("Options:\n");
    printf("  -n <num>      Number of test cases to generate (default: %d)\n", DEFAULT_NUM_CASES);
    printf("  -t <type>     Data type (int/long/short/char/float/double/random, default: random)\n");
    printf("  -l <length>   Vector length (default: %d)\n", DEFAULT_VECTOR_LENGTH);
    printf("  -r            Use random vector lengths\n");
    printf("  -o <file>     Output to file instead of stdout\n");
    printf("  -h            Show this help message\n");
    printf("\nExamples:\n");
    printf("  ./testcase_generator -n 10 -t int -l 5           # Generate 10 int test cases with vector length 5\n");
    printf("  ./testcase_generator -n 20 -t random -r -o test.txt  # Generate 20 random type test cases with random lengths\n");
}
