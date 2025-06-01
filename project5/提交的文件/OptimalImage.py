
import cv2
import numpy as np
import os
import sys
from pathlib import Path
from typing import Optional, Tuple, Union
import logging

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)


class ImageProcessingError(Exception):
    """图像处理异常类"""
    pass


class OptimalImage:
    """
    图像处理类
    """
    
    def __init__(self, image_path: Optional[str] = None):
        """
        初始化图像处理对象
        
        Args:
            image_path (Optional[str]): 图像文件路径
            
        Raises:
            ImageProcessingError: 当图像路径无效或图像加载失败时
        """
        self._image = None
        self._original_image = None
        
        if image_path is not None:
            self.load_image(image_path)
    
    def _validate_image(self, image: np.ndarray) -> bool:
        """
        验证图像数据的有效性
        
        Args:
            image (np.ndarray): 图像数据
            
        Returns:
            bool: 图像是否有效
        """
        if image is None:
            return False
        if not isinstance(image, np.ndarray):
            return False
        if image.size == 0:
            return False
        if len(image.shape) < 2 or len(image.shape) > 3:
            return False
        return True
    
    def _validate_path(self, path: str) -> bool:
        """
        验证文件路径的有效性
        
        Args:
            path (str): 文件路径
            
        Returns:
            bool: 路径是否有效
        """
        if not isinstance(path, str):
            return False
        if not path.strip():
            return False
        
        # 防止路径遍历攻击
        safe_path = os.path.normpath(path)
            
        return True
    
    def _ensure_output_directory(self, output_path: str) -> bool:
        """
        确保输出目录存在
        
        Args:
            output_path (str): 输出文件路径
            
        Returns:
            bool: 目录创建是否成功
        """
        try:
            output_dir = os.path.dirname(output_path)
            if output_dir:
                Path(output_dir).mkdir(parents=True, exist_ok=True)
            return True
        except (OSError, PermissionError) as e:
            logger.error(f"无法创建输出目录: {e}")
            return False
    
    def load_image(self, image_path: str) -> bool:
        """
        加载图像文件
        
        Args:
            image_path (str): 图像文件路径
            
        Returns:
            bool: 加载是否成功
            
        Raises:
            ImageProcessingError: 当路径无效或图像加载失败时
        """
        if not self._validate_path(image_path):
            raise ImageProcessingError(f"无效的图像路径: {image_path}")
        
        if not os.path.exists(image_path):
            raise ImageProcessingError(f"图像文件不存在: {image_path}")
        
        try:
            image = cv2.imread(image_path)
            if not self._validate_image(image):
                raise ImageProcessingError(f"无法加载图像: {image_path}")
            
            self._image = image.copy()
            self._original_image = image.copy()
            # logger.info(f"成功加载图像: {image_path}, 尺寸: {image.shape}")
            return True
            
        except cv2.error as e:
            raise ImageProcessingError(f"OpenCV错误: {e}")
        except Exception as e:
            raise ImageProcessingError(f"加载图像时发生未知错误: {e}")
    
    def adjust_brightness(self, brightness_factor: float) -> 'OptimalImage':
        """
        调整图像亮度
        
        Args:
            brightness_factor (float): 亮度调整因子 (0.0-2.0, 1.0为原始亮度)
            
        Returns:
            OptimalImage: 返回自身以支持链式调用
            
        Raises:
            ImageProcessingError: 当图像未加载或参数无效时
        """
        if not self._validate_image(self._image):
            raise ImageProcessingError("未加载有效图像")
        
        if not isinstance(brightness_factor, (int, float)):
            raise ImageProcessingError("亮度因子必须是数字")
        
        if brightness_factor < 0.0 or brightness_factor > 2.0:
            raise ImageProcessingError("亮度因子必须在0.0到2.0之间")
        
        try:
            # 使用安全的亮度调整方法
            adjusted = cv2.convertScaleAbs(self._image, alpha=brightness_factor, beta=0)
            self._image = adjusted
            # logger.info(f"亮度调整完成，因子: {brightness_factor}")
            return self
            
        except cv2.error as e:
            raise ImageProcessingError(f"亮度调整失败: {e}")
    
    def blend_with_image(self, other_image_path: str, alpha: float = 0.5) -> 'OptimalImage':
        """
        与另一张图像进行混合
        
        Args:
            other_image_path (str): 另一张图像的路径
            alpha (float): 混合权重 (0.0-1.0)
            
        Returns:
            OptimalImage: 返回自身以支持链式调用
            
        Raises:
            ImageProcessingError: 当图像未加载或参数无效时
        """
        if not self._validate_image(self._image):
            raise ImageProcessingError("未加载有效图像")
        
        if not self._validate_path(other_image_path):
            raise ImageProcessingError(f"无效的图像路径: {other_image_path}")
        
        if not isinstance(alpha, (int, float)) or alpha < 0.0 or alpha > 1.0:
            raise ImageProcessingError("混合权重必须在0.0到1.0之间")
        
        if not os.path.exists(other_image_path):
            raise ImageProcessingError(f"混合图像文件不存在: {other_image_path}")
        
        try:
            other_image = cv2.imread(other_image_path)
            if not self._validate_image(other_image):
                raise ImageProcessingError(f"无法加载混合图像: {other_image_path}")
            
            # 调整第二张图像的尺寸与当前图像一致
            if other_image.shape != self._image.shape:
                other_image = cv2.resize(other_image, 
                                       (self._image.shape[1], self._image.shape[0]))
            
            # 执行图像混合
            blended = cv2.addWeighted(self._image, alpha, other_image, 1 - alpha, 0)
            self._image = blended
            # logger.info(f"图像混合完成，权重: {alpha}")
            return self
            
        except cv2.error as e:
            raise ImageProcessingError(f"图像混合失败: {e}")
        except Exception as e:
            raise ImageProcessingError(f"图像混合时发生未知错误: {e}")
    
    def gaussian_blur(self, kernel_size: int, sigma: float) -> 'OptimalImage':
        """
        对图像应用高斯模糊
        
        Args:
            kernel_size (int): 核大小（必须为正奇数）
            sigma (float): 标准差
            
        Returns:
            OptimalImage: 返回自身以支持链式调用
            
        Raises:
            ImageProcessingError: 当图像未加载或参数无效时
        """
        if not self._validate_image(self._image):
            raise ImageProcessingError("未加载有效图像")
        
        if not isinstance(kernel_size, int) or kernel_size <= 0 or kernel_size % 2 == 0:
            raise ImageProcessingError("核大小必须是正奇数")
        
        if not isinstance(sigma, (int, float)) or sigma <= 0:
            raise ImageProcessingError("标准差必须是正数")
        
        try:
            blurred = cv2.GaussianBlur(self._image, (kernel_size, kernel_size), sigma)
            self._image = blurred
            # logger.info(f"高斯模糊完成，核大小: {kernel_size}, 标准差: {sigma}")
            return self
            
        except cv2.error as e:
            raise ImageProcessingError(f"高斯模糊失败: {e}")
    
    def save_image(self, output_path: str, quality: int = 95) -> bool:
        """
        保存处理后的图像
        
        Args:
            output_path (str): 输出文件路径
            quality (int): 图像质量 (1-100)
            
        Returns:
            bool: 保存是否成功
            
        Raises:
            ImageProcessingError: 当图像未加载或路径无效时
        """
        if not self._validate_image(self._image):
            raise ImageProcessingError("没有可保存的图像")
        
        if not self._validate_path(output_path):
            raise ImageProcessingError(f"无效的输出路径: {output_path}")
        
        if not isinstance(quality, int) or quality < 1 or quality > 100:
            raise ImageProcessingError("图像质量必须在1到100之间")
        
        if not self._ensure_output_directory(output_path):
            raise ImageProcessingError("无法创建输出目录")
        
        try:
            # 根据文件扩展名设置保存参数
            file_ext = os.path.splitext(output_path)[1].lower()
            if file_ext in ['.jpg', '.jpeg']:
                params = [cv2.IMWRITE_JPEG_QUALITY, quality]
            elif file_ext == '.png':
                params = [cv2.IMWRITE_PNG_COMPRESSION, 9]
            else:
                params = []
            
            success = cv2.imwrite(output_path, self._image, params)
            if not success:
                raise ImageProcessingError("保存图像失败")
            
            # logger.info(f"图像已保存到: {output_path}")
            return True
            
        except cv2.error as e:
            raise ImageProcessingError(f"保存图像失败: {e}")
        except Exception as e:
            raise ImageProcessingError(f"保存图像时发生未知错误: {e}")
    
    def get_image_info(self) -> dict:
        """
        获取当前图像信息
        
        Returns:
            dict: 包含图像信息的字典
        """
        if not self._validate_image(self._image):
            return {"error": "未加载有效图像"}
        
        return {
            "shape": self._image.shape,
            "dtype": str(self._image.dtype),
            "size": self._image.size,
            "channels": self._image.shape[2] if len(self._image.shape) == 3 else 1
        }
    
    def reset_to_original(self) -> 'OptimalImage':
        """
        重置到原始图像状态
        
        Returns:
            OptimalImage: 返回自身以支持链式调用
        """
        if self._validate_image(self._original_image):
            self._image = self._original_image.copy()
            # logger.info("图像已重置到原始状态")
        return self


