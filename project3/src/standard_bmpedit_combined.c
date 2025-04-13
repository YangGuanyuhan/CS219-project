/**
 * @file standard_bmpedit_combined.c
 * @brief BMP图像处理组合源文件
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/time.h> /* 添加用于gettimeofday()的头文件 */

/**
 * @brief BMP文件头结构
 * 注意：使用打包结构避免内存对齐问题
 */
#pragma pack(push, 1)
typedef struct
{
    uint16_t type;      /* 'BM' - BMP文件标识符 */
    uint32_t size;      /* 文件大小 */
    uint16_t reserved1; /* 保留字段1 */
    uint16_t reserved2; /* 保留字段2 */
    uint32_t offset;    /* 像素数据偏移量 */
} BMPFileHeader;

/**
 * @brief BMP信息头结构
 */
typedef struct
{
    uint32_t size;              /* 信息头大小 */
    int32_t width;              /* 图像宽度 */
    int32_t height;             /* 图像高度 */
    uint16_t planes;            /* 颜色平面数，必须为1 */
    uint16_t bit_count;         /* 每像素位数，本程序仅处理24位 */
    uint32_t compression;       /* 压缩方法，本程序仅处理0（不压缩） */
    uint32_t image_size;        /* 图像数据大小 */
    int32_t x_pixels_per_meter; /* 水平分辨率 */
    int32_t y_pixels_per_meter; /* 垂直分辨率 */
    uint32_t colors_used;       /* 使用的颜色数 */
    uint32_t colors_important;  /* 重要颜色数 */
} BMPInfoHeader;
#pragma pack(pop)

/**
 * @brief BMP图像结构
 */
typedef struct
{
    BMPFileHeader file_header; /* 文件头 */
    BMPInfoHeader info_header; /* 信息头 */
    uint8_t *pixels;           /* 像素数据，BGR格式，每像素3字节 */
} BMPImage;

/**
 * @brief 计时器结构体，用于测量代码执行时间
 */
typedef struct
{
    struct timeval start;
    struct timeval end;
    double elapsed;
} TimerData;

/**
 * @brief 初始化计时器
 * @param timer 计时器结构体指针
 */
void timer_init(TimerData *timer)
{
    if (timer)
    {
        memset(timer, 0, sizeof(TimerData));
    }
}

/**
 * @brief 开始计时
 * @param timer 计时器结构体指针
 */
void timer_start(TimerData *timer)
{
    if (timer)
    {
        gettimeofday(&timer->start, NULL);
    }
}

/**
 * @brief 结束计时
 * @param timer 计时器结构体指针
 */
void timer_stop(TimerData *timer)
{
    if (timer)
    {
        gettimeofday(&timer->end, NULL);
        timer->elapsed = (timer->end.tv_sec - timer->start.tv_sec) * 1000.0 +
                         (timer->end.tv_usec - timer->start.tv_usec) / 1000.0;
    }
}

/**
 * @brief 获取计时器记录的时间，单位为毫秒
 * @param timer 计时器结构体指针
 * @return 执行时间（毫秒）
 */
double timer_elapsed_ms(const TimerData *timer)
{
    if (timer)
    {
        return timer->elapsed;
    }
    return 0.0;
}

/**
 * @brief 打印计时器记录的时间
 * @param prefix 前缀说明文字
 * @param timer 计时器结构体指针
 */
void timer_print(const char *prefix, const TimerData *timer)
{
    if (timer)
    {
        printf("%s: %.3f 毫秒\n", prefix, timer_elapsed_ms(timer));
    }
}

/**
 * @brief 打印错误信息
 * @param message 错误信息
 */
void print_error(const char *message)
{
    if (message != NULL)
    {
        fprintf(stderr, "Error: %s\n", message);
    }
}

/**
 * @brief 获取一行的字节数（包括填充字节）
 * @param width 图像宽度
 * @return 一行的字节数
 */
uint32_t bmp_get_row_stride(int width)
{
    /* 每行必须是4字节的倍数，每像素3字节（BGR） */
    return ((width * 3 + 3) / 4) * 4;
}

/**
 * @brief 检查BMP图像是否有效（24位未压缩格式）
 * @param image BMP图像指针
 * @return 如果有效返回1，如果无效返回0
 */
