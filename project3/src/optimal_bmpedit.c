#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <immintrin.h> // AVX/SSE指令集
#include <unistd.h>    // sysconf获取CPU核心数
#include <sys/time.h>  // 添加用于gettimeofday()的头文件

// 定义最大线程数
#define MAX_THREADS 16

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
    uint32_t row_stride;       /* 缓存行跨度，避免重复计算 */
} BMPImage;

/**
 * @brief 线程处理图像的参数结构
 */
typedef struct
{
    uint8_t *pixels;        /* 指向像素数据的指针 */
    int start_pixel;        /* 起始像素索引 */
    int end_pixel;          /* 结束像素索引 */
    int value;              /* 亮度调整值 */
    uint8_t *pixels2;       /* 第二个图像的像素数据指针（用于混合） */
    uint8_t *result_pixels; /* 结果图像的像素数据指针 */
    int operation;          /* 操作类型：0=亮度调整，1=混合 */
} ThreadData;

/* 函数声明 */
static inline void print_error(const char *message);
static inline uint32_t bmp_get_row_stride(int width);
static inline int bmp_is_valid(const BMPImage *image);
BMPImage *bmp_read(const char *filename);
int bmp_write(const BMPImage *image, const char *filename);
void bmp_free(BMPImage *image);
void *thread_process_pixels(void *arg);
void bmp_adjust_brightness(BMPImage *image, int value);
void bmp_adjust_brightness_parallel(BMPImage *image, int value);
void bmp_adjust_brightness_simd(BMPImage *image, int value);
BMPImage *bmp_average(const BMPImage *image1, const BMPImage *image2);
BMPImage *bmp_average_parallel(const BMPImage *image1, const BMPImage *image2);
BMPImage *bmp_average_simd(const BMPImage *image1, const BMPImage *image2);
void bmp_print_info(const BMPImage *image);
void print_usage();
int get_optimal_thread_count();

/**
 * @brief 获取最优线程数（基于CPU核心数）
 * @return 最优线程数
 */
int get_optimal_thread_count()
{
    int num_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    // 如果无法获取或结果异常，使用默认值
    if (num_cores <= 0)
        return 4;

    // 限制最大线程数以避免线程创建开销超过性能收益
    return (num_cores > MAX_THREADS) ? MAX_THREADS : num_cores;
}

/**
 * @brief 打印错误信息（内联函数提高性能）
 * @param message 错误信息
 */
static inline void print_error(const char *message)
{
    if (message != NULL)
    {
        fprintf(stderr, "Error: %s\n", message);
    }
}

/**
 * @brief 获取一行的字节数（包括填充字节）（内联函数提高性能）
 * @param width 图像宽度
 * @return 一行的字节数
 */
static inline uint32_t bmp_get_row_stride(int width)
{
    /* 每行必须是4字节的倍数，每像素3字节（BGR）*/
    return ((width * 3 + 3) & ~3); // 按4对齐的优化写法
}

/**
 * @brief 检查BMP图像是否有效（24位未压缩格式）（内联函数提高性能）
 * @param image BMP图像指针
 * @return 如果有效返回1，如果无效返回0
 */
static inline int bmp_is_valid(const BMPImage *image)
{
    /* 快速检查参数有效性 */
    if (!image || !image->pixels)
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

    return 1;
}

/**
 * @brief 从文件读取BMP图像（优化版本）
 * @param filename 文件名
 * @return 成功返回BMPImage指针，失败返回NULL
 */
