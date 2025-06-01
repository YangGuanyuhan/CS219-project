#ifndef OPTIMAL_IMAGE_H
#define OPTIMAL_IMAGE_H

#include <cstddef>
#include <memory>
#include <string>
#include <stdexcept>
#include <atomic>
#include <vector>

// 平台检测宏
#if defined(_MSC_VER) // Windows with MSVC
#include <intrin.h>
#define OPT_WINDOWS
#elif defined(__GNUC__) || defined(__clang__) // GCC or Clang
#include <immintrin.h>
#define OPT_UNIX
#endif

namespace mylib
{
    // 前向声明
    class ImageDataManager;

    /**
     * @brief 一个优化的图像处理类，参考OpenCV的设计理念，支持数据共享和SIMD加速
     */
    class OptimalImage
    {
    public:
        /**
         * @brief 默认构造函数
         */
        OptimalImage();

        /**
         * @brief 创建指定大小和通道数的图像
         * @param width 图像宽度
         * @param height 图像高度
         * @param channels 通道数（例如：1为灰度图，3为RGB，4为RGBA）
         * @throw std::invalid_argument 如果参数无效
         */
        OptimalImage(int width, int height, int channels);

        /**
         * @brief 从文件加载图像的构造函数
         * @param filename 图像文件路径
         * @throw mylib::InvalidArgumentException 如果文件路径为空
         * @throw mylib::OperationFailedException 如果文件加载失败或格式不支持
         */
        explicit OptimalImage(const std::string &filename);

        /**
         * @brief 拷贝构造函数 - 创建引用同一数据的图像
         * @param other 要拷贝的图像
         */
        OptimalImage(const OptimalImage &other);

        /**
         * @brief 移动构造函数
         * @param other 要移动的图像
         */
        OptimalImage(OptimalImage &&other) noexcept;

        /**
         * @brief 拷贝赋值运算符 - 创建引用同一数据的图像
         * @param other 要拷贝的图像
         * @return 对this的引用
         */
        OptimalImage &operator=(const OptimalImage &other);

        /**
         * @brief 移动赋值运算符
         * @param other 要移动的图像
         * @return 对this的引用
         */
        OptimalImage &operator=(OptimalImage &&other) noexcept;

        /**
         * @brief 析构函数
         */
        ~OptimalImage();

        /**
         * @brief 获取图像宽度
         * @return 图像宽度
         */
        int width() const;

        /**
         * @brief 获取图像高度
         * @return 图像高度
         */
        int height() const;

        /**
         * @brief 获取图像通道数
         * @return 图像通道数
         */
        int channels() const;

        /**
         * @brief 获取图像大小（宽度 * 高度 * 通道数）
         * @return 图像大小（字节数）
         */
        size_t size() const;

        /**
         * @brief 获取一行的步长（宽度 * 通道数）
         * @return 一行的步长（字节数）
         */
        size_t step() const;

        /**
         * @brief 判断图像是否为空
         * @return 如果图像为空，返回true；否则返回false
         */
        bool empty() const;

        /**
         * @brief 获取图像数据的指针
         * @return 图像数据的指针
         */
        unsigned char *data();

        /**
         * @brief 获取图像数据的常指针
         * @return 图像数据的常指针
         */
        const unsigned char *data() const;

        /**
         * @brief 访问指定位置的像素值
         * @param row 行索引
         * @param col 列索引
         * @param channel 通道索引
         * @return 指定位置的像素值的引用
         * @throw std::out_of_range 如果索引超出范围
         */
        unsigned char &at(int row, int col, int channel = 0);

        /**
         * @brief 访问指定位置的像素值（常量版本）
         * @param row 行索引
         * @param col 列索引
         * @param channel 通道索引
         * @return 指定位置的像素值的常引用
         * @throw std::out_of_range 如果索引超出范围
         */
        const unsigned char &at(int row, int col, int channel = 0) const;

        /**
         * @brief 创建一个新的图像
         * @param width 图像宽度
         * @param height 图像高度
         * @param channels 通道数
         * @return 是否创建成功
         * @throw std::invalid_argument 如果参数无效
         */
        void create(int width, int height, int channels);

        /**
         * @brief 从文件加载图像
         * @param filename 图像文件路径
         * @throw mylib::InvalidArgumentException 如果文件路径为空
         * @throw mylib::OperationFailedException 如果文件加载失败或格式不支持
         */
        void load(const std::string &filename);

        /**
         * @brief 将图像保存到文件
         * @param filename 保存的文件路径 (例如 "output.png", "image.jpg", "result.bmp")
         *                 文件格式将根据扩展名自动推断。
         *                 支持的格式: PNG, JPG, BMP.
         * @throw mylib::InvalidArgumentException 如果文件名为空或不支持的文件扩展名
         * @throw mylib::OperationFailedException 如果图像为空或保存失败
         */
        void save(const std::string &filename) const;

