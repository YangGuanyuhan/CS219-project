#include "../inc/calculator.h"

int main(int argc, char *argv[])
{
    if (argc == 4)
    {
        process_command_line(argc, argv);
    }
    else if (argc == 1)
    {
        interactive_mode();
    }
    else if (argc == 2 && strcmp(argv[1], "sqrt") == 0)
    {
        interactive_mode();
    }
    else if (argc == 2 && strncmp(argv[1], "sqrt(", 5) == 0)
    {
        handle_sqrt(argv[1]);
    }
    else if (argc == 2 && strcmp(argv[1], "quit") == 0)
    {
        return 0;
    }
    else
    {
        fprintf(stderr, "usage: %s [number1 operator number2]\n", argv[0]);
        return 1;
    }
    return 0;
}

void process_command_line(int argc, char *argv[])
{
    (void)argc;
    Number num1, num2;
    char op = argv[2][0];

    if (parse_number(argv[1], &num1) != 0)
    {
        printf("The input cannot be interpret as numbers!\n");
        return;
    }

    if (parse_number(argv[3], &num2) != 0)
    {
        printf("The input cannot be interpret as numbers!\n");
        return;
    }

    printf("%s %c %s = ", argv[1], op, argv[3]);

    handle_operation(num1, op, num2);
}

int parse_number(const char *str, Number *num)
{
    char *endptr;
    // 检查是否是科学计数法
    if (is_scientific_notation(str))
    {
        // 找到e的位置
        const char *e_pos = strchr(str, 'e');
        if (!e_pos)
        {
            e_pos = strchr(str, 'E');
        }

        char mantissa_str[128];
        int mantissa_len = e_pos - str; // 计算尾数的长度
        if ((size_t)mantissa_len >= sizeof(mantissa_str))
        {
            return -1; // 尾数过长
        }
        // 复制尾数部分
        strncpy(mantissa_str, str, mantissa_len);
        mantissa_str[mantissa_len] = '\0';

        int exponent = atoi(e_pos + 1);
        double mantissa = strtod(mantissa_str, NULL);

        num->type = SCIENCE;
        num->sci.mantissa = mantissa;
        num->sci.exponent = exponent;

        return 0;
    }
    // 处理整数和浮点数
    //  检查是否是整数

    errno = 0;
    long long int_val = strtoll(str, &endptr, 10);
    if (errno != ERANGE && *endptr == '\0')
    {
        num->type = INT;
        num->int_val = int_val;
        return 0;
    }

    errno = 0;
    double float_val = strtod(str, &endptr);
    if (errno != ERANGE && *endptr == '\0')
    {
        num->type = FLOAT;
        num->float_val = float_val;
        return 0;
    }

    return -1;
}

