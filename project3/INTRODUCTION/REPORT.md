# 1. REPORT

杨官宇涵12313614

由于设计到的代码文件较多，具体代码请参阅我的github

[YangGuanyuhan/CS219-project](https://github.com/YangGuanyuhan/CS219-project)

## 1.1 什么是bmp图像

##### 1.1.1 bmp图像相对陌生，大家平时主要用jpg，png为多，所以就需要去了解到bmp图像，bmp与其他常见格式的区别是什么

- BMP是英文Bitmap（位图）的简写，它是Windows操作系统中的标准图像文件格式，能够被多种Windows应用程序所支持。随着Windows操作系统的流行与丰富的Windows应用程序的开发，BMP位图格式理所当然地被广泛应用。这种格式的特点是包含的图像信息较丰富，几乎不进行压缩，

- bmp图像的原理：为了描述组成图像的像素色彩信息，需要知道图像的长、宽像素、像素位深等信息，最简单的方法就是将图像所有的像素信息按照行或者列排列方式保存成数组，然后按照图像的信息将数组刷新到屏幕就可以显示图像了，原理和点阵显示字符、汉字很相似，只不过图像是所有的点都需要显示（最方便的就是存储为二维结构，即使不需要边角的图像信息），而点阵字符显示则可能需要跳过一些点。



##### 1.1.2 BMP文件结构（设计struct的时候参考）

1.  位图文件头(bitmap-file header)（大小为14Byte）
2.  位图信息头(bitmap-informationheader)（大小为40Byte）
3.  颜色表(color table)
4.  颜色点阵数据(bits data)



##### 1.1.3 在电脑中可以找到一副bmp图像，去观察那些参数，具体感受bmp的理论知识

![](ReportPicture/bmp图像例子.png)

发现bmp图像中有两个比较重要的参数分别是像素密度和位深度，设计文件的时候就需要考虑到这个问题

1. 像素密度（Pixel Density）
像素密度描述的是图像中单位长度内的像素数量，通常用 **每米像素数（pixels per meter，ppm）** 或 **每英寸像素数（PPI，pixels per inch）** 表示。像素密度与图像的物理尺寸和显示质量有关。

2. 位深度（Bit Depth）
位深度表示的是每个像素所占用的比特数，用于定义图像的颜色和透明度信息。



##### 1.1.4 bmp与jpg，png等的图像具体区别在哪里。

| **特性**       | **BMP**                | **JPG**                | **PNG**                         |
| -------------- | ---------------------- | ---------------------- | ------------------------------- |
| **压缩方式**   | 无压缩                 | 有损压缩               | 无损压缩                        |
| **文件大小**   | 非常大                 | 较小                   | 适中                            |
| **图像质量**   | 无损，质量高           | 有损压缩，质量可能下降 | 无损，质量高                    |
| **透明度支持** | 不支持（部分扩展支持） | 不支持                 | 支持（Alpha 通道）              |
| **适用场景**   | 原始数据存储，图像编辑 | 照片存储，网页传输     | 图标、UI 设计，线条图，网页设计 |
| **兼容性**     | 非常高                 | 非常高                 | 高                              |



**总结**

1. bmp图像几乎不进行压缩，所需要的储存空间大，因此处理所需要的算法需要有效率，需要针对做一些特定的优化。
2. 设计struct的时候需要特别考虑这么大的图像的存储方式
3. 对于图像的加工，如果是互不关联，可以被串行化，在不违背amdahl law的情况下，如果使用到cpu的多核能够对于处理速度有质的飞跃
4. project要求程序应专注于处理 24 位未压缩的 BMP 文件，我们主要考虑24bit的情况

![](ReportPicture/bmp图像文件.png)



## 1.2 代码安全规范

在写完代码，经过初步的运行测试的时候，我将代码发给gpt，要求指出我代码中问题的，并给出完备性的修改

~~~
 1.注意代码的整体结构，符合开发规范。
 2.注意代码规范，实现良好的方法和类的使用，实现良好的模块化使得代码能够拥有良好的移植性。对于所有方法传入的参数都必须进行有效性验证
 3.保证代码的健壮性。处理好任何输入错误或边界条件。同时任何方法中都需要检查参数有效性 强调，对于任何指针操作，使用前都需要检查有效性
 4.安全性代码应尽量减少漏洞风险，比如防止 SQL 注入、跨站脚本攻击 (XSS) 等问题，
 5.对任何指针和内存的操作都需要检查有效性，对指针和内存的操作一定要安全，越保守越好。
~~~

同时我也让gpt给出xiang

## 1.3 代码架构设计

在deepseek的帮助下，实现了如下的代码的架构设计，我将根据这个架构去实现相关的方法和结构体的定义

```
            +---------------------+
            |  Command Line Parser |
            +----------+----------+
                       |
            +----------v----------+
            |   BMP File Loader    |
            +----------+----------+
                       |
            +----------v----------+
            | Image Processor     |
            | - Brightness         |
            | - Blending           |
            | - Custom Filters     |
            +----------+----------+
                       |
            +----------v----------+
            |  BMP File Writer     |
            +---------------------+
```

## 1.4 代码交互

#### 1.4.1 代码能够同时充分打印出错误信息，以及usage信息

~~~bash
christylinux@christywindowscomputer:~/CS219/project3/build$ ./standard_bmpedit 
Error: Missing required parameters
Usage: ./standard_bmpedit -i input.bmp [-i input2.bmp] -o output.bmp -op operation [value]
Operations:
  add value     : Adjust image brightness, positive value increases, negative decreases
  average       : Blend two images by averaging (requires two input files)
Examples:
  ./standard_bmpedit -i input.bmp -o output.bmp -op add 50
  ./standard_bmpedit -i input1.bmp -i input2.bmp -o output.bmp -op average
~~~

### 1.4.2 进行读取时能够打印当前的图像信息

~~~
christylinux@christywindowscomputer:~/CS219/project3$ ./build/standard_bmpedit -i bmptest.bmp -o dark_output.bmp -op add -50

----- BMP information for file bmptest.bmp -----
====== BMP File Information ======
File identifier: BM
File size: 6548618 bytes
Reserved field 1: 0
Reserved field 2: 0
Bitmap data offset: 138 bytes

====== BMP Image Information ======
Info header size: 124 bytes
Bitmap width: 1706 pixels
Bitmap height: 1279 pixels
Color planes: 1
Color depth: 24 bits/pixel
Compression type: 0
Image data size: 6548480 bytes
Horizontal resolution: 0 pixels/meter
Vertical resolution: 0 pixels/meter
Colors used: 0
Important colors: 0
============================

Adjusting brightness by -50...
Writing output file: dark_output.bmp
Operation completed successfully!
~~~



## 1.5 运行示例

#### 1.5.1 选取一副bmp图像作为测试用例

![](ReportPicture/屏幕截图 2025-04-13 135135.png)

![](ReportPicture/bmptest.bmp)



### 1.5.2 这是变亮后的图像

~~~
christylinux@christywindowscomputer:~/CS219/project3$ ./build/standard_bmpedit -i bmptest.bmp -o bright_output.bmp -op add 100

----- BMP information for file bmptest.bmp -----
====== BMP File Information ======
File identifier: BM
File size: 6548618 bytes
Reserved field 1: 0
Reserved field 2: 0
Bitmap data offset: 138 bytes

====== BMP Image Information ======
Info header size: 124 bytes
Bitmap width: 1706 pixels
Bitmap height: 1279 pixels
Color planes: 1
Color depth: 24 bits/pixel
Compression type: 0
Image data size: 6548480 bytes
Horizontal resolution: 0 pixels/meter
Vertical resolution: 0 pixels/meter
Colors used: 0
Important colors: 0
============================

Adjusting brightness by 100...
Writing output file: bright_output.bmp
Operation completed successfully!
~~~

![](ReportPicture/bright_output.bmp)



### 1.5.3 这是变暗的图像

![](ReportPicture/dark_output.bmp)



### 1.5.4 这是将变亮和变暗后融合的图像

~~~
christylinux@christywindowscomputer:~/CS219/project3$ ./build/standard_bmpedit -i bright_output.bmp -i dark_output.bmp -o output.bmp -op average

----- BMP information for file bright_output.bmp -----
====== BMP File Information ======
File identifier: BM
File size: 6548618 bytes
Reserved field 1: 0
Reserved field 2: 0
Bitmap data offset: 138 bytes

====== BMP Image Information ======
Info header size: 124 bytes
Bitmap width: 1706 pixels
Bitmap height: 1279 pixels
Color planes: 1
Color depth: 24 bits/pixel
Compression type: 0
Image data size: 6548480 bytes
Horizontal resolution: 0 pixels/meter
Vertical resolution: 0 pixels/meter
Colors used: 0
Important colors: 0
============================


----- BMP information for file dark_output.bmp -----
====== BMP File Information ======
File identifier: BM
File size: 6548618 bytes
Reserved field 1: 0
Reserved field 2: 0
Bitmap data offset: 138 bytes

====== BMP Image Information ======
Info header size: 124 bytes
Bitmap width: 1706 pixels
Bitmap height: 1279 pixels
Color planes: 1
Color depth: 24 bits/pixel
Compression type: 0
Image data size: 6548480 bytes
Horizontal resolution: 0 pixels/meter
Vertical resolution: 0 pixels/meter
Colors used: 0
Important colors: 0
============================

Blending two images...
Writing output file: output.bmp
Operation completed successfully!
~~~

![](ReportPicture/output.bmp)

这是原本的图像

![](ReportPicture/bmptest.bmp)

发现有较大的差别

其根本原因是对rgb操作的时候发生了溢出，但是时satuating的溢出，使得出现了信息丢失，降低了图片的动态范围

在变亮/变暗过程中由于像素截断，信息不可逆丢失，平均操作无法还原原始图像。

### 1.5.5 这是灰度的图像

![](ReportPicture/grayscale_output.bmp)

这是模糊的图像

![](ReportPicture/blur_output.bmp)

### 1.5.6 这是锐化的图像

![](ReportPicture/sharpen_output.bmp)

##  1.6 初始的结构体定义

### 1.6.1 内存对齐

struct中有严格的内存align方式来提高缓存命中，但是BMP文件格式有严格的字节布局，所以考虑到内存的布局，使用了#pragma pack(push, 1)与 #pragma pack(pop)来对于BMPFileHeader和BMPInfoHeader来实现两个结构体中没有空隙

~~~c
#pragma pack(push, 1)
typedef struct
{

} BMPFileHeader;


typedef struct
{
 
} BMPInfoHeader;
#pragma pack(pop)
~~~

### 1.6.2 struct设计

##### 1.6.2.1 BMPFileHeader

```c
#pragma pack(push, 1) // 保证结构体按1字节紧凑对齐
typedef struct
{
    uint16_t type;      // 2 字节，固定值 'BM'，表示是 BMP 文件（0x4D42）
    uint32_t size;      // 4 字节，整个 BMP 文件的大小（单位：字节）
    uint16_t reserved1; // 2 字节，保留字段，通常为 0
    uint16_t reserved2; // 2 字节，保留字段，通常为 0
    uint32_t offset;    // 4 字节，图像数据相对于文件起始位置的偏移量
} BMPFileHeader;
```
##### 1.6.2.2 BMPInfoHeader
```c
typedef struct
{
    uint32_t size;              // 4 字节，整个 InfoHeader 的大小（通常为 40）
    int32_t width;              // 4 字节，图像宽度（像素数）
    int32_t height;             // 4 字节，图像高度（像素数）
    uint16_t planes;            // 2 字节，颜色平面数，固定为 1
    uint16_t bit_count;         // 2 字节，每个像素的位数（比如 24 表示 RGB 各8位）
    uint32_t compression;       // 4 字节，压缩方式（0 表示不压缩）
    uint32_t image_size;        // 4 字节，图像数据部分的大小（可为 0）
    int32_t x_pixels_per_meter; // 4 字节，水平分辨率（像素/米）
    int32_t y_pixels_per_meter; // 4 字节，垂直分辨率（像素/米）
    uint32_t colors_used;       // 4 字节，调色板中使用的颜色数（0 表示全部）
    uint32_t colors_important;  // 4 字节，重要颜色数（一般设为 0）
} BMPInfoHeader;
```

### 1.6.3 BMPImage（完整的bmp图像）

~~~c
typedef struct
{
    BMPFileHeader file_header; /* 文件头 */
    BMPInfoHeader info_header; /* 信息头 */
    uint8_t *pixels;           /* 像素数据，BGR格式，每像素3字节，是一个指针，指向图像实际的像素数据；*/
} BMPImage;

~~~

## 1.7 图像读取，加工处理，和输出

### 1.7.1 main函数

main函数解析命令行参数（-i输入文件，-o输出文件，-op操作类型），读取BMP图像文件，执行指定的图像处理操作，将处理结果保存到输出文件，并进行完整的错误处理和内存资源管理。特别注意处理 `result` 与 `images[0]` 可能指向同一内存的情况，避免重复释放。

### 1.7.2 读取图像函数（bmp_read）

对指向的内存区域进行清空操作

~~~c
 /* 初始化为零 */
memset(image, 0, sizeof(BMPImage));
~~~

分配BMP图像结构内存，像素数据是以指针的形式存储

~~~c
image = (BMPImage *)malloc(sizeof(BMPImage));
~~~

读取文件头

~~~
/* 读取文件头 */
    read_size = fread(&image->file_header, sizeof(BMPFileHeader), 1, fp);
    if (read_size != 1)
    {
        print_error("Failed to read file header");
        free(image);
        fclose(fp);
        return NULL;
    }
~~~

 分配像素数据内存 

~~~
    /* 分配像素数据内存 */
    image->pixels = (uint8_t *)malloc(image->info_header.width * abs(image->info_header.height) * 3);
    if (image->pixels == NULL)
    {
        print_error("Memory allocation failed");
        free(image);
        fclose(fp);
        return NULL;
    }
~~~

通过信息头定位到有效的位置

~~~、
if (fseek(fp, image->file_header.offset, SEEK_SET) != 0)
    {
        print_error("File seek failed");
        free(row_buffer);
        free(image->pixels);
        free(image);
        fclose(fp);
        return NULL;
    }

~~~

使用buffer接受完整一行的数据，然后通过for循环分配，可以跳过填充字节

~~~
row_buffer = (uint8_t *)malloc(row_stride);
~~~

~~~
 /* 复制像素数据到图像结构（不包括填充字节） */
        for (col = 0; col < image->info_header.width; col++)
        {
            int pixel_index = (current_row * image->info_header.width + col) * 3;
            image->pixels[pixel_index] = row_buffer[col * 3];         /* B */
            image->pixels[pixel_index + 1] = row_buffer[col * 3 + 1]; /* G */
            image->pixels[pixel_index + 2] = row_buffer[col * 3 + 2]; /* R */
        }
~~~

### 1.7.3 加工处理函数（后面还有一个优化的版本）

#### 1.7.3.1 bmp_adjust_brightness

简单的for循环，避免溢出，调整亮度

~~~
/* 计算总像素数 */
    total_pixels = image->info_header.width * abs(image->info_header.height) * 3;

    /* 调整每个像素的亮度 */
    for (i = 0; i < total_pixels; i++)
    {
~~~

#### 1.7.3.2 bmp_average

检查两个图像是否具有相同的尺寸，否则无法进行合并

头信息选择第一个图像的

混合两个图像，也就是简单平均

~~~
result->pixels[i] = (uint8_t)(((int)image1->pixels[i] + (int)image2->pixels[i]) / 2);
~~~

#### 1.7.3.3 灰度化函数

~~~
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
~~~

#### 1.7.3.4 对比度调整功能bmp_adjust_contrast

~~~
/* 调整每个像素的对比度 */
    for (i = 0; i < total_pixels; i++)
    {
        /* 将像素值调整到以128为中心 */
        pixel_value = image->pixels[i] - 128;

        /* 应用对比度因子 */
        pixel_value = pixel_value * factor + 128;
~~~

### 1.7.4 卷积滤波功能

卷积滤波功能，在ai的帮助下实现

~~~
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
~~~



### 1.7.5 输出图像函数（bmp_write）

计算行跨度（含填充）

~~~
row_stride = bmp_get_row_stride(image->info_header.width);
row_padding = row_stride - image->info_header.width * 3;

~~~

写入 BMP 文件头与信息头

~~~
fwrite(&image->file_header, sizeof(BMPFileHeader), 1, fp);
fwrite(&image->info_header, sizeof(BMPInfoHeader), 1, fp);

~~~

分配行缓冲区

~~~
row_buffer = (uint8_t *)malloc(row_stride);

~~~

逐行写入文件数据到buffer

~~~
for (col = 0; col < image->info_header.width; col++)
        {
            int pixel_index = (current_row * image->info_header.width + col) * 3;
            row_buffer[col * 3] = image->pixels[pixel_index];         /* B */
            row_buffer[col * 3 + 1] = image->pixels[pixel_index + 1]; /* G */
            row_buffer[col * 3 + 2] = image->pixels[pixel_index + 2]; /* R */
        }
~~~

写入像素数据

~~~
fwrite(row_buffer, 1, (size_t)(image->info_header.width * 3), fp)

~~~

补齐padding

~~~
if (row_padding > 0)
{
    fwrite(padding_bytes, 1, (size_t)row_padding, fp);
}
~~~



## 1.8 代码优化部分

### 1.8.1 optimal_bmpedit.c，优化部分

**多线程处理**：使用pthread库创建多个线程同时处理不同区域的图像数据

~~~
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
~~~

**SIMD指令集优化**：利用SSE指令集进行向量化操作，一次处理多个像素，显著提升像素处理速度。

~~~
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
~~~

**算法优化**：

   - 使用位运算替代除法操作（例如除以2用右移1位）
   - 使用条件操作符替代if-else分支，减少分支预测失败
   - 内联小函数减少函数调用开销
**IO优化**：
   - 优化文件读写，减少IO操作次数
   - 一次性读取文件头结构体，减少系统调用

**智能功能选择**：

   - 根据图像大小自动选择最佳处理方法（小图像使用SIMD，大图像使用多线程+SIMD）
   - 自适应线程数量，根据CPU核心数优化线程创建

~~~
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
~~~

**使用inline函数**

~~~
static inline int bmp_is_valid(const BMPImage *image)
~~~

由编译器决定是否优化

 **使用指针算法优化像素数据复制**

​    uint8_t *dst_ptr = image->pixels + current_row * width * 3;

​    uint8_t *src_ptr = row_buffer;

### 1.8.2 随机图像生成器

使用c语言完成了随机的图像生成器，能够生成类似这种的图像，便于后面的性能测试

![](ReportPicture/random.bmp)

完成了一个脚本用于测试优化后的程序和优化前程序的时间，计时分为三个时间，读取bmp文件时间，加工bmp文件时间，写入文件时间

~~~
christylinux@christywindowscomputer:~/CS219/project3/test$ cd /home/christylinux/CS219/project3/test && ./run_batch_test.sh
Testing image size: 512x512
Extracted timing values:
Standard: read=3.715 ms, process=3.054 ms, write=9.375 ms, total=16.151 ms
Optimized: read=0.661 ms, process=0.125 ms, write=8.156 ms, total=8.947 ms
Completed test for 512x512
----------------------------------------
Testing image size: 1024x1024
awk: cmd. line:1: fatal: division by zero attempted
Extracted timing values:
Standard: read=17.686 ms, process=13.227 ms, write=23.671 ms, total=54.587 ms
Optimized: read=1.222 ms, process=1.474 ms, write=4.013 ms, total=6.714 ms
Completed test for 1024x1024
----------------------------------------
~~~

| image_size | std_read | std_process | std_write | std_total | opt_read | opt_process | opt_write | opt_total |
| ---------- | -------- | ----------- | --------- | --------- | -------- | ----------- | --------- | --------- |
| 512        | 4.122    | 3.004       | 6.400     | 13.530    | 0.352    | 0.084       | 0.865     | 1.303     |
| 1024       | 16.126   | 12.451      | 23.306    | 51.889    | 0.855    | 1.553       | 4.322     | 6.733     |
| 2048       | 51.754   | 42.372      | 86.354    | 180.486   | 4.215    | 3.322       | 19.950    | 27.492    |
| 4096       | 213.043  | 199.272     | 349.650   | 761.973   | 14.258   | 13.548      | 72.100    | 99.915    |
| 8192       | 851.216  | 717.143     | 1061.410  | 2629.774  | 43.909   | 41.379      | 257.255   | 342.548   |

![](ReportPicture/RunningTime.png)

为了使得输出明显，定义了标准方法与优化方法在各处理阶段的时间比值表（标准时间 / 优化时间）也就是performance的比较

下是标准方法与优化方法在各处理阶段的时间比值表（标准时间 / 优化时间）：

| image_size | read_ratio | process_ratio | write_ratio | total_ratio |
| ---------- | ---------- | ------------- | ----------- | ----------- |
| 512        | 11.71      | 35.76         | 7.40        | 10.38       |
| 1024       | 18.86      | 8.02          | 5.39        | 7.71        |
| 2048       | 12.28      | 12.75         | 4.33        | 6.57        |
| 4096       | 14.94      | 14.71         | 4.85        | 7.63        |
| 8192       | 19.39      | 17.33         | 4.13        | 7.68        |

![](ReportPicture/Performance.png)

分析图像我们可以发现

1. 小图像在处理阶段提升特别大，而写入部分的提升相对稳定。
2. 优化方案在大尺寸图像的读取效率更高，因为使用了更快的 I/O 操作或更有效的缓存机制。
3. 在小图像时优化极其显著，表明标准方法在小数据量处理上存在较大开销
4. 总体而言，优化方案将整个流程的效率提高了 6～10 倍，随着图像尺寸增大，优势更加稳定



由于对于标准的代码也使用了部分的优化策略，所以总体没有课上使用simd提升100倍的图像大，但是总体而言还是不错的性能提升



## 1.9 代码错误处理

由于完成了多种复杂的错误处理输出，所以我让ai整理出了整体的错误处理信息表格，以供参考

| 函数名 / 模块           | 错误触发条件                           | 错误信息输出（print_error）                      | 错误处理操作                                         |
| ----------------------- | -------------------------------------- | ------------------------------------------------ | ---------------------------------------------------- |
| `main`                  | 缺少参数或参数格式错误                 | 无（直接调用 `print_usage()`）                   | 显示使用说明后返回 1                                 |
| `bmp_read`              | 文件名为 `NULL`                        | `Filename is NULL`                               | 返回 NULL                                            |
|                         | 文件打开失败                           | `Cannot open file`                               | 返回 NULL                                            |
|                         | BMPImage 分配失败                      | `Memory allocation failed`                       | 关闭文件，返回 NULL                                  |
|                         | 文件头读取失败                         | `Failed to read file header`                     | 释放内存、关闭文件，返回 NULL                        |
|                         | 信息头读取失败                         | `Failed to read info header`                     | 释放内存、关闭文件，返回 NULL                        |
|                         | 非 BMP 文件（type != 0x4D42）          | `Not a valid BMP file`                           | 释放内存、关闭文件，返回 NULL                        |
|                         | 非 24 位 BMP                           | `Only 24-bit BMP images are supported`           | 释放内存、关闭文件，返回 NULL                        |
|                         | 非未压缩格式                           | `Only uncompressed BMP images are supported`     | 释放内存、关闭文件，返回 NULL                        |
|                         | 像素内存分配失败                       | `Memory allocation failed`                       | 释放 BMPImage 结构、关闭文件，返回 NULL              |
|                         | 行缓冲分配失败                         | `Memory allocation failed`                       | 释放像素内存和 BMPImage，关闭文件，返回 NULL         |
|                         | fseek 定位失败                         | `File seek failed`                               | 释放行缓冲、像素内存和 BMPImage，关闭文件，返回 NULL |
|                         | 行数据读取失败                         | `Failed to read pixel data`                      | 释放行缓冲、像素内存和 BMPImage，关闭文件，返回 NULL |
| `bmp_write`             | 输入参数无效（image 或 filename 为空） | `Invalid parameters`                             | 返回 -1                                              |
|                         | 图像无效（调用 bmp_is_valid 返回 0）   | 由 `bmp_is_valid` 打印原因                       | 返回 -1                                              |
|                         | 文件无法创建                           | `Cannot create file`                             | 返回 -1                                              |
|                         | 文件头写入失败                         | `Failed to write file header`                    | 关闭文件，返回 -1                                    |
|                         | 信息头写入失败                         | `Failed to write info header`                    | 关闭文件，返回 -1                                    |
|                         | 行缓冲分配失败                         | `Memory allocation failed`                       | 关闭文件，返回 -1                                    |
|                         | fseek 定位失败                         | `File seek failed`                               | 释放行缓冲、关闭文件，返回 -1                        |
|                         | 行数据写入失败                         | `Failed to write pixel data`                     | 释放行缓冲、关闭文件，返回 -1                        |
|                         | 填充字节写入失败                       | `Failed to write padding bytes`                  | 释放行缓冲、关闭文件，返回 -1                        |
| `bmp_is_valid`          | 图像指针为空                           | `Invalid BMP pointer`                            | 返回 0                                               |
|                         | 文件标识非 "BM"                        | `Not a valid BMP file identifier`                | 返回 0                                               |
|                         | 非 24 位                               | `Only 24-bit BMP format is supported`            | 返回 0                                               |
|                         | 有压缩格式                             | `Only uncompressed BMP format is supported`      | 返回 0                                               |
|                         | 图像宽或高非法                         | `Invalid image dimensions`                       | 返回 0                                               |
|                         | 像素数据指针为空                       | `Invalid pixel data pointer`                     | 返回 0                                               |
| `bmp_adjust_brightness` | 图像或像素数据为空                     | `Invalid BMP image pointer`                      | 直接返回                                             |
| `bmp_average`           | 两个图像指针或像素为空                 | `Invalid BMP image pointer`                      | 返回 NULL                                            |
|                         | 图像大小不同                           | `Images have different dimensions, cannot blend` | 返回 NULL                                            |
|                         | 分配结果图像失败                       | `Memory allocation failed`                       | 返回 NULL                                            |
|                         | 分配结果像素数据失败                   | `Memory allocation failed`                       | 释放结果图像结构，返回 NULL                          |
| `bmp_print_info`        | 图像指针为空                           | `Invalid BMP image pointer`                      | 直接返回                                             |

## 1.10 总结

我本身是一个摄影爱好者，之前对于图片的了解停留在摄影和调色方面。平时会追求对动辄100多MB的raw格式的照片进行调色处理。我现在认识到小小的相机中一个全画幅的cmos能够在快门按下的瞬间，完成对于14bit色深的记录，完成这么多恐怖数据的传输，这是人类工业上的一颗明珠。同时也对于hdr等的技术有了更深的认识。

同时我的程序有很多不足，我在想对于调色，如何通过一个程序读取到色彩曲线进行调色。蒙版又是如何在程序中体现，在算法上，在数据结构上，蒙版又是怎么一回事？

这是一个有收获的project，无论是对于simd和多线程的了解，还是对基本的图像的了解我都能够收获颇多

## 1.11 引用

GitHub. (n.d.). *Copilot*. GitHub. Retrieved March 30, 2025, from https://github.com/settings/copilot

OpenAI. (n.d.). *ChatGPT*. OpenAI. Retrieved March 30, 2025, from https://chatgpt.com/

知乎用户. (2020, November 6). *BMP图像文件完全解析*. 知乎. https://zhuanlan.zhihu.com/p/260702527

weixin_43455834. (2020, May 26). *图像基础：BMP、RGB、JPG、PNG等格式详解（一）_bmp图像和jpeg图像的组成元素*. CSDN博客. https://blog.csdn.net/weixin_43455834/article/details/105988433
