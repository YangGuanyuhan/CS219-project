#include "optimal_image.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// 用于创建简单的测试图像
mylib::OptimalImage createTestImage(int width, int height, int channels)
{
    mylib::OptimalImage img(width, height, channels);

    // 填充图像数据，创建一个简单的渐变
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            for (int c = 0; c < channels; ++c)
            {
                // 根据通道创建不同的渐变
                if (c == 0)
                {
                    // 红色通道 - 水平渐变
                    img.at(y, x, c) = static_cast<unsigned char>((x * 255) / width);
                }
                else if (c == 1)
                {
                    // 绿色通道 - 垂直渐变
                    img.at(y, x, c) = static_cast<unsigned char>((y * 255) / height);
                }
                else if (c == 2)
                {
                    // 蓝色通道 - 对角线渐变
                    img.at(y, x, c) = static_cast<unsigned char>(((x + y) * 255) / (width + height));
                }
                else
                {
                    // 其他通道 - 固定值
                    img.at(y, x, c) = 128;
                }
            }
        }
    }

    return img;
}

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

int main()
{
    try
    {
        // 检测SIMD支持
        std::cout << mylib::OptimalImage::getSIMDInfo() << std::endl
                  << std::endl;

        // 打印当前工作目录，帮助诊断相对路径问题
        std::cout << "当前工作目录: " << fs::current_path().string() << std::endl;

        std::cout << "=== OptimalImage库演示 ===" << std::endl
                  << std::endl;

        // 创建RGB图像
        mylib::OptimalImage img1 = createTestImage(800, 600, 3);
        printImageInfo(img1, "原始图像 (程序创建)");

        // 测试保存图像
        std::cout << "测试保存图像..." << std::endl;
        try
        {
            img1.save("created_image.png");
            std::cout << "  - 图像已保存为 created_image.png" << std::endl;
            img1.save("created_image.jpg");
            std::cout << "  - 图像已保存为 created_image.jpg" << std::endl;

            // 测试加载保存的图像
            mylib::OptimalImage loaded_png("created_image.png");
            printImageInfo(loaded_png, "从PNG加载的图像");

            mylib::OptimalImage loaded_jpg("created_image.jpg");
            printImageInfo(loaded_jpg, "从JPG加载的图像");
        }
        catch (const mylib::ImageException &e)
        {
            std::cerr << "  保存或加载测试图像时发生错误: " << e.what() << std::endl;
        }
        std::cout << std::endl;

        // 测试从文件加载图像
        std::cout << "测试从资源图像加载..." << std::endl;
        try
        {
            std::string shark_path = "resources/shark.jpg";
            std::string wave_path = "resources/wave.jpg";

            // 检查文件是否存在
            std::cout << "尝试加载的文件: " << fs::absolute(shark_path).string() << std::endl;
            std::cout << "文件存在?: " << (fs::exists(shark_path) ? "是" : "否") << std::endl;

            // 加载图像
            if (fs::exists(shark_path))
            {
                mylib::OptimalImage shark_img(shark_path);
                printImageInfo(shark_img, "shark.jpg");

                // 测试调整图像亮度并保存
                {
                    std::cout << "测试调整shark图像亮度..." << std::endl;
                    mylib::OptimalImage brightened_shark = shark_img.clone(); // 创建副本以保持原始图像不变

                    brightened_shark.adjustBrightness(100); // 提高亮度

                    brightened_shark.save("shark_brightened.jpg");
                    std::cout << "亮度调整后的图像已保存为 shark_brightened.jpg" << std::endl;
                }
            }

            std::cout << "尝试加载的文件: " << fs::absolute(wave_path).string() << std::endl;
            std::cout << "文件存在?: " << (fs::exists(wave_path) ? "是" : "否") << std::endl;

            if (fs::exists(wave_path))
            {
                mylib::OptimalImage wave_img(wave_path);
                printImageInfo(wave_img, "wave.jpg");

                // 测试图像混合
                if (fs::exists(shark_path))
                {
                    mylib::OptimalImage shark_img(shark_path);

                    // 测试两个图像尺寸是否相同
                    if (shark_img.width() == wave_img.width() &&
                        shark_img.height() == wave_img.height() &&
                        shark_img.channels() == wave_img.channels())
                    {

                        mylib::OptimalImage blended = mylib::OptimalImage::blend(wave_img, shark_img, 0.8f);
                        blended.save("blended.jpg");
                        std::cout << "混合图像已保存为 blended.jpg" << std::endl;
                        mylib::OptimalImage blended2 = mylib::OptimalImage::blend(shark_img, wave_img, 0.8f);
                        blended2.save("blended2.jpg");
                        std::cout << "混合图像2已保存为 blended2.jpg" << std::endl;
                    }
                    else
                    {
                        std::cout << "shark.jpg 和 wave.jpg 尺寸不匹配，无法混合" << std::endl;
                    }
                }
            }
        }
        catch (const mylib::ImageException &e)
        {
            std::cerr << "  加载或处理文件图像时发生错误: " << e.what() << std::endl;
        }
        std::cout << std::endl;

        // 测试数据共享模型
        std::cout << "测试数据共享模型..." << std::endl;
        mylib::OptimalImage original = createTestImage(400, 400, 3);
        printImageInfo(original, "原始图像");

        // 创建浅拷贝
        mylib::OptimalImage shallowCopy = original;
        printImageInfo(shallowCopy, "浅拷贝");

        
        // 修改原始图像，不影响浅拷贝，因为会触发写时复制机制
        original.at(100, 100, 0) = 255;
        std::cout << "修改原始图像后：" << std::endl;
        std::cout << "  原始图像引用计数: " << original.refCount() << std::endl;
        std::cout << "  浅拷贝引用计数: " << shallowCopy.refCount() << std::endl;

        // 测试成功
        std::cout << std::endl
                  << "=== 演示完成 ===" << std::endl;
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