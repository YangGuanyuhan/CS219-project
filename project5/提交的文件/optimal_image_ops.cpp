#include "optimal_image.h"
#include <algorithm>
#include <sstream>
#include <cmath>
#include <vector>

// OpenMP支持
#ifdef _OPENMP
#include <omp.h>
#endif

// SIMD支持通用处理
#if defined(OPT_WINDOWS) || defined(OPT_UNIX)
#define USE_SIMD
#endif

// 数据量较大时才启用加速策略的阈值
#define OPTIMIZATION_THRESHOLD 10000

namespace mylib
{

    void OptimalImage::adjustBrightness(int delta)
    {
        if (empty())
        {
            throw OperationFailedException("Cannot adjust brightness of an empty image");
        }

        if (delta < -255 || delta > 255)
        {
            std::stringstream ss;
            ss << "Brightness delta must be in range [-255, 255], but got " << delta;
            throw InvalidArgumentException(ss.str());
        }

        // 确保数据可修改（如果多处引用，会创建副本）
        copyOnWrite();

        unsigned char *imageData = dataManager_->data();
        size_t imageSize = size();
        int pixelCount = width_ * height_;

#ifdef USE_SIMD
        // 仅当数据量大于阈值时使用SIMD指令加速处理
        if (pixelCount > OPTIMIZATION_THRESHOLD)
        {
#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))
            // AVX2指令集实现（处理32个8位整数/次）
            if (channels_ == 1 || channels_ == 3 || channels_ == 4)
            {
                // 创建8个delta值的向量
                __m256i deltaVec = _mm256_set1_epi8(static_cast<char>(delta));

                // 按照8位整数批量处理
                size_t vectorizedEnd = (imageSize / 32) * 32; // 能被32整除的部分

#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (size_t i = 0; i < vectorizedEnd; i += 32)
                {
                    // 加载32字节
                    __m256i pixels = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(imageData + i));

                    // 增加亮度
                    __m256i result = _mm256_adds_epu8(pixels, deltaVec); // 加法并且有饱和保护

                    // 存回内存
                    _mm256_storeu_si256(reinterpret_cast<__m256i *>(imageData + i), result);
                }

// 处理剩余的像素
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (size_t i = vectorizedEnd; i < imageSize; ++i)
                {
                    int newValue = static_cast<int>(imageData[i]) + delta;
                    imageData[i] = static_cast<unsigned char>(std::clamp(newValue, 0, 255));
                }
                return;
            }
#elif defined(__SSE2__) || (defined(_MSC_VER) && !defined(_M_ARM))
            // SSE2指令集实现（处理16个8位整数/次）
            if (channels_ == 1 || channels_ == 3 || channels_ == 4)
            {
                // 创建16个delta值的向量
                __m128i deltaVec = _mm_set1_epi8(static_cast<char>(delta));

                // 按照8位整数批量处理
                size_t vectorizedEnd = (imageSize / 16) * 16; // 能被16整除的部分

#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (size_t i = 0; i < vectorizedEnd; i += 16)
                {
                    // 加载16字节
                    __m128i pixels = _mm_loadu_si128(reinterpret_cast<const __m128i *>(imageData + i));

                    // 增加亮度
                    __m128i result = _mm_adds_epu8(pixels, deltaVec); // 加法并且有饱和保护

                    // 存回内存
                    _mm_storeu_si128(reinterpret_cast<__m128i *>(imageData + i), result);
                }

// 处理剩余的像素
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (size_t i = vectorizedEnd; i < imageSize; ++i)
                {
                    int newValue = static_cast<int>(imageData[i]) + delta;
                    imageData[i] = static_cast<unsigned char>(std::clamp(newValue, 0, 255));
                }
                return;
            }
#endif
        }
#endif

// 如果没有SIMD或者不支持当前通道配置，使用OpenMP加速的标准实现
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
        for (int y = 0; y < height_; ++y)
        {
            unsigned char *rowPtr = imageData + y * step_;
            for (int x = 0; x < width_; ++x)
            {
                for (int c = 0; c < channels_; ++c)
                {
                    int idx = x * channels_ + c;
                    int newValue = static_cast<int>(rowPtr[idx]) + delta;
                    rowPtr[idx] = static_cast<unsigned char>(std::clamp(newValue, 0, 255));
                }
            }
        }
    }

    OptimalImage OptimalImage::blend(const OptimalImage &img1, const OptimalImage &img2, float alpha)
    {
        if (img1.empty() || img2.empty())
        {
            throw InvalidArgumentException("Cannot blend empty images");
        }

        if (alpha < 0.0f || alpha > 1.0f)
        {
            std::stringstream ss;
            ss << "Alpha must be in range [0, 1], but got " << alpha;
            throw InvalidArgumentException(ss.str());
        }

        if (img1.width() != img2.width() || img1.height() != img2.height())
        {
            std::stringstream ss;
            ss << "Image dimensions must match for blending. "
               << "First image: " << img1.width() << "x" << img1.height()
               << ", Second image: " << img2.width() << "x" << img2.height();
            throw InvalidArgumentException(ss.str());
        }

        if (img1.channels() != img2.channels())
        {
            std::stringstream ss;
            ss << "Image channels must match for blending. "
               << "First image: " << img1.channels()
               << ", Second image: " << img2.channels();
            throw InvalidArgumentException(ss.str());
        }

        // 创建结果图像
        OptimalImage result(img1.width(), img1.height(), img1.channels());

        // 计算混合权重
        float beta = 1.0f - alpha;

        // 获取指针以提高访问效率
        const unsigned char *ptr1 = img1.data();
        const unsigned char *ptr2 = img2.data();
        unsigned char *ptrResult = result.data();

        int width = img1.width();
        int height = img1.height();
        int channels = img1.channels();
        size_t step1 = img1.step();
        size_t step2 = img2.step();
        size_t stepResult = result.step();
        int pixelCount = width * height;

#ifdef USE_SIMD
        // 仅当数据量大于阈值时使用SIMD指令加速处理
        if (pixelCount > OPTIMIZATION_THRESHOLD)
        {
#if defined(__AVX2__) || (defined(_MSC_VER) && defined(__AVX2__))
            // 对于连续存储并且通道数少于4的情况，使用特定优化
            if (step1 == static_cast<size_t>(width) * channels &&
                step2 == static_cast<size_t>(width) * channels &&
                stepResult == static_cast<size_t>(width) * channels)
            {

                __m256 alphaVec = _mm256_set1_ps(alpha);
                __m256 betaVec = _mm256_set1_ps(beta);

#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (int y = 0; y < height; ++y)
                {
                    const unsigned char *row1 = ptr1 + y * step1;
                    const unsigned char *row2 = ptr2 + y * step2;
                    unsigned char *rowResult = ptrResult + y * stepResult;

                    // 处理每行的像素
                    int x = 0;
                    // 8个一组处理float（处理32个一组的整数太复杂，这里简化）
                    for (; x <= width * channels - 8; x += 8)
                    {
                        // 加载8个像素值并转换为float
                        __m256i vals1i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(row1 + x)));
                        __m256i vals2i = _mm256_cvtepu8_epi32(_mm_loadl_epi64(reinterpret_cast<const __m128i *>(row2 + x)));

                        __m256 vals1f = _mm256_cvtepi32_ps(vals1i);
                        __m256 vals2f = _mm256_cvtepi32_ps(vals2i);

                        // 混合计算
                        __m256 resultf = _mm256_add_ps(_mm256_mul_ps(vals1f, alphaVec), _mm256_mul_ps(vals2f, betaVec));

                        // 转回整数并裁剪到0-255
                        __m256i resulti = _mm256_cvtps_epi32(resultf);

                        // 压缩回8位
                        resulti = _mm256_packs_epi32(resulti, _mm256_setzero_si256()); // 32位->16位
                        resulti = _mm256_permute4x64_epi64(resulti, 0x08);             // AVX2特有，重排
                        __m128i result128 = _mm256_castsi256_si128(resulti);
                        result128 = _mm_packus_epi16(result128, _mm_setzero_si128()); // 16位->8位并裁剪

                        // 存储结果（只存8个字节）
                        _mm_storel_epi64(reinterpret_cast<__m128i *>(rowResult + x), result128);
                    }

                    // 处理剩余像素
                    for (; x < width * channels; ++x)
                    {
                        float blended_value = alpha * row1[x] + beta * row2[x];
                        rowResult[x] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, blended_value)));
                    }
                }

                return result;
            }
