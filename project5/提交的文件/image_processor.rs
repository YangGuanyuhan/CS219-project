use anyhow::Result;
use image::{DynamicImage, ImageBuffer, Rgb};
use imageproc::filter::gaussian_blur_f32;
use rayon::prelude::*;

pub struct ImageProcessor;

impl ImageProcessor {
    // 并行化阈值：当像素数量超过此值时才使用并行处理
    const PARALLEL_THRESHOLD: u32 = 100_000; // 100K像素

    pub fn new() -> Self {
        ImageProcessor
    }

    /// 调整图像亮度（根据数据量自动选择并行或顺序处理）
    ///
    /// # 参数
    /// * `image` - 输入图像
    /// * `factor` - 亮度调整因子 (1.0 表示原始亮度)
    ///
    /// # 返回
    /// 返回调整亮度后的新图像
    pub fn adjust_brightness(&self, image: &DynamicImage, factor: f32) -> Result<DynamicImage> {
        if factor <= 0.0 {
            return Err(anyhow::anyhow!("亮度因子必须大于0"));
        }

        let rgb_image = image.to_rgb8();
        let (width, height) = rgb_image.dimensions();
        let pixel_count = width * height;

        let mut output = ImageBuffer::new(width, height);

        // 根据像素数量决定是否使用并行处理
        if pixel_count > Self::PARALLEL_THRESHOLD {
            // 使用并行迭代器处理所有像素
            output
                .enumerate_pixels_mut()
                .collect::<Vec<_>>()
                .par_iter_mut()
                .for_each(|(x, y, pixel)| {
                    let input_pixel = rgb_image.get_pixel(*x, *y);
                    **pixel = Rgb([
                        (input_pixel[0] as f32 * factor).min(255.0) as u8,
                        (input_pixel[1] as f32 * factor).min(255.0) as u8,
                        (input_pixel[2] as f32 * factor).min(255.0) as u8,
                    ]);
                });
        } else {
            // 使用普通迭代器处理所有像素
            for (x, y, pixel) in output.enumerate_pixels_mut() {
                let input_pixel = rgb_image.get_pixel(x, y);
                *pixel = Rgb([
                    (input_pixel[0] as f32 * factor).min(255.0) as u8,
                    (input_pixel[1] as f32 * factor).min(255.0) as u8,
                    (input_pixel[2] as f32 * factor).min(255.0) as u8,
                ]);
            }
        }

        Ok(DynamicImage::ImageRgb8(output))
    }

    /// 混合两张图像（根据数据量自动选择并行或顺序处理）
    ///
    /// # 参数
    /// * `image1` - 第一张输入图像
    /// * `image2` - 第二张输入图像
    /// * `alpha` - 混合因子 (0.0-1.0)
    ///
    /// # 返回
    /// 返回混合后的新图像
    pub fn blend_images(
        &self,
        image1: &DynamicImage,
        image2: &DynamicImage,
        alpha: f32,
    ) -> Result<DynamicImage> {
        if !(0.0..=1.0).contains(&alpha) {
            return Err(anyhow::anyhow!("混合因子必须在0.0到1.0之间"));
        }

        let rgb1 = image1.to_rgb8();
        let rgb2 = image2.to_rgb8();

        if rgb1.dimensions() != rgb2.dimensions() {
            return Err(anyhow::anyhow!("输入图像尺寸必须相同"));
        }

        let (width, height) = rgb1.dimensions();
        let pixel_count = width * height;
        let mut output = ImageBuffer::new(width, height);

        // 根据像素数量决定是否使用并行处理
        if pixel_count > Self::PARALLEL_THRESHOLD {
            // 使用并行迭代器处理像素
            output
                .enumerate_pixels_mut()
                .collect::<Vec<_>>()
                .par_iter_mut()
                .for_each(|(x, y, pixel)| {
                    let p1 = rgb1.get_pixel(*x, *y);
                    let p2 = rgb2.get_pixel(*x, *y);

                    **pixel = Rgb([
                        ((p1[0] as f32 * alpha + p2[0] as f32 * (1.0 - alpha)) as u8),
                        ((p1[1] as f32 * alpha + p2[1] as f32 * (1.0 - alpha)) as u8),
                        ((p1[2] as f32 * alpha + p2[2] as f32 * (1.0 - alpha)) as u8),
                    ]);
                });
        } else {
            // 使用普通迭代器处理像素
            for (x, y, pixel) in output.enumerate_pixels_mut() {
                let p1 = rgb1.get_pixel(x, y);
                let p2 = rgb2.get_pixel(x, y);

                *pixel = Rgb([
                    ((p1[0] as f32 * alpha + p2[0] as f32 * (1.0 - alpha)) as u8),
                    ((p1[1] as f32 * alpha + p2[1] as f32 * (1.0 - alpha)) as u8),
                    ((p1[2] as f32 * alpha + p2[2] as f32 * (1.0 - alpha)) as u8),
                ]);
            }
        }

        Ok(DynamicImage::ImageRgb8(output))
    }

