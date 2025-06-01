#!/usr/bin/env python3

from OptimalImage import OptimalImage, ImageProcessingError
import logging

# 配置日志级别
logging.basicConfig(level=logging.INFO)


def basic_usage_example():
    """基础使用示例"""
    # print("=== 基础使用示例 ===")
    
    try:
        # 创建图像处理对象
        img_processor = OptimalImage()
        
        # 加载图像
        img_processor.load_image("./input/shark.jpg")
        
        # 获取图像信息
        info = img_processor.get_image_info()
        # print(f"图像信息: {info}")
        
        # 调整亮度并保存
        img_processor.adjust_brightness(1.3)
        img_processor.save_image("./output/bright_shark.jpg")

        # print("基础使用示例完成！")

    except ImageProcessingError as e:
        print(f"处理错误: {e}")


def chain_operations_example():
    """链式操作示例"""
    print("\n=== 链式操作示例 ===")
    
    try:
        # 创建并执行链式操作
        result = (OptimalImage("./input/wave.jpg")
                 .adjust_brightness(0.9)
                 .gaussian_blur(7, 1.5)
                 .save_image("./output/processed_wave.jpg"))
        
        print("链式操作完成！")
        
    except ImageProcessingError as e:
        print(f"处理错误: {e}")


def image_blending_example():
    """图像混合示例"""
    print("\n=== 图像混合示例 ===")
    
    try:
        # 加载第一张图像
        processor = OptimalImage("./input/shark.jpg")
        
        # 与第二张图像混合
        processor.blend_with_image("./input/wave.jpg", alpha=0.3)
        
        # 保存混合结果
        processor.save_image("./output/custom_blend.jpg", quality=90)
        
        print("图像混合完成！")
        
    except ImageProcessingError as e:
        print(f"处理错误: {e}")


def error_handling_example():
    """错误处理示例"""
    print("\n=== 错误处理示例 ===")
    
    # 演示各种错误情况的处理
    test_cases = [
        ("不存在的文件", "./input/nonexistent.jpg"),
        ("无效路径", ""),
        ("危险路径", "../../../etc/passwd"),
    ]
    
    for description, path in test_cases:
        try:
            processor = OptimalImage(path)
            print(f"{description}: 意外成功")
        except ImageProcessingError as e:
            print(f"{description}: 正确捕获错误 - {e}")
        except Exception as e:
            print(f"{description}: 未预期的错误 - {e}")


def advanced_processing_example():
    """高级处理示例"""
    print("\n=== 高级处理示例 ===")
    
    try:
        # 创建处理管道
        processor = OptimalImage("./input/shark.jpg")
        
        # 保存原始状态
        original_info = processor.get_image_info()
        print(f"原始图像: {original_info}")
        
        # 多步骤处理
        processor.adjust_brightness(1.1)
        processor.gaussian_blur(5, 1.0)
        processor.save_image("./output/step1_result.jpg")
        
        # 重置到原始状态
        processor.reset_to_original()
        
        # 不同的处理路径
        processor.blend_with_image("./input/wave.jpg", alpha=0.7)
        processor.adjust_brightness(0.8)
        processor.save_image("./output/step2_result.jpg")
        
        print("高级处理完成！")
        
    except ImageProcessingError as e:
        print(f"处理错误: {e}")


def batch_processing_example():
    """批量处理示例"""
    print("\n=== 批量处理示例 ===")
    
    # 定义处理参数
    brightness_levels = [0.7, 1.0, 1.3]
    blur_params = [(3, 0.5), (7, 1.5), (15, 3.0)]
    
    try:
        base_processor = OptimalImage("./input/shark.jpg")
        
        # 批量亮度调整
        for i, brightness in enumerate(brightness_levels):
            base_processor.reset_to_original()
            base_processor.adjust_brightness(brightness)
            base_processor.save_image(f"./output/brightness_{i+1}.jpg")
        
        # 批量模糊处理
        for i, (kernel, sigma) in enumerate(blur_params):
            base_processor.reset_to_original()
            base_processor.gaussian_blur(kernel, sigma)
            base_processor.save_image(f"./output/blur_{i+1}.jpg")
        
        print("批量处理完成！")
        
    except ImageProcessingError as e:
        print(f"处理错误: {e}")


def main():
    # """主函数"""
    # print("图像处理模块使用示例")
    # print("=" * 50)
    
    # 运行所有示例
    basic_usage_example()
    # chain_operations_example()
    # image_blending_example()
    # error_handling_example()
    # advanced_processing_example()
    # batch_processing_example()
    
    # print("\n所有示例运行完成！")
    # print("请检查 ./output/ 目录查看生成的图像文件。")


if __name__ == "__main__":
    main()
