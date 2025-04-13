/**
 * @file bmp_generator.c
 * @brief BMP图像生成器，可以随机生成不同模式的BMP图像
 * 支持通过命令行参数指定生成的图像宽度、高度和生成模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

/**
 * @brief BMP文件头结构
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

/* 定义生成模式 */
#define MODE_RANDOM 0     /* 完全随机像素 */
#define MODE_GRADIENT 1   /* 渐变效果 */
#define MODE_CHECKER 2    /* 棋盘格效果 */
#define MODE_CIRCLES 3    /* 同心圆效果 */
#define MODE_NOISE 4      /* 噪点效果 */
#define MODE_STRIPES 5    /* 条纹效果 */
#define MODE_MANDELBROT 6 /* 曼德布洛特分形 */

/* 函数声明 */
void print_error(const char *message);
uint32_t bmp_get_row_stride(int width);
BMPImage *bmp_create(int width, int height);
void bmp_fill_random(BMPImage *image);
void bmp_fill_gradient(BMPImage *image);
void bmp_fill_checker(BMPImage *image, int square_size);
void bmp_fill_circles(BMPImage *image);
void bmp_fill_noise(BMPImage *image, int noise_level);
void bmp_fill_stripes(BMPImage *image, int stripe_width);
void bmp_fill_mandelbrot(BMPImage *image, double zoom, double center_x, double center_y);
int bmp_write(const BMPImage *image, const char *filename);
void bmp_free(BMPImage *image);
void print_usage();

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
 * @brief 获取BMP行跨度（包括填充）
 * @param width 图像宽度
 * @return 行跨度（字节数）
 */
uint32_t bmp_get_row_stride(int width)
{
    /* 每行必须是4字节的倍数，每像素3字节（BGR） */
    return ((width * 3 + 3) & ~3);
}

/**
 * @brief 创建指定尺寸的BMP图像
 * @param width 图像宽度
 * @param height 图像高度
 * @return 成功返回BMPImage指针，失败返回NULL
 */
BMPImage *bmp_create(int width, int height)
{
    BMPImage *image = NULL;
    uint32_t row_stride;
    uint32_t header_size;
    uint32_t pixel_data_size;

    /* 检查参数有效性 */
    if (width <= 0 || height == 0)
    {
        print_error("Invalid image dimensions");
        return NULL;
    }

    /* 分配BMP图像结构内存 */
    if (!(image = (BMPImage *)malloc(sizeof(BMPImage))))
    {
        print_error("Memory allocation failed");
        return NULL;
    }

    /* 初始化为零 */
    memset(image, 0, sizeof(BMPImage));

    /* 填充BMP头信息 */
    image->file_header.type = 0x4D42; /* 'BM' */
    image->file_header.reserved1 = 0;
    image->file_header.reserved2 = 0;
    image->file_header.offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    image->info_header.size = sizeof(BMPInfoHeader);
    image->info_header.width = width;
    image->info_header.height = height;
    image->info_header.planes = 1;
    image->info_header.bit_count = 24;            /* 24位BMP图像 */
    image->info_header.compression = 0;           /* 不压缩 */
    image->info_header.x_pixels_per_meter = 2835; /* 72 DPI */
    image->info_header.y_pixels_per_meter = 2835; /* 72 DPI */
    image->info_header.colors_used = 0;
    image->info_header.colors_important = 0;

    /* 计算文件大小 */
    row_stride = bmp_get_row_stride(width);
    pixel_data_size = row_stride * abs(height);
    image->info_header.image_size = pixel_data_size;
    image->file_header.size = image->file_header.offset + pixel_data_size;

    /* 分配像素数据内存 */
    if (!(image->pixels = (uint8_t *)malloc(pixel_data_size)))
    {
        print_error("Memory allocation failed");
        free(image);
        return NULL;
    }

    /* 初始化像素数据为零 */
    memset(image->pixels, 0, pixel_data_size);

    return image;
}

/**
 * @brief 填充随机像素值
 * @param image BMP图像指针
 */