void handle_operation(Number num1, char op, Number num2)
{
    Number result;

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

    if (num1.type == SCIENCE || num2.type == SCIENCE ||
        (op == '*' && ((num1.type == FLOAT && (fabs(num1.float_val) > 1e100 || fabs(num1.float_val) < 1e-100)) ||
                       (num2.type == FLOAT && (fabs(num2.float_val) > 1e100 || fabs(num2.float_val) < 1e-100)))))
    { // 转换为科学计数法
        if (num1.type != SCIENCE)
        {
            convert_to_scientific(&num1);
        }
        if (num2.type != SCIENCE)
        {
            convert_to_scientific(&num2);
        }

        result.type = SCIENCE;
        // 进行科学计数法的加减运算
        switch (op)
        {
        case '+':
        case '-':
        {
            double a = num1.sci.mantissa;
            double b = num2.sci.mantissa;
            int exp_a = num1.sci.exponent;
            int exp_b = num2.sci.exponent;

            // 对齐指数

            if (exp_a > exp_b)
            {
                while (exp_a > exp_b)
                {
                    b /= 10.0;
                    exp_b++;
                }
            }
            else if (exp_b > exp_a)
            {
                while (exp_b > exp_a)
                {
                    a /= 10.0;
                    exp_a++;
                }
            }
            // 进行加减运算

            double res = (op == '+') ? a + b : a - b;

            result.sci.mantissa = res;
            result.sci.exponent = exp_a;
            // 规范，使得尾数在1到10之间
            if (result.sci.mantissa != 0)
            {
                while (fabs(result.sci.mantissa) >= 10.0)
                {
                    result.sci.mantissa /= 10.0;
                    result.sci.exponent++;
                }
                while (fabs(result.sci.mantissa) < 1.0)
                {
                    result.sci.mantissa *= 10.0;
                    result.sci.exponent--;
                }
            }
            break;
        }

        case 'x':
        case '*':
            result.sci.mantissa = num1.sci.mantissa * num2.sci.mantissa;
            result.sci.exponent = num1.sci.exponent + num2.sci.exponent;

            if (result.sci.mantissa != 0)
            {
                while (fabs(result.sci.mantissa) >= 10.0)
                {
                    result.sci.mantissa /= 10.0;
                    result.sci.exponent++;
                }
                while (fabs(result.sci.mantissa) < 1.0)
                {
                    result.sci.mantissa *= 10.0;
                    result.sci.exponent--;
                }
            }
            break;

        case '/':
            result.sci.mantissa = num1.sci.mantissa / num2.sci.mantissa;
            result.sci.exponent = num1.sci.exponent - num2.sci.exponent;

            if (result.sci.mantissa != 0)
            {
                while (fabs(result.sci.mantissa) >= 10.0)
                {
                    result.sci.mantissa /= 10.0;
                    result.sci.exponent++;
                }
                while (fabs(result.sci.mantissa) < 1.0)
                {
                    result.sci.mantissa *= 10.0;
                    result.sci.exponent--;
                }
            }
            break;

        default:
            printf("invalid opretor\n");
            return;
        }

        print_result(result);
        return;
    }

    switch (op)
    {
    case '+':
        if (num1.type == INT && num2.type == INT)
        {
            result.type = INT;
            result.int_val = num1.int_val + num2.int_val;
        }
        else
        {
            result.type = FLOAT;
            result.float_val = (num1.type == INT ? num1.int_val : num1.float_val) +
                               (num2.type == INT ? num2.int_val : num2.float_val);
        }
        break;

    case '-':
        if (num1.type == INT && num2.type == INT)
        {
            result.type = INT;
            result.int_val = num1.int_val - num2.int_val;
        }
        else
        {
            result.type = FLOAT;
            result.float_val = (num1.type == INT ? num1.int_val : num1.float_val) -
                               (num2.type == INT ? num2.int_val : num2.float_val);
        }
        break;

    case 'x':
    case '*':
        if (num1.type == INT && num2.type == INT)
        {
            if (is_mult_overflow(num1.int_val, num2.int_val))
            {
                result.type = FLOAT;
                result.float_val = (double)num1.int_val * num2.int_val;
            }
            else
            {
                result.type = INT;
                result.int_val = num1.int_val * num2.int_val;
            }
        }
        else
        {
            result.type = FLOAT;
            result.float_val = (num1.type == INT ? num1.int_val : num1.float_val) *
                               (num2.type == INT ? num2.int_val : num2.float_val);
        }
        break;

    case '/':
        result.type = FLOAT;
        result.float_val = (num1.type == INT ? num1.int_val : num1.float_val) /
                           (num2.type == INT ? num2.int_val : num2.float_val);
        break;

    default:
        printf("invalid oprerator\n");
        return;
    }

    if (result.type == FLOAT && !isfinite(result.float_val))
    {
        convert_to_scientific(&num1);
        convert_to_scientific(&num2);
        handle_operation(num1, op, num2);
        return;
    }

    print_result(result);
}

int is_mult_overflow(long long a, long long b)
{
    if (a > 0)
    {
        if (b > 0)
        {
            if (a > LLONG_MAX / b)
                return 1;
        }
        else
        {
            if (b < LLONG_MIN / a)
                return 1;
        }
    }
    else
    {
        if (b > 0)
        {
            if (a < LLONG_MIN / b)
                return 1;
        }
        else
        {
            if (a != 0 && b < LLONG_MAX / a)
                return 1;
        }
    }
    return 0;
}

int is_scientific_notation(const char *str)
{
    const char *e_pos = strchr(str, 'e');
    if (!e_pos)
    {
        e_pos = strchr(str, 'E');
    }

    if (!e_pos)
    {
        return 0;
    }

    char *endptr;
    strtod(str, &endptr);

    if (endptr != e_pos)
    {
        return 0;
    }

    strtol(e_pos + 1, &endptr, 10);
    if (*endptr != '\0')
    {
        return 0;
    }

    return 1;
}