#elif defined(__SSE2__) || (defined(_MSC_VER) && !defined(_M_ARM))
            // SSE2实现类似，但处理4个而不是8个
            if (step1 == static_cast<size_t>(width) * channels &&
                step2 == static_cast<size_t>(width) * channels &&
                stepResult == static_cast<size_t>(width) * channels)
            {

                __m128 alphaVec = _mm_set1_ps(alpha);
                __m128 betaVec = _mm_set1_ps(beta);

#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
                for (int y = 0; y < height; ++y)
                {
                    const unsigned char *row1 = ptr1 + y * step1;
                    const unsigned char *row2 = ptr2 + y * step2;
                    unsigned char *rowResult = ptrResult + y * stepResult;

                    // 处理每行的像素
                    int x = 0;
                    // 4个一组处理float
                    for (; x <= width * channels - 4; x += 4)
                    {
                        // 加载4个像素值并转换为float
                        __m128i vals1i = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*reinterpret_cast<const int *>(row1 + x)));
                        __m128i vals2i = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*reinterpret_cast<const int *>(row2 + x)));

                        __m128 vals1f = _mm_cvtepi32_ps(vals1i);
                        __m128 vals2f = _mm_cvtepi32_ps(vals2i);

                        // 混合计算
                        __m128 resultf = _mm_add_ps(_mm_mul_ps(vals1f, alphaVec), _mm_mul_ps(vals2f, betaVec));

                        // 转回整数并裁剪到0-255
                        __m128i resulti = _mm_cvtps_epi32(resultf);

                        // 压缩回8位
                        resulti = _mm_packs_epi32(resulti, _mm_setzero_si128());  // 32位->16位
                        resulti = _mm_packus_epi16(resulti, _mm_setzero_si128()); // 16位->8位并裁剪

                        // 存储结果（只存4个字节）
                        *reinterpret_cast<int *>(rowResult + x) = _mm_cvtsi128_si32(resulti);
                    }

                    // 处理剩余像素
                    for (; x < width * channels; ++x)
                    {
                        float blended_value = alpha * row1[x] + beta * row2[x];
                        rowResult[x] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, blended_value)));
                    }
                }

                return result;
            }