void bmp_fill_random(BMPImage *image)
{
    int width, height, x, y;
    uint32_t row_stride;
    uint8_t *pixel;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    width = image->info_header.width;
    height = abs(image->info_header.height);
    row_stride = bmp_get_row_stride(width);

    /* 填充随机像素值 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;
            pixel[0] = rand() % 256; /* B */
            pixel[1] = rand() % 256; /* G */
            pixel[2] = rand() % 256; /* R */
        }
    }
}

/**
 * @brief 填充渐变效果
 * @param image BMP图像指针
 */
void bmp_fill_gradient(BMPImage *image)
{
    int width, height, x, y;
    uint8_t *pixel;
    float r_factor, g_factor, b_factor;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    width = image->info_header.width;
    height = abs(image->info_header.height);

    /* 填充渐变效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            r_factor = (float)x / width;
            g_factor = (float)y / height;
            b_factor = 1.0f - ((float)x / width + (float)y / height) / 2.0f;

            pixel[0] = (uint8_t)(255 * b_factor); /* B */
            pixel[1] = (uint8_t)(255 * g_factor); /* G */
            pixel[2] = (uint8_t)(255 * r_factor); /* R */
        }
    }
}

/**
 * @brief 填充棋盘格效果
 * @param image BMP图像指针
 * @param square_size 方格大小
 */
void bmp_fill_checker(BMPImage *image, int square_size)
{
    int width, height, x, y;
    uint8_t *pixel;
    int is_white;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    if (square_size <= 0)
    {
        square_size = 20; /* 默认方格大小 */
    }

    width = image->info_header.width;
    height = abs(image->info_header.height);

    /* 填充棋盘格效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            /* 确定当前格子是黑还是白 */
            is_white = ((x / square_size) + (y / square_size)) % 2 == 0;

            if (is_white)
            {
                pixel[0] = 255; /* B */
                pixel[1] = 255; /* G */
                pixel[2] = 255; /* R */
            }
            else
            {
                pixel[0] = 0; /* B */
                pixel[1] = 0; /* G */
                pixel[2] = 0; /* R */
            }
        }
    }
}

/**
 * @brief 填充同心圆效果
 * @param image BMP图像指针
 */
void bmp_fill_circles(BMPImage *image)
{
    int width, height, x, y;
    uint8_t *pixel;
    int center_x, center_y, distance;
    float normalized_distance;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    width = image->info_header.width;
    height = abs(image->info_header.height);
    center_x = width / 2;
    center_y = height / 2;

    /* 计算最大距离（对角线的一半） */
    int max_distance = (int)sqrt(center_x * center_x + center_y * center_y);

    /* 填充同心圆效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            /* 计算到中心的距离 */
            distance = (int)sqrt((x - center_x) * (x - center_x) +
                                 (y - center_y) * (y - center_y));

            /* 归一化距离并创建颜色渐变 */
            normalized_distance = (float)distance / max_distance;

            /* 根据距离设置颜色 */
            if (distance % 20 < 10) /* 每20个像素一个圆环 */
            {
                pixel[0] = 255 * (1 - normalized_distance); /* B */
                pixel[1] = 128 * normalized_distance;       /* G */
                pixel[2] = 255 * normalized_distance;       /* R */
            }
            else
            {
                pixel[0] = 128 * normalized_distance;       /* B */
                pixel[1] = 255 * (1 - normalized_distance); /* G */
                pixel[2] = 192 * normalized_distance;       /* R */
            }
        }
    }
}

/**
 * @brief 填充噪点效果
 * @param image BMP图像指针
 * @param noise_level 噪点级别 (0-100)
 */