int bmp_is_valid(const BMPImage *image)
{
    /* 检查参数有效性 */
    if (image == NULL)
    {
        print_error("Invalid BMP pointer");
        return 0;
    }

    /* 检查BMP标识符'BM' */
    if (image->file_header.type != 0x4D42)
    {
        print_error("Not a valid BMP file identifier");
        return 0;
    }

    /* 检查位深度是24位 */
    if (image->info_header.bit_count != 24)
    {
        print_error("Only 24-bit BMP format is supported");
        return 0;
    }

    /* 检查是未压缩的 */
    if (image->info_header.compression != 0)
    {
        print_error("Only uncompressed BMP format is supported");
        return 0;
    }

    /* 检查其他必要条件 */
    if (image->info_header.width <= 0 || image->info_header.height == 0)
    {
        print_error("Invalid image dimensions");
        return 0;
    }

    if (image->pixels == NULL)
    {
        print_error("Invalid pixel data pointer");
        return 0;
    }

    return 1;
}

/**
 * @brief 从文件读取BMP图像
 * @param filename 文件名
 * @return 成功返回BMPImage指针，失败返回NULL
 */
BMPImage *bmp_read(const char *filename)
{
    FILE *fp = NULL;
    BMPImage *image = NULL;
    size_t read_size;
    uint32_t row_stride;
    int row, col;
    uint8_t *row_buffer = NULL;

    /* 检查参数有效性 */
    if (filename == NULL)
    {
        print_error("Filename is NULL");
        return NULL;
    }

    /* 打开文件 */
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        print_error("Cannot open file");
        return NULL;
    }

    /* 分配BMP图像结构内存 */
    image = (BMPImage *)malloc(sizeof(BMPImage));
    if (image == NULL)
    {
        print_error("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    /* 初始化为零 */
    memset(image, 0, sizeof(BMPImage));

    /* 读取文件头 */
    read_size = fread(&image->file_header, sizeof(BMPFileHeader), 1, fp);
    if (read_size != 1)
    {
        print_error("Failed to read file header");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 读取信息头 */
    read_size = fread(&image->info_header, sizeof(BMPInfoHeader), 1, fp);
    if (read_size != 1)
    {
        print_error("Failed to read info header");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 验证BMP格式 */
    if (image->file_header.type != 0x4D42)
    {
        print_error("Not a valid BMP file");
        free(image);
        fclose(fp);
        return NULL;
    }

    if (image->info_header.bit_count != 24)
    {
        print_error("Only 24-bit BMP images are supported");
        free(image);
        fclose(fp);
        return NULL;
    }

    if (image->info_header.compression != 0)
    {
        print_error("Only uncompressed BMP images are supported");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 计算行跨度，包括填充字节 */
    row_stride = bmp_get_row_stride(image->info_header.width);

    /* 分配像素数据内存 */
    image->pixels = (uint8_t *)malloc(image->info_header.width * abs(image->info_header.height) * 3);
    if (image->pixels == NULL)
    {
        print_error("Memory allocation failed");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 分配行缓冲区 */
    row_buffer = (uint8_t *)malloc(row_stride);
    if (row_buffer == NULL)
    {
        print_error("Memory allocation failed");
        free(image->pixels);
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 定位到像素数据开始位置 */
    if (fseek(fp, image->file_header.offset, SEEK_SET) != 0)
    {
        print_error("File seek failed");
        free(row_buffer);
        free(image->pixels);
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 读取像素数据 */
    for (row = 0; row < abs(image->info_header.height); row++)
    {
        /* 确定当前行索引（BMP从下到上存储行） */
        int current_row = (image->info_header.height > 0) ? (abs(image->info_header.height) - 1 - row) : row;

        /* 读取一行（包括填充字节） */
        read_size = fread(row_buffer, 1, row_stride, fp);
        if (read_size != row_stride)
        {
            print_error("Failed to read pixel data");
            free(row_buffer);
            free(image->pixels);
            free(image);
            fclose(fp);
            return NULL;
        }

        /* 复制像素数据到图像结构（不包括填充字节） */
        for (col = 0; col < image->info_header.width; col++)
        {
            int pixel_index = (current_row * image->info_header.width + col) * 3;
            image->pixels[pixel_index] = row_buffer[col * 3];         /* B */
            image->pixels[pixel_index + 1] = row_buffer[col * 3 + 1]; /* G */
            image->pixels[pixel_index + 2] = row_buffer[col * 3 + 2]; /* R */
        }
    }

    /* 释放资源并返回 */
    free(row_buffer);
    fclose(fp);
    return image;
}

/**
 * @brief 将BMP图像写入文件
 * @param image BMP图像指针
 * @param filename 文件名
 * @return 成功返回0，失败返回非0值
 */
int bmp_write(const BMPImage *image, const char *filename)
{
    FILE *fp = NULL;
    uint32_t row_stride;
    int row_padding;
    int row, col;
    uint8_t *row_buffer = NULL;
    uint8_t padding_bytes[3] = {0, 0, 0}; /* 用于行填充的字节 */

    /* 检查参数有效性 */
    if (image == NULL || filename == NULL)
    {
        print_error("Invalid parameters");
        return -1;
    }

    if (!bmp_is_valid(image))
    {
        return -1;
    }

    /* 打开文件 */
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        print_error("Cannot create file");
        return -1;
    }

    /* 计算行跨度，包括填充字节 */
    row_stride = bmp_get_row_stride(image->info_header.width);
    row_padding = row_stride - image->info_header.width * 3;

    /* 写入文件头 */
    if (fwrite(&image->file_header, sizeof(BMPFileHeader), 1, fp) != 1)
    {
        print_error("Failed to write file header");
        fclose(fp);
        return -1;
    }

    /* 写入信息头 */
    if (fwrite(&image->info_header, sizeof(BMPInfoHeader), 1, fp) != 1)
    {
        print_error("Failed to write info header");
        fclose(fp);
        return -1;
    }

    /* 分配行缓冲区 */
    row_buffer = (uint8_t *)malloc(row_stride);
    if (row_buffer == NULL)
    {
        print_error("Memory allocation failed");
        fclose(fp);
        return -1;
    }

    /* 定位到像素数据开始位置 */
    if (fseek(fp, image->file_header.offset, SEEK_SET) != 0)
    {
        print_error("File seek failed");
        free(row_buffer);
        fclose(fp);
        return -1;
    }

    /* 写入像素数据 */
    for (row = 0; row < abs(image->info_header.height); row++)
    {
        /* 确定当前行索引（BMP从下到上存储行） */
        int current_row = (image->info_header.height > 0) ? (abs(image->info_header.height) - 1 - row) : row;

        /* 复制像素数据到行缓冲区 */
        for (col = 0; col < image->info_header.width; col++)
        {
            int pixel_index = (current_row * image->info_header.width + col) * 3;
            row_buffer[col * 3] = image->pixels[pixel_index];         /* B */
            row_buffer[col * 3 + 1] = image->pixels[pixel_index + 1]; /* G */
            row_buffer[col * 3 + 2] = image->pixels[pixel_index + 2]; /* R */
        }

        /* 写入一行像素数据 */
        if (fwrite(row_buffer, 1, (size_t)(image->info_header.width * 3), fp) != (size_t)(image->info_header.width * 3))
        {
            print_error("Failed to write pixel data");
            free(row_buffer);
            fclose(fp);
            return -1;
        }

        /* 写入填充字节 */
        if (row_padding > 0)
        {
            if (fwrite(padding_bytes, 1, (size_t)row_padding, fp) != (size_t)row_padding)
            {
                print_error("Failed to write padding bytes");
                free(row_buffer);
                fclose(fp);
                return -1;
            }
        }
    }

    /* 释放资源并返回 */
    free(row_buffer);
    fclose(fp);
    return 0;
}

/**
 * @brief 释放BMP图像内存
 * @param image BMP图像指针
 */
void bmp_free(BMPImage *image)
{
    if (image != NULL)
    {
        if (image->pixels != NULL)
        {
            free(image->pixels);
            image->pixels = NULL;
        }
        free(image);
    }
}

/**
 * @brief 调整BMP图像亮度
 * @param image BMP图像指针
 * @param value 亮度调整值，正值增加亮度，负值减少亮度
 */
void bmp_adjust_brightness(BMPImage *image, int value)
{
    int total_pixels;
    int i;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 计算总像素数 */
    total_pixels = image->info_header.width * abs(image->info_header.height) * 3;

    /* 调整每个像素的亮度 */
    for (i = 0; i < total_pixels; i++)
    {
        int new_value = image->pixels[i] + value;

        /* 防止溢出 */
        if (new_value > 255)
        {
            new_value = 255;
        }
        else if (new_value < 0)
        {
            new_value = 0;
        }

        image->pixels[i] = (uint8_t)new_value;
    }
}

/**
 * @brief 调整BMP图像对比度
 * @param image BMP图像指针
 * @param factor 对比度调整因子，大于1增加对比度，0-1之间降低对比度
 */
void bmp_adjust_contrast(BMPImage *image, float factor)
{
    int total_pixels;
    int i;
    float pixel_value;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    if (factor < 0)
    {
        print_error("Contrast factor must be non-negative");
        return;
    }

    /* 计算总像素数 */
    total_pixels = image->info_header.width * abs(image->info_header.height) * 3;

    /* 调整每个像素的对比度 */
    for (i = 0; i < total_pixels; i++)
    {
        /* 将像素值调整到以128为中心 */
        pixel_value = image->pixels[i] - 128;

        /* 应用对比度因子 */
        pixel_value = pixel_value * factor + 128;

        /* 防止溢出 */
        if (pixel_value > 255)
        {
            pixel_value = 255;
        }
        else if (pixel_value < 0)
        {
            pixel_value = 0;
        }

        image->pixels[i] = (uint8_t)pixel_value;
    }
}

/**
 * @brief 通过平均值混合两个BMP图像
 * @param image1 第一个BMP图像指针
 * @param image2 第二个BMP图像指针
 * @return 成功返回混合后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_average(const BMPImage *image1, const BMPImage *image2)
{
    BMPImage *result = NULL;
    int total_pixels;
    int i;

    /* 检查参数有效性 */
    if (image1 == NULL || image2 == NULL ||
        image1->pixels == NULL || image2->pixels == NULL)
    {
        print_error("Invalid BMP image pointer");
        return NULL;
    }

    /* 检查两个图像是否具有相同的尺寸 */
    if (image1->info_header.width != image2->info_header.width ||
        abs(image1->info_header.height) != abs(image2->info_header.height))
    {
        print_error("Images have different dimensions, cannot blend");
        return NULL;
    }

    /* 创建结果图像 */
    result = (BMPImage *)malloc(sizeof(BMPImage));
    if (result == NULL)
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 从第一个图像复制头信息 */
    memcpy(&result->file_header, &image1->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image1->info_header, sizeof(BMPInfoHeader));

    /* 分配像素数据内存 */
    result->pixels = (uint8_t *)malloc(image1->info_header.width * abs(image1->info_header.height) * 3);
    if (result->pixels == NULL)
    {
        print_error("Memory allocation failed");
        free(result);
        return NULL;
    }

    /* 计算总像素数 */
    total_pixels = image1->info_header.width * abs(image1->info_header.height) * 3;

    /* 混合两个图像 */
    for (i = 0; i < total_pixels; i++)
    {
        /* 简单平均 */
        result->pixels[i] = (uint8_t)(((int)image1->pixels[i] + (int)image2->pixels[i]) / 2);
    }

    return result;
}

/**
 * @brief 将BMP图像转换为灰度
 * @param image BMP图像指针
 */
void bmp_convert_grayscale(BMPImage *image)
{
    int total_pixels;
    int i;
    uint8_t gray_value;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 计算总像素数 */
    total_pixels = image->info_header.width * abs(image->info_header.height);

    /* 转换为灰度 */
    for (i = 0; i < total_pixels; i++)
    {
        int pixel_index = i * 3;
        /* 计算灰度值 - 使用加权公式：0.299*R + 0.587*G + 0.114*B */
        gray_value = (uint8_t)(0.299 * image->pixels[pixel_index + 2] +
                               0.587 * image->pixels[pixel_index + 1] +
                               0.114 * image->pixels[pixel_index]);

        /* 将RGB三个通道都设置为相同的灰度值 */
        image->pixels[pixel_index] = gray_value;     /* B */
        image->pixels[pixel_index + 1] = gray_value; /* G */
        image->pixels[pixel_index + 2] = gray_value; /* R */
    }
}

/**
 * @brief 对BMP图像执行卷积操作
 * @param image 输入BMP图像指针
 * @param kernel 卷积核数组
 * @param kernel_size 卷积核大小（必须是奇数）
 * @return 成功返回处理后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_apply_convolution(const BMPImage *image, const float *kernel, int kernel_size)
{
    int width, height, x, y, kx, ky;
    int kernel_radius;
    float r_sum, g_sum, b_sum;
    float kernel_value;
    int src_x, src_y;
    BMPImage *result = NULL;
    uint8_t *src_pixel, *dst_pixel;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL || kernel == NULL)
    {
        print_error("Invalid parameters");
        return NULL;
    }

    /* 检查卷积核大小是否为奇数 */
    if (kernel_size <= 0 || kernel_size % 2 == 0)
    {
        print_error("Kernel size must be a positive odd number");
        return NULL;
    }

    /* 创建结果图像 */
    result = (BMPImage *)malloc(sizeof(BMPImage));
    if (result == NULL)
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 从原图像复制头信息 */
    memcpy(&result->file_header, &image->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image->info_header, sizeof(BMPInfoHeader));

    width = image->info_header.width;
    height = abs(image->info_header.height);

    /* 分配像素数据内存 */
    result->pixels = (uint8_t *)malloc(width * height * 3);
    if (result->pixels == NULL)
    {
        print_error("Memory allocation failed");
        free(result);
        return NULL;
    }

    /* 计算卷积核半径 */
    kernel_radius = kernel_size / 2;

    /* 对每个像素执行卷积操作 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            /* 初始化累加和 */
            r_sum = g_sum = b_sum = 0.0f;

            /* 应用卷积核 */
            for (ky = -kernel_radius; ky <= kernel_radius; ky++)
            {
                for (kx = -kernel_radius; kx <= kernel_radius; kx++)
                {
                    /* 计算源图像坐标，处理边界情况 */
                    src_x = x + kx;
                    src_y = y + ky;

                    /* 边界处理：如果超出图像边界则使用最近的有效像素 */
                    if (src_x < 0)
                        src_x = 0;
                    if (src_x >= width)
                        src_x = width - 1;
                    if (src_y < 0)
                        src_y = 0;
                    if (src_y >= height)
                        src_y = height - 1;

                    /* 获取当前核位置的权重 */
                    kernel_value = kernel[(ky + kernel_radius) * kernel_size + (kx + kernel_radius)];

                    /* 获取源像素指针 */
                    src_pixel = image->pixels + (src_y * width + src_x) * 3;

                    /* 累加像素值乘以权重 */
                    b_sum += src_pixel[0] * kernel_value;
                    g_sum += src_pixel[1] * kernel_value;
                    r_sum += src_pixel[2] * kernel_value;
                }
            }

            /* 获取目标像素指针 */
            dst_pixel = result->pixels + (y * width + x) * 3;

            /* 限制值范围在0-255之间 */
            dst_pixel[0] = (uint8_t)(b_sum < 0 ? 0 : (b_sum > 255 ? 255 : b_sum)); /* B */
            dst_pixel[1] = (uint8_t)(g_sum < 0 ? 0 : (g_sum > 255 ? 255 : g_sum)); /* G */
            dst_pixel[2] = (uint8_t)(r_sum < 0 ? 0 : (r_sum > 255 ? 255 : r_sum)); /* R */
        }
    }

    return result;
}

/**
 * @brief 对BMP图像应用高斯模糊
 * @param image 输入BMP图像指针
 * @param radius 模糊半径
 * @return 成功返回处理后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_apply_blur(const BMPImage *image, int radius)
{
    float *kernel = NULL;
    int kernel_size;
    float sigma, sum;
    int i, j;
    BMPImage *result = NULL;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL || radius < 1)
    {
        print_error("Invalid parameters for blur");
        return NULL;
    }

    /* 计算卷积核大小 */
    kernel_size = radius * 2 + 1;

    /* 分配卷积核内存 */
    kernel = (float *)malloc(kernel_size * kernel_size * sizeof(float));
    if (kernel == NULL)
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 高斯模糊参数 */
    sigma = radius / 2.0f;
    sum = 0.0f;

    /* 生成高斯核 */
    for (i = 0; i < kernel_size; i++)
    {
        for (j = 0; j < kernel_size; j++)
        {
            int x = i - radius;
            int y = j - radius;
            float value = (1.0f / (2.0f * M_PI * sigma * sigma)) *
                          exp(-(x * x + y * y) / (2.0f * sigma * sigma));
            kernel[i * kernel_size + j] = value;
            sum += value;
        }
    }

    /* 归一化高斯核 */
    for (i = 0; i < kernel_size * kernel_size; i++)
    {
        kernel[i] /= sum;
    }

    /* 应用卷积 */
    result = bmp_apply_convolution(image, kernel, kernel_size);

    /* 释放卷积核内存 */
    free(kernel);

    return result;
}

/**
 * @brief 对BMP图像应用锐化效果
 * @param image 输入BMP图像指针
 * @param strength 锐化强度因子（0.0-5.0）
 * @return 成功返回处理后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_apply_sharpen(const BMPImage *image, float strength)
{
    float kernel[9];
    int kernel_size = 3;
    BMPImage *result = NULL;

    /* 检查参数有效性 */
    if (image == NULL || image->pixels == NULL)
    {
        print_error("Invalid parameters for sharpen");
        return NULL;
    }

    /* 限制锐化强度范围 */
    if (strength < 0.0f)
        strength = 0.0f;
    if (strength > 5.0f)
        strength = 5.0f;

    /* 设置锐化卷积核 */
    kernel[0] = kernel[2] = kernel[6] = kernel[8] = 0.0f;
    kernel[1] = kernel[3] = kernel[5] = kernel[7] = -strength;
    kernel[4] = 1.0f + 4.0f * strength; /* 中心像素 */

    /* 应用卷积 */
    result = bmp_apply_convolution(image, kernel, kernel_size);

    return result;
}