void convert_to_scientific(Number *num)
{
    if (num->type == SCIENCE)
    {
        return;
    }

    double value = (num->type == INT) ? (double)num->int_val : num->float_val;

    int exponent = 0;
    double mantissa = value;

    if (mantissa != 0)
    {
        while (fabs(mantissa) >= 10.0)
        {
            mantissa /= 10.0;
            exponent++;
        }
        while (fabs(mantissa) < 1.0)
        {
            mantissa *= 10.0;
            exponent--;
        }
    }

    num->type = SCIENCE;
    num->sci.mantissa = mantissa;
    num->sci.exponent = exponent;
}

void print_result(Number result)
{
    if (result.type == INT)
    {
        printf("%lld\n", result.int_val);
    }
    else if (result.type == FLOAT)
    {
        double val = result.float_val;
        if (val == (long long)val)
        {
            printf("%lld\n", (long long)val);
        }
        else
        {
            printf("%.8f\n", val);
        }
    }
    else if (result.type == SCIENCE)
    {
        if (result.sci.exponent > -50 && result.sci.exponent < 50)
        {
            double val = result.sci.mantissa * pow(10, result.sci.exponent);
            if (val == (long long)val)
            {
                printf("%lld\n", (long long)val);
            }
            else
            {
                printf("%.8f\n", val);
            }
        }
        else
        {
            printf("%.8fe%d\n", result.sci.mantissa, result.sci.exponent);
        }
    }
}

void handle_sqrt(const char *input)
{
    const char *start = input + 5;
    char *end;

    end = strchr(start, ')');
    if (!end)
    {
        printf("Invalid syntax for the square root function.\n");
        return;
    }

    int len = end - start;
    char *num_str = (char *)malloc(len + 1);
    strncpy(num_str, start, len);
    num_str[len] = '\0';

    Number num;
    if (parse_number(num_str, &num) != 0)
    {
        printf("The parameter for the sqrt function cannot be parsed as a number.\n");
        free(num_str);
        return;
    }
    free(num_str);

    double value;

    if (num.type == SCIENCE)
    {
        if (num.sci.exponent % 2 == 0)
        {
            value = sqrt(num.sci.mantissa);

            Number result;
            result.type = SCIENCE;
            result.sci.mantissa = value;
            result.sci.exponent = num.sci.exponent / 2;

            if (result.sci.mantissa != 0)
            {
                while (fabs(result.sci.mantissa) >= 10.0)
                {
                    result.sci.mantissa /= 10.0;
                    result.sci.exponent++;
                }
                while (fabs(result.sci.mantissa) < 1.0)
                {
                    result.sci.mantissa *= 10.0;
                    result.sci.exponent--;
                }
            }

            print_result(result);
            return;
        }
        else
        {
            double mantissa = num.sci.mantissa * 10;
            int exponent = num.sci.exponent - 1;

            value = sqrt(mantissa);

            Number result;
            result.type = SCIENCE;
            result.sci.mantissa = value;
            result.sci.exponent = exponent / 2;

            if (result.sci.mantissa != 0)
            {
                while (fabs(result.sci.mantissa) >= 10.0)
                {
                    result.sci.mantissa /= 10.0;
                    result.sci.exponent++;
                }
                while (fabs(result.sci.mantissa) < 1.0)
                {
                    result.sci.mantissa *= 10.0;
                    result.sci.exponent--;
                }
            }

            print_result(result);
            return;
        }
    }
    else
    {
        value = num.type == INT ? (double)num.int_val : num.float_val;
    }

    if (value < 0)
    {
        printf("The square root of a negative number cannot be calculated.\n");
        return;
    }

    if (value > 1e100)
    {
        convert_to_scientific(&num);
        handle_sqrt(input);
        return;
    }

    Number result;
    result.type = FLOAT;
    result.float_val = sqrt(value);
    print_result(result);
}

void interactive_mode(void)
{
    char input[256];
    printf("Welcome to the calculator! Enter 'quit' to exit.\n");
    printf("Supported operations: +, -, x(*), /, sqrt()\n");

    while (1)
    {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin))
        {
            break;
        }
        // 去掉换行符
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0)
        {
            break;
        }

        if (strncmp(input, "sqrt(", 5) == 0)
        {
            handle_sqrt(input);
            continue;
        }

        char op;
        char num1_str[128], num2_str[128];
        int matched = sscanf(input, "%s %c %s", num1_str, &op, num2_str);

        if (matched != 3)
        {
            printf("Invalid expression formatting\n");
            continue;
        }

        Number num1, num2;
        if (parse_number(num1_str, &num1) != 0 || parse_number(num2_str, &num2) != 0)
        {
            printf("Inputs can't be parsed to numbers!\n");
            continue;
        }

        handle_operation(num1, op, num2);
    }
}