    /// 应用高斯模糊
    ///
    /// # 参数
    /// * `image` - 输入图像
    /// * `kernel_size` - 核大小（必须是正奇数，如 3, 5, 7）
    /// * `sigma` - 高斯分布的标准差
    ///
    /// # 返回
    /// 返回模糊后的新图像
    pub fn gaussian_blur(
        &self,
        image: &DynamicImage,
        kernel_size: i32,
        sigma: f32,
    ) -> Result<DynamicImage> {
        if kernel_size <= 0 {
            return Err(anyhow::anyhow!("核大小必须是正数"));
        }
        if kernel_size % 2 == 0 {
            return Err(anyhow::anyhow!("核大小必须是奇数"));
        }
        if sigma <= 0.0 {
            return Err(anyhow::anyhow!("sigma必须大于0"));
        }

        let rgb_image = image.to_rgb8();
        let blurred = gaussian_blur_f32(&rgb_image, sigma);
        Ok(DynamicImage::ImageRgb8(blurred))
    }

    /// 调整图像对比度（根据数据量自动选择并行或顺序处理）
    ///
    /// # 参数
    /// * `image` - 输入图像
    /// * `contrast` - 对比度调整因子 (1.0 表示原始对比度)
    ///
    /// # 返回
    /// 返回调整对比度后的新图像
    pub fn adjust_contrast(&self, image: &DynamicImage, contrast: f32) -> Result<DynamicImage> {
        if contrast < 0.0 {
            return Err(anyhow::anyhow!("对比度因子不能为负数"));
        }

        let rgb_image = image.to_rgb8();
        let (width, height) = rgb_image.dimensions();
        let pixel_count = width * height;
        let mut output = ImageBuffer::new(width, height);

        // 根据像素数量决定是否使用并行处理
        if pixel_count > Self::PARALLEL_THRESHOLD {
            // 使用并行迭代器处理所有像素
            output
                .enumerate_pixels_mut()
                .collect::<Vec<_>>()
                .par_iter_mut()
                .for_each(|(x, y, pixel)| {
                    let input_pixel = rgb_image.get_pixel(*x, *y);
                    **pixel = Rgb([
                        ((input_pixel[0] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0)
                            as u8,
                        ((input_pixel[1] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0)
                            as u8,
                        ((input_pixel[2] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0)
                            as u8,
                    ]);
                });
        } else {
            // 使用普通迭代器处理所有像素
            for (x, y, pixel) in output.enumerate_pixels_mut() {
                let input_pixel = rgb_image.get_pixel(x, y);
                *pixel = Rgb([
                    ((input_pixel[0] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0) as u8,
                    ((input_pixel[1] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0) as u8,
                    ((input_pixel[2] as f32 - 128.0) * contrast + 128.0).clamp(0.0, 255.0) as u8,
                ]);
            }
        }

        Ok(DynamicImage::ImageRgb8(output))
    }

    /// 颜色反转（根据数据量自动选择并行或顺序处理）
    ///
    /// # 参数
    /// * `image` - 输入图像
    ///
    /// # 返回
    /// 返回颜色反转后的新图像
    pub fn invert_colors(&self, image: &DynamicImage) -> Result<DynamicImage> {
        let rgb_image = image.to_rgb8();
        let (width, height) = rgb_image.dimensions();
        let pixel_count = width * height;
        let mut output = ImageBuffer::new(width, height);

        // 根据像素数量决定是否使用并行处理
        if pixel_count > Self::PARALLEL_THRESHOLD {
            // 使用并行迭代器反转所有像素的颜色
            output
                .enumerate_pixels_mut()
                .collect::<Vec<_>>()
                .par_iter_mut()
                .for_each(|(x, y, pixel)| {
                    let input_pixel = rgb_image.get_pixel(*x, *y);
                    **pixel = Rgb([
                        255 - input_pixel[0],
                        255 - input_pixel[1],
                        255 - input_pixel[2],
                    ]);
                });
        } else {
            // 使用普通迭代器反转所有像素的颜色
            for (x, y, pixel) in output.enumerate_pixels_mut() {
                let input_pixel = rgb_image.get_pixel(x, y);
                *pixel = Rgb([
                    255 - input_pixel[0],
                    255 - input_pixel[1],
                    255 - input_pixel[2],
                ]);
            }
        }

        Ok(DynamicImage::ImageRgb8(output))
    }

    /// 转换为灰度图像（根据数据量自动选择并行或顺序处理）
    ///
    /// # 参数
    /// * `image` - 输入图像
    ///
    /// # 返回
    /// 返回灰度图像
    pub fn to_grayscale(&self, image: &DynamicImage) -> Result<DynamicImage> {
        let rgb_image = image.to_rgb8();
        let (width, height) = rgb_image.dimensions();
        let pixel_count = width * height;
        let mut output = ImageBuffer::new(width, height);

        // 根据像素数量决定是否使用并行处理
        if pixel_count > Self::PARALLEL_THRESHOLD {
            // 使用并行迭代器处理所有像素
            output
                .enumerate_pixels_mut()
                .collect::<Vec<_>>()
                .par_iter_mut()
                .for_each(|(x, y, pixel)| {
                    let input_pixel = rgb_image.get_pixel(*x, *y);
                    // 使用标准的RGB到灰度转换公式
                    let gray_value = (0.299 * input_pixel[0] as f32
                        + 0.587 * input_pixel[1] as f32
                        + 0.114 * input_pixel[2] as f32) as u8;
                    **pixel = Rgb([gray_value, gray_value, gray_value]);
                });
        } else {
            // 使用普通迭代器处理所有像素
            for (x, y, pixel) in output.enumerate_pixels_mut() {
                let input_pixel = rgb_image.get_pixel(x, y);
                // 使用标准的RGB到灰度转换公式
                let gray_value = (0.299 * input_pixel[0] as f32
                    + 0.587 * input_pixel[1] as f32
                    + 0.114 * input_pixel[2] as f32) as u8;
                *pixel = Rgb([gray_value, gray_value, gray_value]);
            }
        }

        Ok(DynamicImage::ImageRgb8(output))
    }

    /// 性能测试函数：测量并行和顺序处理的性能差异
    ///
    /// # 参数
    /// * `image` - 输入图像
    /// * `iterations` - 测试迭代次数
    pub fn benchmark_parallel_performance(
        &self,
        image: &DynamicImage,
        iterations: usize,
    ) -> Result<()> {
        use std::time::Instant;

        println!("开始并行性能测试，迭代 {} 次...", iterations);

        // 测试并行亮度调整
        let start = Instant::now();
        for _ in 0..iterations {
            let _ = self.adjust_brightness(image, 1.2)?;
        }
        let parallel_duration = start.elapsed();

        println!(
            "并行亮度调整 {} 次耗时: {:?}",
            iterations, parallel_duration
        );
        println!(
            "平均每次处理时间: {:?}",
            parallel_duration / iterations as u32
        );

        Ok(())
    }
}