#endif
        }
#endif

// 如果没有SIMD或者不支持当前数据排列，使用OpenMP优化的标准实现
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
        for (int y = 0; y < height; ++y)
        {
            const unsigned char *row1 = ptr1 + y * step1;
            const unsigned char *row2 = ptr2 + y * step2;
            unsigned char *rowResult = ptrResult + y * stepResult;

            for (int x = 0; x < width; ++x)
            {
                for (int c = 0; c < channels; ++c)
                {
                    int idx = x * channels + c;
                    float blended_value = alpha * row1[idx] + beta * row2[idx];
                    rowResult[idx] = static_cast<unsigned char>(std::min(255.0f, std::max(0.0f, blended_value)));
                }
            }
        }

        return result;
    }

    OptimalImage OptimalImage::gaussianBlur(int kernelSize, double sigma) const
    {
        if (empty())
        {
            throw OperationFailedException("Cannot apply Gaussian blur to an empty image");
        }

        if (kernelSize <= 0 || kernelSize % 2 == 0)
        {
            std::stringstream ss;
            ss << "Kernel size must be a positive odd number, but got " << kernelSize;
            throw InvalidArgumentException(ss.str());
        }

        if (sigma <= 0.0)
        {
            std::stringstream ss;
            ss << "Sigma must be positive, but got " << sigma;
            throw InvalidArgumentException(ss.str());
        }

        // 创建结果图像
        OptimalImage result(width_, height_, channels_);

        // 创建高斯核
        std::vector<float> kernel(kernelSize);
        float kernelSum = 0.0f;
        int radius = kernelSize / 2;

        for (int i = 0; i < kernelSize; ++i)
        {
            int x = i - radius;
            kernel[i] = static_cast<float>(exp(-(x * x) / (2 * sigma * sigma)));
            kernelSum += kernel[i];
        }

        // 归一化核
        for (int i = 0; i < kernelSize; ++i)
        {
            kernel[i] /= kernelSum;
        }

        // 创建临时图像用于中间结果（水平模糊）
        OptimalImage temp(width_, height_, channels_);

        const unsigned char *srcData = data();
        unsigned char *tempData = temp.data();
        unsigned char *dstData = result.data();

        size_t srcStep = step();
        size_t tempStep = temp.step();
        size_t dstStep = result.step();

        int pixelCount = width_ * height_;

// 水平方向模糊 (源图像 -> 临时图像)
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                for (int c = 0; c < channels_; ++c)
                {
                    float sum = 0.0f;

                    for (int i = -radius; i <= radius; ++i)
                    {
                        int sampleX = std::clamp(x + i, 0, width_ - 1);
                        sum += srcData[y * srcStep + sampleX * channels_ + c] * kernel[i + radius];
                    }

                    tempData[y * tempStep + x * channels_ + c] = static_cast<unsigned char>(sum + 0.5f);
                }
            }
        }

// 垂直方向模糊 (临时图像 -> 结果图像)
#pragma omp parallel for if (pixelCount > OPTIMIZATION_THRESHOLD)
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                for (int c = 0; c < channels_; ++c)
                {
                    float sum = 0.0f;

                    for (int i = -radius; i <= radius; ++i)
                    {
                        int sampleY = std::clamp(y + i, 0, height_ - 1);
                        sum += tempData[sampleY * tempStep + x * channels_ + c] * kernel[i + radius];
                    }

                    dstData[y * dstStep + x * channels_ + c] = static_cast<unsigned char>(sum + 0.5f);
                }
            }
        }

        return result;
    }

} // namespace mylib
