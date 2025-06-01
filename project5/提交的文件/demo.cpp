#include "optimal_image.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

// 用于计时的工具类
class Timer
{
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsedMilliseconds() const
    {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(now - start_).count();
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// 显示图像基本信息
void printImageInfo(const mylib::OptimalImage &img, const std::string &name)
{
    std::cout << "图像信息 '" << name << "':" << std::endl;
    std::cout << "  - 大小: " << img.width() << "x" << img.height() << std::endl;
    std::cout << "  - 通道数: " << img.channels() << std::endl;
    std::cout << "  - 内存大小: " << img.size() << " 字节" << std::endl;
    std::cout << "  - 步长: " << img.step() << " 字节/行" << std::endl;
    std::cout << "  - 是否为空: " << (img.empty() ? "是" : "否") << std::endl;
    std::cout << "  - 引用计数: " << img.refCount() << std::endl;
    std::cout << std::endl;
}

// 性能测试结果结构
struct TestResults
{
    double loadTime;
    double shallowCopyTime;
    double deepCopyTime;
    double brightnessAdjustTime;
    double gaussianBlurTime;
    int shallowCopyRefCount;
    int deepCopyRefCount;
};

// 进行性能测试
TestResults performanceTest(const std::string &imagePath)
{
    TestResults results;
    std::string filename = fs::path(imagePath).filename().string();

    std::cout << "===== 测试文件: " << filename << " =====" << std::endl;

    // 测试图像加载性能
    Timer loadTimer;
    mylib::OptimalImage img(imagePath);
    results.loadTime = loadTimer.elapsedMilliseconds();
    std::cout << "读取时间: " << results.loadTime << "ms" << std::endl;

    // 显示图像基本信息
    printImageInfo(img, filename);

    // 测试浅拷贝性能
    {
        Timer timer;
        mylib::OptimalImage img2 = img; // 浅拷贝，共享数据
        results.shallowCopyTime = timer.elapsedMilliseconds();
        results.shallowCopyRefCount = img2.refCount();
        std::cout << "浅拷贝时间: " << results.shallowCopyTime << "ms" << std::endl;
        std::cout << "引用计数: " << results.shallowCopyRefCount << std::endl;
    }

    // 测试深拷贝性能
    mylib::OptimalImage clone;
    {
        Timer timer;
        clone = img.clone(); // 深拷贝，不共享数据
        results.deepCopyTime = timer.elapsedMilliseconds();
        results.deepCopyRefCount = clone.refCount();
        std::cout << "深拷贝时间: " << results.deepCopyTime << "ms" << std::endl;
        std::cout << "引用计数: " << results.deepCopyRefCount << std::endl;
    }

    // 测试亮度调整性能
    {
        mylib::OptimalImage testImg = img.clone();
        Timer timer;
        testImg.adjustBrightness(20);
        results.brightnessAdjustTime = timer.elapsedMilliseconds();
        std::cout << "亮度调整时间: " << results.brightnessAdjustTime << "ms" << std::endl;
    }

    // 测试高斯模糊性能
    {
        Timer timer;
        mylib::OptimalImage blurred = img.gaussianBlur(5, 1.5);
        results.gaussianBlurTime = timer.elapsedMilliseconds();
        std::cout << "高斯模糊时间 (5x5): " << results.gaussianBlurTime << "ms" << std::endl;
    }

    std::cout << std::endl;
    return results;
}

// 输出性能测试结果表格
void printResultsTable(const std::vector<std::pair<std::string, TestResults>> &allResults)
{
    std::cout << "\n==== 性能测试结果汇总 ====\n"
              << std::endl;

    // 打印表头
    std::cout << std::left << std::setw(20) << "文件名"
              << std::setw(15) << "读取时间(ms)"
              << std::setw(15) << "浅拷贝(ms)"
              << std::setw(15) << "深拷贝(ms)"
              << std::setw(15) << "亮度调整(ms)"
              << std::setw(15) << "高斯模糊(ms)"
              << std::endl;

    std::cout << std::string(95, '-') << std::endl;

    // 打印每个文件的结果
    for (const auto &[filename, results] : allResults)
    {
        std::cout << std::left << std::setw(20) << fs::path(filename).filename().string()
                  << std::fixed << std::setprecision(4)
                  << std::setw(15) << results.loadTime
                  << std::setw(15) << results.shallowCopyTime
                  << std::setw(15) << results.deepCopyTime
                  << std::setw(15) << results.brightnessAdjustTime
                  << std::setw(15) << results.gaussianBlurTime
                  << std::endl;
    }
}

int main()
{
    try
    {
        // 检测SIMD支持
        std::cout << mylib::OptimalImage::getSIMDInfo() << std::endl
                  << std::endl;

        // 打印当前工作目录，帮助诊断相对路径问题
        std::cout << "当前工作目录: " << fs::current_path().string() << std::endl;

        std::cout << "=== OptimalImage库性能测试 ===" << std::endl
                  << std::endl;

        // 测试目录
        std::string testDir = "performancetest";
        if (!fs::exists(testDir))
        {
            std::cerr << "错误: 测试目录不存在: " << testDir << std::endl;
            return 1;
        }

        // 收集所有测试图像文件
        std::vector<std::string> imageFiles;
        for (const auto &entry : fs::directory_iterator(testDir))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                // 只处理图像文件，跳过Zone.Identifier文件
                if (filename.find(".bmp") != std::string::npos &&
                    filename.find(":Zone.Identifier") == std::string::npos)
                {
                    imageFiles.push_back(entry.path().string());
                }
            }
        }

        // 按图像大小排序（从文件名中提取尺寸信息）
        std::sort(imageFiles.begin(), imageFiles.end(), [](const std::string &a, const std::string &b)
        {
            // 从文件名中提取数字部分
            auto extractSize = [](const std::string& path) {
                std::string filename = fs::path(path).filename().string();
                size_t pos1 = filename.find('_');
                size_t pos2 = filename.find('.');
                if (pos1 != std::string::npos && pos2 != std::string::npos) {
                    std::string sizeStr = filename.substr(pos1 + 1, pos2 - pos1 - 1);
                    try {
                        return std::stoi(sizeStr);
                    } catch (...) {
                        return 0;
                    }
                }
                return 0;
            };
            return extractSize(a) < extractSize(b); });

        // 对每个文件进行性能测试
        std::vector<std::pair<std::string, TestResults>> allResults;

        for (const auto &file : imageFiles)
        {
            TestResults results = performanceTest(file);
            allResults.emplace_back(file, results);
        }

        // 打印汇总表格
        printResultsTable(allResults);

        std::cout << std::endl
                  << "=== 性能测试完成 ===" << std::endl;
        return 0;
    }
    catch (const mylib::ImageException &e)
    {
        std::cerr << "图像处理异常: " << e.what() << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "标准异常: " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cerr << "未知异常!" << std::endl;
    }

    return 1;
}