def process_images_demo():
    """
    演示函数：展示图像处理功能
    """
    try:
        # logger.info("开始图像处理演示...")
        
        # 创建图像处理对象并加载图像
        processor = OptimalImage("./input/shark.jpg")
        
        # 显示原始图像信息
        print("原始图像信息:", processor.get_image_info())
        
        # 调整亮度
        processor.adjust_brightness(1.2)
        processor.save_image("./output/bright_shark.jpg")
        
        # 重置并与另一张图像混合
        processor.reset_to_original()
        processor.blend_with_image("./input/wave.jpg", alpha=0.5)
        processor.save_image("./output/blend_image.jpg")
        
        # 重置并应用高斯模糊
        processor.reset_to_original()
        processor.gaussian_blur(15, 2.0)
        processor.save_image("./output/blurred_shark.jpg")
        
        # 链式调用示例
        processor.reset_to_original()
        processor.adjust_brightness(0.8).gaussian_blur(5, 1.0)
        processor.save_image("./output/dark_blurred_shark.jpg")
        
        # logger.info("图像处理演示完成！")
        
    except ImageProcessingError as e:
        logger.error(f"图像处理错误: {e}")
    except Exception as e:
        logger.error(f"未知错误: {e}")


if __name__ == "__main__":
    process_images_demo()