void bmp_fill_noise(BMPImage *image, int noise_level)
{
    int width, height, x, y;
    uint8_t *pixel;
    int base_value, noise;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 限制噪点级别范围 */
    if (noise_level < 0)
        noise_level = 0;
    if (noise_level > 100)
        noise_level = 100;

    width = image->info_header.width;
    height = abs(image->info_header.height);

    /* 根据噪点级别计算基础颜色值和噪声范围 */
    base_value = 128;
    noise = (255 * noise_level) / 100;

    /* 填充噪点效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            /* 添加随机噪声到基础颜色值 */
            pixel[0] = base_value + (rand() % (2 * noise + 1)) - noise; /* B */
            pixel[1] = base_value + (rand() % (2 * noise + 1)) - noise; /* G */
            pixel[2] = base_value + (rand() % (2 * noise + 1)) - noise; /* R */
        }
    }
}

/**
 * @brief 填充条纹效果
 * @param image BMP图像指针
 * @param stripe_width 条纹宽度
 */
void bmp_fill_stripes(BMPImage *image, int stripe_width)
{
    int width, height, x, y;
    uint8_t *pixel;
    int is_blue_stripe;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 设置默认条纹宽度 */
    if (stripe_width <= 0)
    {
        stripe_width = 15;
    }

    width = image->info_header.width;
    height = abs(image->info_header.height);

    /* 填充条纹效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            /* 确定当前是哪种颜色的条纹 */
            is_blue_stripe = ((x / stripe_width) % 2 == 0);

            /* 设置颜色：蓝色或红色条纹 */
            if (is_blue_stripe)
            {
                pixel[0] = 255; /* B */
                pixel[1] = 0;   /* G */
                pixel[2] = 0;   /* R */
            }
            else
            {
                pixel[0] = 0;   /* B */
                pixel[1] = 0;   /* G */
                pixel[2] = 255; /* R */
            }
        }
    }
}

/**
 * @brief 填充曼德布洛特分形效果
 * @param image BMP图像指针
 * @param zoom 缩放因子
 * @param center_x X轴中心位置 (-2.5 to 1.0)
 * @param center_y Y轴中心位置 (-1.0 to 1.0)
 */
void bmp_fill_mandelbrot(BMPImage *image, double zoom, double center_x, double center_y)
{
    int width, height, x, y;
    uint8_t *pixel;
    double x0, y0, x1, y1, x2, y2;
    int iteration, max_iteration;
    double color_scale;

    /* 检查参数有效性 */
    if (!image || !image->pixels)
    {
        print_error("Invalid BMP image pointer");
        return;
    }

    /* 设置默认值 */
    if (zoom <= 0.0)
        zoom = 1.0;
    if (center_x < -2.5 || center_x > 1.0)
        center_x = -0.5;
    if (center_y < -1.0 || center_y > 1.0)
        center_y = 0.0;

    width = image->info_header.width;
    height = abs(image->info_header.height);
    max_iteration = 100; /* 最大迭代次数 */

    /* 填充曼德布洛特分形效果 */
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            pixel = image->pixels + y * width * 3 + x * 3;

            /* 将像素坐标转换为复平面坐标 */
            x0 = (((double)x / width) - 0.5) * 3.5 / zoom + center_x;
            y0 = (((double)y / height) - 0.5) * 2.0 / zoom + center_y;

            x1 = 0.0;
            y1 = 0.0;
            iteration = 0;

            /* 迭代计算 */
            while (x1 * x1 + y1 * y1 <= 4.0 && iteration < max_iteration)
            {
                x2 = x1 * x1 - y1 * y1 + x0;
                y2 = 2 * x1 * y1 + y0;
                x1 = x2;
                y1 = y2;
                iteration++;
            }

            /* 根据迭代次数设置颜色 */
            if (iteration == max_iteration)
            {
                /* 集合内部点为黑色 */
                pixel[0] = 0;
                pixel[1] = 0;
                pixel[2] = 0;
            }
            else
            {
                /* 根据迭代速度设置颜色 */
                color_scale = (double)iteration / max_iteration;

                pixel[0] = (uint8_t)(255 * (1.0 - color_scale) * color_scale * 4.0); /* B */
                pixel[1] = (uint8_t)(255 * color_scale * color_scale);               /* G */
                pixel[2] = (uint8_t)(255 * (0.7 + 0.3 * sin(color_scale * 20.0)));   /* R */
            }
        }
    }
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
    int row, col;
    uint8_t *row_buffer = NULL;
    uint32_t row_stride;
    uint8_t padding_bytes[3] = {0, 0, 0}; /* 用于行填充的字节 */

    /* 检查参数有效性 */
    if (!image || !image->pixels || !filename)
    {
        print_error("Invalid parameters");
        return -1;
    }

    /* 检查BMP格式是否有效 */
    if (image->file_header.type != 0x4D42 ||
        image->info_header.bit_count != 24 ||
        image->info_header.width <= 0)
    {
        print_error("Invalid BMP format");
        return -1;
    }

    /* 打开文件 */
    if (!(fp = fopen(filename, "wb")))
    {
        print_error("Cannot create file");
        return -1;
    }

    /* 写入文件头和信息头 */
    if (fwrite(&image->file_header, 1, sizeof(BMPFileHeader), fp) != sizeof(BMPFileHeader) ||
        fwrite(&image->info_header, 1, sizeof(BMPInfoHeader), fp) != sizeof(BMPInfoHeader))
    {
        print_error("Failed to write headers");
        fclose(fp);
        return -1;
    }

    /* 计算行跨度 */
    row_stride = bmp_get_row_stride(image->info_header.width);

    /* 分配行缓冲区 */
    if (!(row_buffer = (uint8_t *)malloc(row_stride)))
    {
        print_error("Memory allocation failed");
        fclose(fp);
        return -1;
    }

    /* 写入像素数据 */
    const int width = image->info_header.width;
    const int height_abs = abs(image->info_header.height);
    const int row_padding = row_stride - width * 3;

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

        /* 写入一行数据 */
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
 * @brief 打印程序使用说明
 */