/**
 * @brief 打印BMP图像信息
 * @param image BMP图像指针
 */
void bmp_print_info(const BMPImage *image)
{
    if (image == NULL)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 文件头信息 */
    printf("====== BMP File Information ======\n");
    printf("File identifier: %c%c\n", (image->file_header.type & 0xFF), ((image->file_header.type >> 8) & 0xFF));
    printf("File size: %u bytes\n", image->file_header.size);
    printf("Reserved field 1: %u\n", image->file_header.reserved1);
    printf("Reserved field 2: %u\n", image->file_header.reserved2);
    printf("Bitmap data offset: %u bytes\n", image->file_header.offset);

    /* 信息头信息 */
    printf("\n====== BMP Image Information ======\n");
    printf("Info header size: %u bytes\n", image->info_header.size);
    printf("Bitmap width: %d pixels\n", image->info_header.width);
    printf("Bitmap height: %d pixels\n", image->info_header.height);
    printf("Color planes: %u\n", image->info_header.planes);
    printf("Color depth: %u bits/pixel\n", image->info_header.bit_count);
    printf("Compression type: %u\n", image->info_header.compression);
    printf("Image data size: %u bytes\n", image->info_header.image_size);
    printf("Horizontal resolution: %d pixels/meter\n", image->info_header.x_pixels_per_meter);
    printf("Vertical resolution: %d pixels/meter\n", image->info_header.y_pixels_per_meter);
    printf("Colors used: %u\n", image->info_header.colors_used);
    printf("Important colors: %u\n", image->info_header.colors_important);
    printf("============================\n");
}