        /**
         * @brief 释放图像数据
         */
        void release();

        /**
         * @brief 深拷贝图像，创建独立的数据副本
         * @return 拷贝后的新图像
         */
        OptimalImage clone() const;

        /**
         * @brief 确保数据独占访问权，如果数据被多个图像共享，则创建数据副本
         * 当需要修改图像数据时，应先调用此方法以避免影响其他引用相同数据的图像
         */
        void copyOnWrite();

        /**
         * @brief 获取当前图像数据的引用计数
         * @return 引用计数
         */
        int refCount() const;

        /**
         * @brief 调整图像亮度，SIMD和OpenMP优化版
         * @param delta 亮度增量，取值范围[-255, 255]
         * @throw std::invalid_argument 如果参数无效
         */
        void adjustBrightness(int delta);

        /**
         * @brief 静态方法：将两张图像混合，SIMD和OpenMP优化版
         * @param img1 第一张图像
         * @param img2 第二张图像
         * @param alpha 混合比例，取值范围[0, 1]
         * @return 混合后的新图像
         * @throw std::invalid_argument 如果参数无效或两张图像大小不一致
         */
        static OptimalImage blend(const OptimalImage &img1, const OptimalImage &img2, float alpha);

        /**
         * @brief 高斯模糊，使用SIMD和OpenMP优化
         * @param kernelSize 卷积核大小（必须是奇数，如3、5、7等）
         * @param sigma 高斯函数的标准差
         * @return 模糊后的新图像
         * @throw std::invalid_argument 如果参数无效
         */
        OptimalImage gaussianBlur(int kernelSize, double sigma) const;

        /**
         * @brief 检测CPU支持的SIMD指令集
         * @return 支持的SIMD指令集名称字符串
         */
        static std::string getSIMDInfo();

    private:
        std::shared_ptr<ImageDataManager> dataManager_; // 数据管理器，负责图像数据存储和引用计数
        int width_;                                     // 图像宽度
        int height_;                                    // 图像高度
        int channels_;                                  // 通道数
        size_t step_;                                   // 步长（每行字节数）

        /**
         * @brief 检查索引是否有效
         * @param row 行索引
         * @param col 列索引
         * @param channel 通道索引
         * @throw std::out_of_range 如果索引超出范围
         */
        void checkRange(int row, int col, int channel) const;
    };

    /**
     * @brief 图像数据管理类，负责图像数据的存储和引用计数
     */
    class ImageDataManager
    {
    public:
        /**
         * @brief 创建指定大小的数据管理器
         * @param dataSize 数据大小
         */
        explicit ImageDataManager(size_t dataSize);

        /**
         * @brief 析构函数
         */
        ~ImageDataManager();

        /**
         * @brief 获取数据指针
         * @return 数据指针
         */
        unsigned char *data();

        /**
         * @brief 获取数据常指针
         * @return 数据常指针
         */
        const unsigned char *data() const;

        /**
         * @brief 获取数据大小
         * @return 数据大小（字节数）
         */
        size_t size() const;

        /**
         * @brief 增加引用计数
         */
        void addRef();

        /**
         * @brief 减少引用计数
         * @return 减少后的引用计数
         */
        int release();

        /**
         * @brief 获取当前引用计数
         * @return 引用计数
         */
        int refCount() const;

    private:
        std::unique_ptr<unsigned char[]> data_; // 图像数据
        size_t size_;                           // 数据大小
        std::atomic<int> refCount_;             // 引用计数
    };

    /**
     * @brief 图像处理库异常基类
     */
    class ImageException : public std::runtime_error
    {
    public:
        explicit ImageException(const std::string &message) : std::runtime_error(message) {}
    };

    /**
     * @brief 图像参数无效异常
     */
    class InvalidArgumentException : public ImageException
    {
    public:
        explicit InvalidArgumentException(const std::string &message)
            : ImageException("Invalid argument: " + message) {}
    };

    /**
     * @brief 图像索引越界异常
     */
    class OutOfRangeException : public ImageException
    {
    public:
        explicit OutOfRangeException(const std::string &message)
            : ImageException("Index out of range: " + message) {}
    };

    /**
     * @brief 图像操作失败异常
     */
    class OperationFailedException : public ImageException
    {
    public:
        explicit OperationFailedException(const std::string &message)
            : ImageException("Operation failed: " + message) {}
    };

} // namespace mylib

#endif // OPTIMAL_IMAGE_H