void print_usage()
{
    printf("Usage: ./bmp_generator -o output.bmp [options]\n");
    printf("Options:\n");
    printf("  -w width       : Image width (default: 800)\n");
    printf("  -h height      : Image height (default: 600)\n");
    printf("  -m mode        : Generation mode (default: 0)\n");
    printf("                   0 = Random pixels\n");
    printf("                   1 = Gradient\n");
    printf("                   2 = Checker pattern\n");
    printf("                   3 = Concentric circles\n");
    printf("                   4 = Noise\n");
    printf("                   5 = Stripes\n");
    printf("                   6 = Mandelbrot fractal\n");
    printf("  -s size        : Size parameter for patterns (default varies by mode)\n");
    printf("  -z zoom        : Zoom level for fractal (default: 1.0)\n");
    printf("  -x center_x    : X center for fractal (-2.5 to 1.0, default: -0.5)\n");
    printf("  -y center_y    : Y center for fractal (-1.0 to 1.0, default: 0.0)\n");
    printf("\nExamples:\n");
    printf("  ./bmp_generator -o random.bmp -w 1024 -h 768 -m 0\n");
    printf("  ./bmp_generator -o gradient.bmp -w 800 -h 600 -m 1\n");
    printf("  ./bmp_generator -o checker.bmp -w 500 -h 500 -m 2 -s 25\n");
    printf("  ./bmp_generator -o mandelbrot.bmp -w 1024 -h 1024 -m 6 -z 2.5 -x -0.75 -y 0.1\n");
}

/**
 * @brief 主函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 成功返回0，失败返回非0值
 */
