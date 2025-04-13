# BMP 图像编辑工具报告

## 1. 简介

`standard_bmpedit_combined` 是一个简单而强大的命令行工具，用于处理和编辑 BMP 图像文件。该工具支持以下功能：

- 读取和写入 24 位未压缩 BMP 图像
- 调整图像亮度
- 通过平均值混合两个图像

该工具遵循良好的编程实践，具有高度的健壮性、安全性和可维护性。

## 2. 实现方法

### 2.1 代码结构

代码采用模块化设计，主要包含以下功能模块：

- **BMP 文件处理**：读取、验证和写入 BMP 文件
- **图像处理**：提供亮度调整和图像混合等功能
- **命令行界面**：解析用户输入的参数并调用相应的处理功能

### 2.2 核心数据结构

```c
/* BMP 文件头结构 */
typedef struct {
    uint16_t type;      /* 'BM' - BMP 文件标识符 */
    uint32_t size;      /* 文件大小 */
    uint16_t reserved1; /* 保留字段 1 */
    uint16_t reserved2; /* 保留字段 2 */
    uint32_t offset;    /* 像素数据偏移量 */
} BMPFileHeader;

/* BMP 信息头结构 */
typedef struct {
    uint32_t size;              /* 信息头大小 */
    int32_t width;              /* 图像宽度 */
    int32_t height;             /* 图像高度 */
    uint16_t planes;            /* 颜色平面，必须为 1 */
    uint16_t bit_count;         /* 每像素位数，此程序仅处理 24 位 */
    uint32_t compression;       /* 压缩方法，此程序仅处理 0（无压缩）*/
    uint32_t image_size;        /* 图像数据大小 */
    int32_t x_pixels_per_meter; /* 水平分辨率 */
    int32_t y_pixels_per_meter; /* 垂直分辨率 */
    uint32_t colors_used;       /* 使用的颜色数 */
    uint32_t colors_important;  /* 重要的颜色数 */
} BMPInfoHeader;

/* BMP 图像结构 */
typedef struct {
    BMPFileHeader file_header; /* 文件头 */
    BMPInfoHeader info_header; /* 信息头 */
    uint8_t *pixels;           /* 像素数据，BGR 格式，每像素 3 字节 */
} BMPImage;
```

### 2.3 主要功能实现

#### 2.3.1 图像读取与写入

- **`bmp_read()`**: 从文件读取 BMP 图像，处理 BMP 特有的行填充和自下而上的存储方式
- **`bmp_write()`**: 将处理后的图像写入文件，确保符合 BMP 文件格式规范

#### 2.3.2 图像处理

- **`bmp_adjust_brightness()`**: 通过增加或减少像素值来调整图像亮度，同时防止溢出
- **`bmp_average()`**: 通过对对应像素值取平均来混合两个图像

## 3. 代码安全性

### 3.1 输入验证

- 对所有函数参数进行有效性检查
- 验证 BMP 文件格式是否符合要求（24位、未压缩）
- 对用户输入的命令行参数进行严格验证

### 3.2 内存安全

- 所有内存分配后都进行有效性检查
- 操作前验证指针的有效性
- 安全释放所有分配的资源，避免内存泄漏
- 防止重复释放同一内存区域

### 3.3 错误处理

- 使用详细的错误信息
- 出错时安全退出并释放已分配的资源
- 使用结构化的错误处理流程，避免使用 goto 语句
- 处理文件操作的各种可能失败情况

### 3.4 边界检查

- 在亮度调整时防止像素值溢出（保持在 0-255 范围内）
- 图像混合前确保两个图像尺寸相同
- 处理文件读写时的边界情况

## 4. 使用说明

### 4.1 编译

使用项目根目录中的 Makefile 编译程序：

```bash
make
```

### 4.2 命令行参数

程序接受以下命令行参数：

```
./standard_bmpedit -i input.bmp [-i input2.bmp] -o output.bmp -op operation [value]
```

参数说明：
- `-i input.bmp`: 输入 BMP 文件，可以指定一个或两个输入文件
- `-o output.bmp`: 输出 BMP 文件
- `-op operation`: 要执行的操作，可以是 `add` 或 `average`
- `value`: 仅用于 `add` 操作，表示亮度调整值

### 4.3 操作类型

#### 亮度调整 (add)

调整图像的亮度，正值增加亮度，负值减少亮度。例如：

```bash
./standard_bmpedit -i bmptest.bmp -o bright_bmptest.bmp -op add 50
```

此命令将 `bmptest.bmp` 的亮度提高 50 点，并将结果保存到 `bright_bmptest.bmp`。

#### 图像混合 (average)

通过对每个像素值取平均值来混合两个图像。例如：

```bash
./standard_bmpedit -i bmptest.bmp -i dark_bmptest.bmp -o average_bmptest.bmp -op average
```

此命令将 `bmptest.bmp` 和 `dark_bmptest.bmp` 混合，并将结果保存到 `average_bmptest.bmp`。

### 4.4 示例

1. 增加亮度：
   ```bash
   ./standard_bmpedit -i bmptest.bmp -o bright_bmptest.bmp -op add 50
   ```

2. 降低亮度：
   ```bash
   ./standard_bmpedit -i bmptest.bmp -o dark_bmptest.bmp -op add -50
   ```

3. 混合两个图像：
   ```bash
   ./standard_bmpedit -i bmptest.bmp -i dark_bmptest.bmp -o average_bmptest.bmp -op average
   ```

## 5. 结论

`standard_bmpedit_combined` 工具提供了一种简单而有效的方式来处理 BMP 图像。该工具遵循良好的编程实践，确保代码的健壮性、安全性和可维护性。通过严格的输入验证、内存安全性考虑和结构化的错误处理，该工具能够安全地处理用户输入和各种边界情况。