/**
 * @brief 打印程序使用说明
 */
void print_usage()
{
    printf("Usage: ./standard_bmpedit -i input.bmp [-i input2.bmp] -o output.bmp -op operation [value]\n");
    printf("Operations:\n");
    printf("  add value      : 调整亮度，正值增加，负值减少\n");
    printf("  average        : 混合两张图像（需要两个输入文件）\n");
    printf("  grayscale      : 将图像转换为灰度\n");
    printf("  contrast value : 调整对比度，值为浮点数，大于1增加对比度，0-1之间降低对比度\n");
    printf("  blur radius    : 应用高斯模糊效果，radius为模糊半径（1-10）\n");
    printf("  sharpen value  : 应用锐化效果，value为锐化强度（0.1-5.0）\n");
    printf("Examples:\n");
    printf("  ./standard_bmpedit -i input.bmp -o output.bmp -op add 50\n");
    printf("  ./standard_bmpedit -i input1.bmp -i input2.bmp -o output.bmp -op average\n");
    printf("  ./standard_bmpedit -i input.bmp -o output.bmp -op grayscale\n");
    printf("  ./standard_bmpedit -i input.bmp -o output.bmp -op contrast 1.5\n");
    printf("  ./standard_bmpedit -i input.bmp -o output.bmp -op blur 3\n");
    printf("  ./standard_bmpedit -i input.bmp -o output.bmp -op sharpen 2.0\n");
}