int main(int argc, char *argv[])
{
    char *output_file = NULL;
    int width = 800;        /* 默认宽度 */
    int height = 600;       /* 默认高度 */
    int mode = MODE_RANDOM; /* 默认模式：随机像素 */
    int size_param = 0;     /* 默认大小参数 */
    double zoom = 1.0;      /* 默认缩放级别 */
    double center_x = -0.5; /* 默认X中心 */
    double center_y = 0.0;  /* 默认Y中心 */
    BMPImage *image = NULL;
    int i, ret = EXIT_FAILURE;

    /* 初始化随机数生成器 */
    srand((unsigned)time(NULL));

    /* 解析命令行参数 */
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-o") == 0)
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
        else if (strcmp(argv[i], "-w") == 0)
        {
            /* 宽度参数 */
            if (i + 1 < argc)
            {
                width = atoi(argv[++i]);
                if (width <= 0)
                {
                    fprintf(stderr, "Error: Invalid width value\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -w option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-h") == 0)
        {
            /* 高度参数 */
            if (i + 1 < argc)
            {
                height = atoi(argv[++i]);
                if (height <= 0)
                {
                    fprintf(stderr, "Error: Invalid height value\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -h option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-m") == 0)
        {
            /* 模式参数 */
            if (i + 1 < argc)
            {
                mode = atoi(argv[++i]);
                if (mode < 0 || mode > 6)
                {
                    fprintf(stderr, "Error: Invalid mode value (0-6)\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -m option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            /* 大小参数 */
            if (i + 1 < argc)
            {
                size_param = atoi(argv[++i]);
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -s option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-z") == 0)
        {
            /* 缩放参数 */
            if (i + 1 < argc)
            {
                zoom = atof(argv[++i]);
                if (zoom <= 0.0)
                {
                    fprintf(stderr, "Error: Invalid zoom value\n");
                    print_usage();
                    return EXIT_FAILURE;
                }
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -z option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-x") == 0)
        {
            /* X中心参数 */
            if (i + 1 < argc)
            {
                center_x = atof(argv[++i]);
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -x option\n");
                print_usage();
                return EXIT_FAILURE;
            }
        }
        else if (strcmp(argv[i], "-y") == 0)
        {
            /* Y中心参数 */
            if (i + 1 < argc)
            {
                center_y = atof(argv[++i]);
            }
            else
            {
                fprintf(stderr, "Error: Missing value after -y option\n");
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
    if (!output_file)
    {
        fprintf(stderr, "Error: Output file not specified\n");
        print_usage();
        return EXIT_FAILURE;
    }

    /* 创建BMP图像 */
    printf("Creating %dx%d BMP image in mode %d\n", width, height, mode);
    image = bmp_create(width, height);
    if (!image)
    {
        fprintf(stderr, "Error: Failed to create BMP image\n");
        return EXIT_FAILURE;
    }

    /* 根据选定的模式填充图像 */
    switch (mode)
    {
    case MODE_RANDOM:
        printf("Generating random pixel pattern...\n");
        bmp_fill_random(image);
        break;

    case MODE_GRADIENT:
        printf("Generating gradient pattern...\n");
        bmp_fill_gradient(image);
        break;

    case MODE_CHECKER:
        printf("Generating checker pattern...\n");
        bmp_fill_checker(image, size_param);
        break;

    case MODE_CIRCLES:
        printf("Generating concentric circles pattern...\n");
        bmp_fill_circles(image);
        break;

    case MODE_NOISE:
        printf("Generating noise pattern...\n");
        bmp_fill_noise(image, size_param > 0 ? size_param : 50);
        break;

    case MODE_STRIPES:
        printf("Generating stripes pattern...\n");
        bmp_fill_stripes(image, size_param);
        break;

    case MODE_MANDELBROT:
        printf("Generating Mandelbrot fractal...\n");
        bmp_fill_mandelbrot(image, zoom, center_x, center_y);
        break;

    default:
        fprintf(stderr, "Error: Invalid mode\n");
        bmp_free(image);
        return EXIT_FAILURE;
    }

    /* 写入输出文件 */
    printf("Writing output file: %s\n", output_file);
    if (bmp_write(image, output_file) != 0)
    {
        fprintf(stderr, "Error: Failed to write output file: %s\n", output_file);
        ret = EXIT_FAILURE;
    }
    else
    {
        printf("BMP image generated successfully!\n");
        ret = EXIT_SUCCESS;
    }

    /* 释放资源 */
    bmp_free(image);

    return ret;
}