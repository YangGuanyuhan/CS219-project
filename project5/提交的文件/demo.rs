use anyhow::{Context, Result};
use std::time::Instant;

mod image_processor;

fn main() -> Result<()> {
    // 创建输出目录
    std::fs::create_dir_all("output").context("无法创建输出目录")?;

    // 读取输入图像
    let img1 = image::open("input/test0_256.bmp").context("无法读取图像1")?;

    // 创建图像处理器实例
    let processor = image_processor::ImageProcessor::new();

    println!("开始图像处理任务...");

    // 1. 调整亮度（并行化）
    println!("\n1. 调整亮度...");
    let start = Instant::now();
    let brightened = processor.adjust_brightness(&img1, 1.5)?;
    brightened.save("output/brightened.jpg")?;
    println!("亮度调整完成，耗时: {:?}", start.elapsed());

    // 2. 混合图像（并行化）
    println!("\n2. 混合图像...");
    let img3 = image::open("input/shark.jpg").context("无法读取图像3")?;
    let img4 = image::open("input/wave.jpg").context("无法读取图像4")?;
    let start = Instant::now();
    let blended = processor.blend_images(&img3, &img4, 0.5)?;
    blended.save("output/blended.jpg")?;
    println!("图像混合完成，耗时: {:?}", start.elapsed());

    // 3. 应用高斯模糊
    println!("\n3. 应用高斯模糊...");
    let start = Instant::now();
    let blurred = processor.gaussian_blur(&img1, 5, 2.0)?;
    blurred.save("output/blurred.jpg")?;
    println!("高斯模糊完成，耗时: {:?}", start.elapsed());

    // 4. 调整对比度（并行化）
    println!("\n4. 调整对比度...");
    let start = Instant::now();
    let contrasted = processor.adjust_contrast(&img3, 3.0)?;
    contrasted.save("output/contrasted.jpg")?;
    println!("对比度调整完成，耗时: {:?}", start.elapsed());

    // 5. 颜色反转（并行化）
    println!("\n5. 颜色反转...");
    let start = Instant::now();
    let inverted = processor.invert_colors(&img3)?;
    inverted.save("output/inverted.jpg")?;
    println!("颜色反转完成，耗时: {:?}", start.elapsed());

    // 6. 转换为灰度（并行化）
    println!("\n6. 转换为灰度...");
    let start = Instant::now();
    let grayscale = processor.to_grayscale(&img1)?;
    grayscale.save("output/grayscale.jpg")?;
    println!("灰度转换完成，耗时: {:?}", start.elapsed());

    // 7. 性能基准测试
    println!("\n7. 性能基准测试...");
    processor.benchmark_parallel_performance(&img1, 10)?;

    // 8. 简单的 rayon 并行计算演示
    println!("\n8. Rayon 并行计算演示...");
    demonstrate_rayon_basics();

    println!("\n所有图像处理任务完成！输出文件保存在 output 目录中。");
    Ok(())
}

/// 演示 rayon 基本并行计算功能
fn demonstrate_rayon_basics() {
    use rayon::prelude::*;
    use std::time::Instant;

    println!("演示 rayon 并行计算:");

    // 创建较小的数据集以避免溢出
    let data: Vec<u32> = (1..10_000).collect();

    // 顺序处理 - 使用 u64 避免溢出
    let start = Instant::now();
    let sum_sequential: u64 = data.iter().map(|&x| (x as u64) * (x as u64)).sum();
    let sequential_time = start.elapsed();

    // 并行处理 - 使用 u64 避免溢出
    let start = Instant::now();
    let sum_parallel: u64 = data.par_iter().map(|&x| (x as u64) * (x as u64)).sum();
    let parallel_time = start.elapsed();

    println!("  数据集大小: {} 个元素", data.len());
    println!(
        "  顺序计算结果: {}, 耗时: {:?}",
        sum_sequential, sequential_time
    );
    println!(
        "  并行计算结果: {}, 耗时: {:?}",
        sum_parallel, parallel_time
    );

    if parallel_time.as_nanos() > 0 {
        let speedup = sequential_time.as_nanos() as f64 / parallel_time.as_nanos() as f64;
        println!("  加速比: {:.2}x", speedup);
    }

    // 验证结果一致性
    assert_eq!(sum_sequential, sum_parallel);
    println!("  ✓ 顺序和并行计算结果一致");
}