/**
 * @brief 主函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 成功返回0，失败返回非0值
 */
int main(int argc, char *argv[])
{
    char *input_files[2] = {NULL, NULL};
    char *output_file = NULL;
    char *operation = NULL;
    int input_count = 0;
    int value = 0;
    BMPImage *images[2] = {NULL, NULL};
    BMPImage *result = NULL;
    int i, ret = EXIT_FAILURE;

    /* 计时器变量 */
    TimerData timer_total;
    TimerData timer_read;
    TimerData timer_process;
    TimerData timer_write;

    /* 初始化所有计时器 */
    timer_init(&timer_total);
    timer_init(&timer_read);
    timer_init(&timer_process);
    timer_init(&timer_write);

    /* 开始总时间计时 */
    timer_start(&timer_total);

    /* 解析命令行参数 */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-i") == 0)
        {
            /* 输入文件参数 */
            if (i + 1 < argc)
            {
                if (input_count < 2)
                {
                    input_files[input_count++] = argv[++i];
                }
                else
                {
                    fprintf(stderr, "Error: Maximum of two input files allowed\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing filename after -i option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            /* 输出文件参数 */
            if (i + 1 < argc)
            {
                output_file = argv[++i];
            }
            else
            {
                fprintf(stderr, "Error: Missing filename after -o option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-op") == 0)
        {
            /* 操作参数 */
            if (i + 1 < argc)
            {
                operation = argv[++i];

                /* 检查是否需要额外的参数 */
                if (strcmp(operation, "add") == 0)
                {
                    if (i + 1 < argc)
                    {
                        value = atoi(argv[++i]);
                    }
                    else
                    {
                        fprintf(stderr, "Error: add operation requires a value parameter\n");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                }
                else if (strcmp(operation, "contrast") == 0)
                {
                    if (i + 1 < argc)
                    {
                        /* 对比度需要一个浮点数参数，保存到value会在后面处理 */
                        value = atoi(argv[++i]); /* 暂时保存在value中 */
                    }
                    else
                    {
                        fprintf(stderr, "Error: contrast operation requires a factor parameter\n");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                }
                else if (strcmp(operation, "blur") == 0)
                {
                    if (i + 1 < argc)
                    {
                        value = atoi(argv[++i]);
                        if (value < 1 || value > 10)
                        {
                            fprintf(stderr, "Warning: Blur radius should be between 1 and 10, using default value 3\n");
                            value = 3;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Error: blur operation requires a radius parameter\n");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                }
                else if (strcmp(operation, "sharpen") == 0)
                {
                    if (i + 1 < argc)
                    {
                        /* 锐化需要一个浮点数参数，保存到value会在后面处理 */
                        value = atoi(argv[++i]); /* 暂时保存在value中 */
                    }
                    else
                    {
                        fprintf(stderr, "Error: sharpen operation requires a strength parameter\n");
                        print_usage();
                        return EXIT_FAILURE;
                    }
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing operation type after -op option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else
        {
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            print_usage();
            return EXIT_FAILURE;
        }
    }

    /* 检查必需的参数 */
    if (input_count == 0 || output_file == NULL || operation == NULL)
    {
        fprintf(stderr, "Error: Missing required parameters\n");
        print_usage();
        return EXIT_FAILURE;
    }

    /* 根据操作类型检查参数 */
    if (strcmp(operation, "average") == 0 && input_count != 2)
    {
        fprintf(stderr, "Error: average operation requires two input files\n");
        print_usage();
        return EXIT_FAILURE;
    }

    if ((strcmp(operation, "add") == 0 ||
         strcmp(operation, "grayscale") == 0 ||
         strcmp(operation, "contrast") == 0 ||
         strcmp(operation, "blur") == 0 ||
         strcmp(operation, "sharpen") == 0) &&
        input_count != 1)
    {
        fprintf(stderr, "Error: %s operation requires only one input file\n", operation);
        print_usage();
        return EXIT_FAILURE;
    }

    /* 初始化图像指针 */
    for (i = 0; i < 2; i++)
    {
        images[i] = NULL;
    }
    result = NULL;

    /* 开始文件读取计时 */
    timer_start(&timer_read);

    /* 读取输入文件 */
    for (i = 0; i < input_count; i++)
    {
        if (input_files[i] == NULL)
        {
            fprintf(stderr, "Error: Invalid input filename for image %d\n", i + 1);
            ret = EXIT_FAILURE;
            /* 直接跳到清理资源部分 */
            i = input_count; /* 终止循环 */
            continue;
        }

        images[i] = bmp_read(input_files[i]);
        if (images[i] == NULL)
        {
            fprintf(stderr, "Error: Cannot read input file: %s\n", input_files[i]);
            ret = EXIT_FAILURE;
            /* 直接跳到清理资源部分 */
            i = input_count; /* 终止循环 */
            continue;
        }

        /* 打印当前读取的BMP文件信息 */
        printf("\n----- BMP information for file %s -----\n", input_files[i]);
        bmp_print_info(images[i]);
        printf("\n");

        /* 设置ret = EXIT_SUCCESS表示读取成功 */
        ret = EXIT_SUCCESS;
    }

    /* 结束文件读取计时 */
    timer_stop(&timer_read);

    /* 只有当读取图像成功时才继续处理 */
    if (ret != EXIT_FAILURE)
    {
        /* 开始图像处理计时 */
        timer_start(&timer_process);

        /* 根据操作类型处理图像 */
        if (strcmp(operation, "add") == 0)
        {
            /* 调整亮度 */
            printf("调整亮度: %d...\n", value);
            if (images[0] == NULL)
            {
                fprintf(stderr, "Error: Invalid image for brightness adjustment\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = images[0];
                bmp_adjust_brightness(result, value);
            }
        }
        else if (strcmp(operation, "average") == 0)
        {
            /* 混合图像 */
            printf("混合两张图像...\n");
            if (images[0] == NULL || images[1] == NULL)
            {
                fprintf(stderr, "Error: Invalid images for blending\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = bmp_average(images[0], images[1]);
                if (result == NULL)
                {
                    fprintf(stderr, "Error: Failed to blend images\n");
                    ret = EXIT_FAILURE;
                }
            }
        }
        else if (strcmp(operation, "grayscale") == 0)
        {
            /* 灰度化 */
            printf("将图像转换为灰度...\n");
            if (images[0] == NULL)
            {
                fprintf(stderr, "Error: Invalid image for grayscale conversion\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = images[0];
                bmp_convert_grayscale(result);
            }
        }
        else if (strcmp(operation, "contrast") == 0)
        {
            /* 调整对比度 */
            float factor = atof(argv[i - 1]); /* 使用正确的参数索引 */
            printf("调整对比度，因子为 %.2f...\n", factor);
            if (images[0] == NULL)
            {
                fprintf(stderr, "Error: Invalid image for contrast adjustment\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = images[0];
                bmp_adjust_contrast(result, factor);
            }
        }
        else if (strcmp(operation, "blur") == 0)
        {
            /* 高斯模糊 */
            printf("应用高斯模糊，半径为 %d...\n", value);
            if (images[0] == NULL)
            {
                fprintf(stderr, "Error: Invalid image for blur effect\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = bmp_apply_blur(images[0], value);
                if (result == NULL)
                {
                    fprintf(stderr, "Error: Failed to apply blur effect\n");
                    ret = EXIT_FAILURE;
                }
            }
        }
        else if (strcmp(operation, "sharpen") == 0)
        {
            /* 锐化 */
            float strength = atof(argv[i - 1]); /* 使用正确的参数索引 */
            printf("应用锐化效果，强度为 %.2f...\n", strength);
            if (images[0] == NULL)
            {
                fprintf(stderr, "Error: Invalid image for sharpen effect\n");
                ret = EXIT_FAILURE;
            }
            else
            {
                result = bmp_apply_sharpen(images[0], strength);
                if (result == NULL)
                {
                    fprintf(stderr, "Error: Failed to apply sharpen effect\n");
                    ret = EXIT_FAILURE;
                }
            }
        }
        else
        {
            fprintf(stderr, "Error: Unsupported operation: %s\n", operation ? operation : "NULL");
            ret = EXIT_FAILURE;
        }

        /* 结束图像处理计时 */
        timer_stop(&timer_process);
    }

    /* 如果上面的操作有错误，就跳过写入部分 */
    if (ret != EXIT_FAILURE && result != NULL)
    {
        /* 开始文件写入计时 */
        timer_start(&timer_write);

        /* 写入输出文件 */
        if (output_file == NULL)
        {
            fprintf(stderr, "Error: Invalid output filename\n");
            ret = EXIT_FAILURE;
        }
        else
        {
            printf("Writing output file: %s\n", output_file);
            if (bmp_write(result, output_file) != 0)
            {
                fprintf(stderr, "Error: Failed to write output file: %s\n", output_file);
                ret = EXIT_FAILURE;
            }
            else
            {
                printf("Operation completed successfully!\n");
                ret = EXIT_SUCCESS;
            }
        }

        /* 结束文件写入计时 */
        timer_stop(&timer_write);
    }

    /* 结束总时间计时 */
    timer_stop(&timer_total);

    /* 打印各阶段执行时间 */
    printf("\n====== 执行时间统计 ======\n");
    timer_print("图像读取时间", &timer_read);
    timer_print("图像处理时间", &timer_process);
    timer_print("图像写入时间", &timer_write);
    timer_print("总执行时间", &timer_total);
    printf("==========================\n");

    /* 释放资源 - 安全地释放所有资源 */
    for (i = 0; i < input_count; i++)
    {
        /* 确保不会重复释放相同的内存 */
        if (images[i] != NULL && images[i] != result)
        {
            bmp_free(images[i]);
            images[i] = NULL;
        }
    }

    /* 处理result指针 */
    if (result != NULL && result != images[0])
    {
        bmp_free(result);
    }
    else if (result == images[0])
    {
        /* 如果result指向images[0]，images[0]已经被释放，所以我们只需要将指针置空 */
        images[0] = NULL;
    }

    return ret;
}