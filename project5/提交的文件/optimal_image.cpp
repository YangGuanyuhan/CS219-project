#include "optimal_image.h"
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <cmath>

// OpenMP支持
#ifdef _OPENMP
#include <omp.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace mylib
{
    // ===== ImageDataManager实现 =====
    ImageDataManager::ImageDataManager(size_t dataSize)
        : size_(dataSize), refCount_(1)
    {
        if (dataSize > 0) {
            data_ = std::make_unique<unsigned char[]>(dataSize);
            std::memset(data_.get(), 0, dataSize);
        }
    }

    ImageDataManager::~ImageDataManager() = default;

    unsigned char* ImageDataManager::data() {
        return data_.get();
    }

    const unsigned char* ImageDataManager::data() const {
        return data_.get();
    }

    size_t ImageDataManager::size() const {
        return size_;
    }

    void ImageDataManager::addRef() {
        ++refCount_;
    }

    int ImageDataManager::release() {
        return --refCount_;
    }

    int ImageDataManager::refCount() const {
        return refCount_;
    }

    // ===== OptimalImage实现 =====
    OptimalImage::OptimalImage()
        : width_(0), height_(0), channels_(0), step_(0)
    {
        // 不需要创建dataManager_，留为nullptr
    }

    OptimalImage::OptimalImage(int width, int height, int channels)
        : width_(0), height_(0), channels_(0), step_(0)
    {
        create(width, height, channels);
    }

    OptimalImage::OptimalImage(const OptimalImage &other)
        : dataManager_(other.dataManager_), width_(other.width_), 
          height_(other.height_), channels_(other.channels_), step_(other.step_)
    {
        // 增加引用计数
        if (dataManager_) {
            dataManager_->addRef();
        }
    }

    OptimalImage::OptimalImage(const std::string &filename)
        : width_(0), height_(0), channels_(0), step_(0)
    {
        load(filename);
    }

    OptimalImage::OptimalImage(OptimalImage &&other) noexcept
        : dataManager_(std::move(other.dataManager_)), 
          width_(other.width_), height_(other.height_), 
          channels_(other.channels_), step_(other.step_)
    {
        other.width_ = 0;
        other.height_ = 0;
        other.channels_ = 0;
        other.step_ = 0;
    }

    OptimalImage &OptimalImage::operator=(const OptimalImage &other)
    {
        if (this != &other)
        {
            // 减少旧数据的引用计数
            if (dataManager_) {
                if (dataManager_->release() <= 0) {
                    dataManager_.reset();
                }
            }

            // 指向新的数据并增加引用计数
            dataManager_ = other.dataManager_;
            if (dataManager_) {
                dataManager_->addRef();
            }

            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            step_ = other.step_;
        }
        return *this;
    }

    OptimalImage &OptimalImage::operator=(OptimalImage &&other) noexcept
    {
        if (this != &other)
        {
            // 减少旧数据的引用计数
            if (dataManager_) {
                if (dataManager_->release() <= 0) {
                    dataManager_.reset();
                }
            }

            // 移动新数据的所有权
            dataManager_ = std::move(other.dataManager_);
            width_ = other.width_;
            height_ = other.height_;
            channels_ = other.channels_;
            step_ = other.step_;

            other.width_ = 0;
            other.height_ = 0;
            other.channels_ = 0;
            other.step_ = 0;
        }
        return *this;
    }

    OptimalImage::~OptimalImage()
    {
        release();
    }

    int OptimalImage::width() const
    {
        return width_;
    }

    int OptimalImage::height() const
    {
        return height_;
    }

    int OptimalImage::channels() const
    {
        return channels_;
    }

    size_t OptimalImage::size() const
    {
        return static_cast<size_t>(height_) * step_;
    }

    size_t OptimalImage::step() const
    {
        return step_;
    }

    bool OptimalImage::empty() const
    {
        return !dataManager_ || width_ <= 0 || height_ <= 0 || channels_ <= 0;
    }

    unsigned char *OptimalImage::data()
    {
        return dataManager_ ? dataManager_->data() : nullptr;
    }

    const unsigned char *OptimalImage::data() const
    {
        return dataManager_ ? dataManager_->data() : nullptr;
    }

    int OptimalImage::refCount() const
    {
        return dataManager_ ? dataManager_->refCount() : 0;
    }

    void OptimalImage::checkRange(int row, int col, int channel) const
    {
        if (empty())
        {
            throw OutOfRangeException("Image is empty");
        }

        std::stringstream ss;
        if (row < 0 || row >= height_)
        {
            ss << "Row index " << row << " out of range [0, " << height_ - 1 << "]";
            throw OutOfRangeException(ss.str());
        }

        if (col < 0 || col >= width_)
        {
            ss << "Column index " << col << " out of range [0, " << width_ - 1 << "]";
            throw OutOfRangeException(ss.str());
        }

        if (channel < 0 || channel >= channels_)
        {
            ss << "Channel index " << channel << " out of range [0, " << channels_ - 1 << "]";
            throw OutOfRangeException(ss.str());
        }
    }

    unsigned char &OptimalImage::at(int row, int col, int channel)
    {
        checkRange(row, col, channel);
        // 如果有多个引用，复制图像数据
        copyOnWrite();
        return dataManager_->data()[static_cast<size_t>(row) * step_ + static_cast<size_t>(col) * channels_ + channel];
    }

    const unsigned char &OptimalImage::at(int row, int col, int channel) const
    {
        checkRange(row, col, channel);
        return dataManager_->data()[static_cast<size_t>(row) * step_ + static_cast<size_t>(col) * channels_ + channel];
    }

    void OptimalImage::copyOnWrite()
    {
        if (dataManager_ && dataManager_->refCount() > 1)
        {
            // 创建新的数据副本
            auto newDataManager = std::make_shared<ImageDataManager>(size());
            std::memcpy(newDataManager->data(), dataManager_->data(), size());
            
            // 减少原数据引用计数
            dataManager_->release();
            
            // 使用新数据
            dataManager_ = std::move(newDataManager);
        }
    }

    void OptimalImage::create(int width, int height, int channels)
    {
        if (width <= 0 || height <= 0 || channels <= 0)
        {
            std::stringstream ss;
            ss << "Invalid dimensions: width=" << width
               << ", height=" << height
               << ", channels=" << channels;
            throw InvalidArgumentException(ss.str());
        }

        // 计算一行的字节数（对齐到4字节边界以提高内存访问效率）
        size_t newStep = ((static_cast<size_t>(width) * channels + 3) / 4) * 4;
        size_t totalSize = static_cast<size_t>(height) * newStep;

        // 如果尺寸改变，需要重新分配内存
        if (width_ != width || height_ != height || channels_ != channels || step_ != newStep || !dataManager_)
        {
            // 释放旧的数据
            release();

            try
            {
                // 分配新的数据
                dataManager_ = std::make_shared<ImageDataManager>(totalSize);
                width_ = width;
                height_ = height;
                channels_ = channels;
                step_ = newStep;
            }
            catch (const std::bad_alloc &)
            {
                throw OperationFailedException("Memory allocation failed during create");
            }
            catch (...)
            {
                throw OperationFailedException("Unknown error occurred during image creation");
            }
        }
        else
        {
            // 如果尺寸没变但有多个引用，需要创建数据副本
            copyOnWrite();
            // 清除数据
            std::memset(dataManager_->data(), 0, totalSize);
        }
    }

    void OptimalImage::load(const std::string &filename)
    {
        if (filename.empty())
        {
            throw InvalidArgumentException("Filename cannot be empty for load operation.");
        }

        // 检查文件是否存在
        std::ifstream f(filename.c_str());
        if (!f.good())
        {
            throw OperationFailedException("File not found or not accessible: " + filename);
        }
        f.close();

        int img_width, img_height, img_channels;
        unsigned char *loaded_data = stbi_load(filename.c_str(), &img_width, &img_height, &img_channels, 0);

        if (!loaded_data)
        {
            std::stringstream ss;
            ss << "Failed to load image '" << filename << "'. Reason: " << stbi_failure_reason();
            throw OperationFailedException(ss.str());
        }

        // 释放旧数据并创建新的图像结构
        release();
        try
        {
            // 计算步长（对齐到4字节边界）
            size_t newStep = ((static_cast<size_t>(img_width) * img_channels + 3) / 4) * 4;
            size_t totalSize = static_cast<size_t>(img_height) * newStep;
            
            dataManager_ = std::make_shared<ImageDataManager>(totalSize);
            width_ = img_width;
            height_ = img_height;
            channels_ = img_channels;
            step_ = newStep;
            
            // 如果步长等于宽度*通道数，可以一次性复制
            if (newStep == static_cast<size_t>(img_width) * img_channels) {
                std::memcpy(dataManager_->data(), loaded_data, totalSize);
            } else {
                // 否则需要逐行复制
                size_t rowBytes = static_cast<size_t>(img_width) * img_channels;
                for (int y = 0; y < img_height; ++y) {
                    std::memcpy(
                        dataManager_->data() + y * newStep,
                        loaded_data + y * rowBytes,
                        rowBytes
                    );
                }
            }
        }
        catch (...)
        {
            stbi_image_free(loaded_data);
            throw;
        }
        
        stbi_image_free(loaded_data);
    }

    void OptimalImage::save(const std::string &filename) const
    {
        if (filename.empty())
        {
            throw InvalidArgumentException("Filename cannot be empty for save operation.");
        }

        if (empty())
        {
            throw OperationFailedException("Cannot save an empty image.");
        }

        // 获取文件扩展名以确定格式
        size_t dot_pos = filename.find_last_of('.');
        if (dot_pos == std::string::npos)
        {
            throw InvalidArgumentException("Filename must have an extension to determine format: " + filename);
        }

        std::string ext = filename.substr(dot_pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        // 创建临时缓冲区，如果步长不等于宽度*通道数
        std::unique_ptr<unsigned char[]> temp_buffer;
        const unsigned char* saveData = dataManager_->data();
        
        if (step_ != static_cast<size_t>(width_) * channels_) {
            // 创建一个连续的缓冲区
            size_t actualSize = static_cast<size_t>(width_) * height_ * channels_;
            temp_buffer = std::make_unique<unsigned char[]>(actualSize);
            
            size_t rowBytes = static_cast<size_t>(width_) * channels_;
            for (int y = 0; y < height_; ++y) {
                std::memcpy(
                    temp_buffer.get() + y * rowBytes,
                    dataManager_->data() + y * step_,
                    rowBytes
                );
            }
            saveData = temp_buffer.get();
        }

        int result = 0;
        if (ext == "png")
        {
            result = stbi_write_png(filename.c_str(), width_, height_, channels_, saveData, width_ * channels_);
        }
        else if (ext == "jpg" || ext == "jpeg")
        {
            result = stbi_write_jpg(filename.c_str(), width_, height_, channels_, saveData, 95); // 95% quality
        }
        else if (ext == "bmp")
        {
            result = stbi_write_bmp(filename.c_str(), width_, height_, channels_, saveData);
        }
        else
        {
            throw InvalidArgumentException("Unsupported image format: " + ext);
        }

        if (result == 0)
        {
            throw OperationFailedException("Failed to save image to " + filename);
        }
    }

    void OptimalImage::release()
    {
        if (dataManager_)
        {
            if (dataManager_->release() <= 0)
            {
                dataManager_.reset();
            }
        }
        
        width_ = 0;
        height_ = 0;
        channels_ = 0;
        step_ = 0;
    }

    OptimalImage OptimalImage::clone() const
    {
        if (empty())
        {
            return OptimalImage();
        }

        OptimalImage copy(width_, height_, channels_);
        std::memcpy(copy.dataManager_->data(), dataManager_->data(), size());
        return copy;
    }

    std::string OptimalImage::getSIMDInfo()
    {
        std::stringstream ss;
        ss << "SIMD 支持: ";

    #if defined(OPT_WINDOWS)
        int cpuInfo[4];
        __cpuid(cpuInfo, 1);
        bool hasSSE = (cpuInfo[3] & (1 << 25)) != 0;
        bool hasSSE2 = (cpuInfo[3] & (1 << 26)) != 0;
        bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;
        
        __cpuid(cpuInfo, 7);
        bool hasAVX2 = (cpuInfo[1] & (1 << 5)) != 0;
    #elif defined(OPT_UNIX)
        #ifdef __SSE__
        bool hasSSE = true;
        #else
        bool hasSSE = false;
        #endif
        
        #ifdef __SSE2__
        bool hasSSE2 = true;
        #else
        bool hasSSE2 = false;
        #endif
        
        #ifdef __AVX__
        bool hasAVX = true;
        #else
        bool hasAVX = false;
        #endif
        
        #ifdef __AVX2__
        bool hasAVX2 = true;
        #else
        bool hasAVX2 = false;
        #endif
    #else
        bool hasSSE = false;
        bool hasSSE2 = false;
        bool hasAVX = false;
        bool hasAVX2 = false;
    #endif

        if (hasAVX2) ss << "AVX2 ";
        if (hasAVX) ss << "AVX ";
        if (hasSSE2) ss << "SSE2 ";
        if (hasSSE) ss << "SSE ";
        
        if (!hasSSE && !hasSSE2 && !hasAVX && !hasAVX2)
            ss << "None";
            
    #ifdef _OPENMP
        ss << "| OpenMP: " << _OPENMP;
    #else
        ss << "| OpenMP: Not supported";
    #endif

        return ss.str();
    }
} // namespace mylib