BMPImage *bmp_read(const char *filename)
{
    FILE *fp = NULL;
    BMPImage *image = NULL;
    size_t read_size;
    int row, col;
    uint8_t *row_buffer = NULL;

    /* 检查参数有效性 */
    if (!filename)
    {
        print_error("Filename is NULL");
        return NULL;
    }

    /* 打开文件 */
    if (!(fp = fopen(filename, "rb")))
    {
        print_error("Cannot open file");
        return NULL;
    }

    /* 分配BMP图像结构内存 */
    if (!(image = (BMPImage *)malloc(sizeof(BMPImage))))
    {
        print_error("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    /* 初始化为零 */
    memset(image, 0, sizeof(BMPImage));

    /* 读取文件头和信息头（一次读取两个结构体，提高效率） */
    if (fread(&image->file_header, 1, sizeof(BMPFileHeader), fp) != sizeof(BMPFileHeader) ||
        fread(&image->info_header, 1, sizeof(BMPInfoHeader), fp) != sizeof(BMPInfoHeader))
    {
        print_error("Failed to read BMP headers");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 验证BMP格式 */
    if (image->file_header.type != 0x4D42 ||
        image->info_header.bit_count != 24 ||
        image->info_header.compression != 0)
    {
        print_error("Unsupported BMP format");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 缓存行跨度，避免重复计算 */
    image->row_stride = bmp_get_row_stride(image->info_header.width);

    /* 分配像素数据内存，一次性分配足够大的内存块，提高效率 */
    const int height_abs = abs(image->info_header.height);
    const int required_size = image->info_header.width * height_abs * 3;

    if (!(image->pixels = (uint8_t *)aligned_alloc(32, ((required_size + 31) & ~31)))) // 32字节对齐，用于SIMD指令
    {
        print_error("Memory allocation failed");
        free(image);
        fclose(fp);
        return NULL;
    }

    /* 分配行缓冲区 */
    if (!(row_buffer = (uint8_t *)malloc(image->row_stride)))
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

    /* 读取像素数据 - 优化循环 */
    const int width = image->info_header.width;
    for (row = 0; row < height_abs; row++)
    {
        /* 确定当前行索引（BMP从下到上存储行） */
        const int current_row = (image->info_header.height > 0) ? (height_abs - 1 - row) : row;

        /* 读取一行（包括填充字节） */
        if (fread(row_buffer, 1, image->row_stride, fp) != image->row_stride)
        {
            print_error("Failed to read pixel data");
            free(row_buffer);
            free(image->pixels);
            free(image);
            fclose(fp);
            return NULL;
        }

        /* 使用指针算法优化像素数据复制 */
        uint8_t *dst_ptr = image->pixels + current_row * width * 3;
        uint8_t *src_ptr = row_buffer;

        // 使用内存复制优化处理
        for (col = 0; col < width; col++, dst_ptr += 3, src_ptr += 3)
        {
            dst_ptr[0] = src_ptr[0]; /* B */
            dst_ptr[1] = src_ptr[1]; /* G */
            dst_ptr[2] = src_ptr[2]; /* R */
        }
    }

    /* 释放资源并返回 */
    free(row_buffer);
    fclose(fp);
    return image;
}

/**
 * @brief 将BMP图像写入文件（优化版本）
 * @param image BMP图像指针
 * @param filename 文件名
 * @return 成功返回0，失败返回非0值
 */
int bmp_write(const BMPImage *image, const char *filename)
{
    FILE *fp = NULL;
    int row, col;
    uint8_t *row_buffer = NULL;
    uint8_t padding_bytes[3] = {0, 0, 0}; /* 用于行填充的字节 */

    /* 检查参数有效性 */
    if (!image || !filename)
    {
        print_error("Invalid parameters");
        return -1;
    }

    if (!bmp_is_valid(image))
    {
        return -1;
    }

    /* 打开文件 */
    if (!(fp = fopen(filename, "wb")))
    {
        print_error("Cannot create file");
        return -1;
    }

    /* 计算行跨度和填充字节 */
    const uint32_t row_stride = image->row_stride ? image->row_stride : bmp_get_row_stride(image->info_header.width);
    const int row_padding = row_stride - image->info_header.width * 3;

    /* 写入文件头和信息头（一次性写入，减少IO操作） */
    if (fwrite(&image->file_header, 1, sizeof(BMPFileHeader), fp) != sizeof(BMPFileHeader) ||
        fwrite(&image->info_header, 1, sizeof(BMPInfoHeader), fp) != sizeof(BMPInfoHeader))
    {
        print_error("Failed to write headers");
        fclose(fp);
        return -1;
    }

    /* 分配行缓冲区 */
    if (!(row_buffer = (uint8_t *)malloc(row_stride)))
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

    /* 写入像素数据 - 优化循环 */
    const int width = image->info_header.width;
    const int height_abs = abs(image->info_header.height);

    for (row = 0; row < height_abs; row++)
    {
        /* 确定当前行索引（BMP从下到上存储行） */
        const int current_row = (image->info_header.height > 0) ? (height_abs - 1 - row) : row;

        /* 使用指针算法优化像素数据复制 */
        uint8_t *src_ptr = image->pixels + current_row * width * 3;
        uint8_t *dst_ptr = row_buffer;

        for (col = 0; col < width; col++, dst_ptr += 3, src_ptr += 3)
        {
            dst_ptr[0] = src_ptr[0]; /* B */
            dst_ptr[1] = src_ptr[1]; /* G */
            dst_ptr[2] = src_ptr[2]; /* R */
        }

        /* 一次性写入一行数据+填充（减少IO操作） */
        if (fwrite(row_buffer, 1, width * 3, fp) != (size_t)(width * 3))
        {
            print_error("Failed to write pixel data");
            free(row_buffer);
            fclose(fp);
            return -1;
        }

        /* 写入填充字节 */
        if (row_padding > 0)
        {
            if (fwrite(padding_bytes, 1, row_padding, fp) != (size_t)row_padding)
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
    if (image)
    {
        if (image->pixels)
        {
            free(image->pixels);
            image->pixels = NULL;
        }
        free(image);
    }
}

/**
 * @brief 线程处理像素函数
 * @param arg 线程数据结构指针
 * @return NULL
 */
void *thread_process_pixels(void *arg)
{
    ThreadData *data = (ThreadData *)arg;
    int i;

    if (data->operation == 0) // 亮度调整
    {
        for (i = data->start_pixel; i < data->end_pixel; i++)
        {
            int new_value = data->pixels[i] + data->value;
            /* 使用条件操作而非分支，提高性能 */
            data->pixels[i] = (new_value > 255) ? 255 : ((new_value < 0) ? 0 : new_value);
        }
    }
    else if (data->operation == 1) // 图像混合
    {
        for (i = data->start_pixel; i < data->end_pixel; i++)
        {
            data->result_pixels[i] = (data->pixels[i] + data->pixels2[i]) >> 1; // 位运算代替除法
        }
    }

    return NULL;
}

/**
 * @brief 调整BMP图像亮度（使用多线程）
 * @param image BMP图像指针
 * @param value 亮度调整值，正值增加亮度，负值减少亮度
 */
void bmp_adjust_brightness_parallel(BMPImage *image, int value)
{
    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 计算总像素数 */
    const int total_pixels = image->info_header.width * abs(image->info_header.height) * 3;
    const int thread_count = get_optimal_thread_count();
    const int pixels_per_thread = total_pixels / thread_count;

    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int i, ret;

    /* 创建多个线程处理不同区域的像素 */
    for (i = 0; i < thread_count; i++)
    {
        thread_data[i].pixels = image->pixels;
        thread_data[i].start_pixel = i * pixels_per_thread;
        thread_data[i].end_pixel = (i == thread_count - 1) ? total_pixels : // 最后一个线程处理剩余所有像素
                                       (i + 1) * pixels_per_thread;
        thread_data[i].value = value;
        thread_data[i].operation = 0; // 亮度调整操作

        ret = pthread_create(&threads[i], NULL, thread_process_pixels, &thread_data[i]);
        if (ret)
        {
            print_error("Failed to create thread");
            return;
        }
    }

    /* 等待所有线程完成 */
    for (i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

/**
 * @brief 调整BMP图像亮度（使用SIMD指令优化）
 * @param image BMP图像指针
 * @param value 亮度调整值，正值增加亮度，负值减少亮度
 */
void bmp_adjust_brightness_simd(BMPImage *image, int value)
{
    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 计算总像素数 */
    const int total_pixels = image->info_header.width * abs(image->info_header.height) * 3;
    int i = 0;

    /* 使用SSE指令处理像素 */
    if (total_pixels >= 16) // 确保有足够的像素进行SIMD处理
    {
        /* 创建亮度调整值的向量 */
        __m128i add_val = _mm_set1_epi8((char)value);
        __m128i zero = _mm_setzero_si128();
        __m128i max_val = _mm_set1_epi8((char)255);

        /* 使用SSE2指令一次处理16个字节 */
        for (; i <= total_pixels - 16; i += 16)
        {
            /* 加载16个字节 */
            __m128i pixels = _mm_loadu_si128((__m128i *)(image->pixels + i));

            /* 将有符号整数转换为无符号整数，以便正确处理饱和加法 */
            __m128i result;

            if (value >= 0)
            {
                /* 对于正值，使用饱和加法直接处理 */
                result = _mm_adds_epu8(pixels, add_val);
            }
            else
            {
                /* 对于负值，使用饱和减法 */
                add_val = _mm_set1_epi8((char)(-value)); // 取负值的绝对值
                result = _mm_subs_epu8(pixels, add_val);
            }

            /* 存储结果 */
            _mm_storeu_si128((__m128i *)(image->pixels + i), result);
        }
    }

    /* 处理剩余的像素 */
    for (; i < total_pixels; i++)
    {
        int new_value = image->pixels[i] + value;
        /* 使用条件操作而非分支，提高性能 */
        image->pixels[i] = (new_value > 255) ? 255 : ((new_value < 0) ? 0 : new_value);
    }
}

/**
 * @brief 调整BMP图像亮度（根据图像大小选择最佳实现）
 * @param image BMP图像指针
 * @param value 亮度调整值，正值增加亮度，负值减少亮度
 */
void bmp_adjust_brightness(BMPImage *image, int value)
{
    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 根据图像大小选择最佳实现 */
    const int total_pixels = image->info_header.width * abs(image->info_header.height);

    if (total_pixels > 500000) // 大图像使用多线程+SIMD
    {
        bmp_adjust_brightness_parallel(image, value);
    }
    else // 小图像直接使用SIMD
    {
        bmp_adjust_brightness_simd(image, value);
    }
}

/**
 * @brief 通过平均值混合两个BMP图像（使用多线程）
 * @param image1 第一个BMP图像指针
 * @param image2 第二个BMP图像指针
 * @return 成功返回混合后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_average_parallel(const BMPImage *image1, const BMPImage *image2)
{
    BMPImage *result = NULL;
    int total_pixels;
    pthread_t threads[MAX_THREADS];
    ThreadData thread_data[MAX_THREADS];
    int i, ret;
    const int thread_count = get_optimal_thread_count();

    /* 检查参数有效性 */
    if (!image1 || !image2 || !image1->pixels || !image2->pixels)
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
    if (!(result = (BMPImage *)malloc(sizeof(BMPImage))))
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 从第一个图像复制头信息 */
    memcpy(&result->file_header, &image1->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image1->info_header, sizeof(BMPInfoHeader));
    result->row_stride = image1->row_stride ? image1->row_stride : bmp_get_row_stride(image1->info_header.width);

    /* 计算总像素数 */
    total_pixels = image1->info_header.width * abs(image1->info_header.height) * 3;

    /* 分配像素数据内存（对齐分配以便SIMD指令使用） */
    if (!(result->pixels = (uint8_t *)aligned_alloc(32, ((total_pixels + 31) & ~31))))
    {
        print_error("Memory allocation failed");
        free(result);
        return NULL;
    }

    /* 每线程处理的像素数量 */
    const int pixels_per_thread = total_pixels / thread_count;

    /* 创建多个线程处理不同区域的像素 */
    for (i = 0; i < thread_count; i++)
    {
        thread_data[i].pixels = (uint8_t *)image1->pixels;  // 第一个图像的像素
        thread_data[i].pixels2 = (uint8_t *)image2->pixels; // 第二个图像的像素
        thread_data[i].result_pixels = result->pixels;      // 结果图像的像素
        thread_data[i].start_pixel = i * pixels_per_thread;
        thread_data[i].end_pixel = (i == thread_count - 1) ? total_pixels : // 最后一个线程处理剩余所有像素
                                       (i + 1) * pixels_per_thread;
        thread_data[i].operation = 1; // 混合操作

        ret = pthread_create(&threads[i], NULL, thread_process_pixels, &thread_data[i]);
        if (ret)
        {
            print_error("Failed to create thread");
            free(result->pixels);
            free(result);
            return NULL;
        }
    }

    /* 等待所有线程完成 */
    for (i = 0; i < thread_count; i++)
    {
        pthread_join(threads[i], NULL);
    }

    return result;
}

/**
 * @brief 通过平均值混合两个BMP图像（使用SIMD指令优化）
 * @param image1 第一个BMP图像指针
 * @param image2 第二个BMP图像指针
 * @return 成功返回混合后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_average_simd(const BMPImage *image1, const BMPImage *image2)
{
    BMPImage *result = NULL;
    int total_pixels;
    int i = 0;

    /* 检查参数有效性 */
    if (!image1 || !image2 || !image1->pixels || !image2->pixels)
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
    if (!(result = (BMPImage *)malloc(sizeof(BMPImage))))
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 从第一个图像复制头信息 */
    memcpy(&result->file_header, &image1->file_header, sizeof(BMPFileHeader));
    memcpy(&result->info_header, &image1->info_header, sizeof(BMPInfoHeader));
    result->row_stride = image1->row_stride ? image1->row_stride : bmp_get_row_stride(image1->info_header.width);

    /* 计算总像素数 */
    total_pixels = image1->info_header.width * abs(image1->info_header.height) * 3;

    /* 分配像素数据内存（对齐分配以便SIMD指令使用） */
    if (!(result->pixels = (uint8_t *)aligned_alloc(32, ((total_pixels + 31) & ~31))))
    {
        print_error("Memory allocation failed");
        free(result);
        return NULL;
    }

    /* 使用SSE指令处理像素 */
    if (total_pixels >= 16) // 确保有足够的像素进行SIMD处理
    {
        /* 使用SSE2指令一次处理16个字节 */
        for (; i <= total_pixels - 16; i += 16)
        {
            /* 加载两个图像的16个字节 */
            __m128i pixels1 = _mm_loadu_si128((__m128i *)(image1->pixels + i));
            __m128i pixels2 = _mm_loadu_si128((__m128i *)(image2->pixels + i));

            /* 计算平均值 - 使用更高效的向量指令 */
            __m128i avg_result = _mm_avg_epu8(pixels1, pixels2);

            /* 存储结果 */
            _mm_storeu_si128((__m128i *)(result->pixels + i), avg_result);
        }
    }

    /* 处理剩余的像素 */
    for (; i < total_pixels; i++)
    {
        result->pixels[i] = (image1->pixels[i] + image2->pixels[i]) >> 1; // 位运算代替除法
    }

    return result;
}

/**
 * @brief 通过平均值混合两个BMP图像（根据图像大小选择最佳实现）
 * @param image1 第一个BMP图像指针
 * @param image2 第二个BMP图像指针
 * @return 成功返回混合后的BMPImage指针，失败返回NULL
 */
BMPImage *bmp_average(const BMPImage *image1, const BMPImage *image2)
{
    /* 检查参数有效性 */
    if (!image1 || !image2 || !image1->pixels || !image2->pixels)
    {
        print_error("Invalid BMP image pointer");
        return NULL;
    }

    /* 根据图像大小选择最佳实现 */
    const int total_pixels = image1->info_header.width * abs(image1->info_header.height);

    if (total_pixels > 500000) // 大图像使用多线程
    {
        return bmp_average_parallel(image1, image2);
    }
    else // 小图像直接使用SIMD
    {
        return bmp_average_simd(image1, image2);
    }
}

/**
 * @brief 打印BMP图像信息
 * @param image BMP图像指针
 */
void bmp_print_info(const BMPImage *image)
{
    if (!image)
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
    printf("Usage: ./optimal_bmpedit -i input.bmp [-i input2.bmp] -o output.bmp -op operation [value]\n");
    printf("Operations:\n");
    printf("  add value     : Adjust image brightness, positive value increases, negative decreases\n");
    printf("  average       : Blend two images by averaging (requires two input files)\n");
    printf("Examples:\n");
    printf("  ./optimal_bmpedit -i input.bmp -o output.bmp -op add 50\n");
    printf("  ./optimal_bmpedit -i input1.bmp -i input2.bmp -o output.bmp -op average\n");
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
    if (input_count == 0 || !output_file || !operation)
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

    if (strcmp(operation, "add") == 0 && input_count != 1)
    {
        fprintf(stderr, "Error: add operation requires only one input file\n");
        print_usage();
        return EXIT_FAILURE;
    }

    /* 开始文件读取计时 */
    timer_start(&timer_read);

    /* 读取输入文件 */
    for (i = 0; i < input_count; i++)
    {
        if (!input_files[i])
        {
            fprintf(stderr, "Error: Invalid input filename for image %d\n", i + 1);
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        images[i] = bmp_read(input_files[i]);
        if (!images[i])
        {
            fprintf(stderr, "Error: Cannot read input file: %s\n", input_files[i]);
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        /* 打印当前读取的BMP文件信息 */
        printf("\n----- BMP information for file %s -----\n", input_files[i]);
        bmp_print_info(images[i]);
        printf("\n");

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
            printf("Optimized brightness adjustment by %d...\n", value);
            if (!images[0])
            {
                fprintf(stderr, "Error: Invalid image for brightness adjustment\n");
                ret = EXIT_FAILURE;
                goto cleanup;
            }

            result = images[0];
            bmp_adjust_brightness(result, value);
        }
        else if (strcmp(operation, "average") == 0)
        {
            /* 混合图像 */
            printf("Optimized image blending...\n");
            if (!images[0] || !images[1])
            {
                fprintf(stderr, "Error: Invalid images for blending\n");
                ret = EXIT_FAILURE;
                goto cleanup;
            }

            result = bmp_average(images[0], images[1]);
            if (!result)
            {
                fprintf(stderr, "Error: Failed to blend images\n");
                ret = EXIT_FAILURE;
                goto cleanup;
            }
        }
        else
        {
            fprintf(stderr, "Error: Unsupported operation: %s\n", operation ? operation : "NULL");
            ret = EXIT_FAILURE;
            goto cleanup;
        }

        /* 结束图像处理计时 */
        timer_stop(&timer_process);
    }

    /* 写入输出文件 */
    if (ret != EXIT_FAILURE && result)
    {
        /* 开始文件写入计时 */
        timer_start(&timer_write);

        printf("Writing output file: %s\n", output_file);
        if (bmp_write(result, output_file) != 0)
        {
            fprintf(stderr, "Error: Failed to write output file: %s\n", output_file);
            ret = EXIT_FAILURE;
        }
        else
        {
            printf("Operation completed successfully with optimized performance!\n");
            ret = EXIT_SUCCESS;
        }

        /* 结束文件写入计时 */
        timer_stop(&timer_write);
    }

    /* 结束总时间计时 */
    timer_stop(&timer_total);

    /* 打印各阶段执行时间 */
    printf("\n====== 优化版执行时间统计 ======\n");
    timer_print("图像读取时间", &timer_read);
    timer_print("图像处理时间", &timer_process);
    timer_print("图像写入时间", &timer_write);
    timer_print("总执行时间", &timer_total);
    printf("================================\n");

cleanup:
    /* 释放资源 - 安全地释放所有资源 */
    for (i = 0; i < input_count; i++)
    {
        /* 确保不会重复释放相同的内存 */
        if (images[i] && images[i] != result)
        {
            bmp_free(images[i]);
            images[i] = NULL;
        }
    }

    /* 处理result指针 */
    if (result && result != images[0])
    {
        bmp_free(result);
    }
    else if (result == images[0])
    {
        images[0] = NULL;
    }

    return ret;
}