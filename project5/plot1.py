import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

from io import StringIO

data_str = """
Language,Time(ms),RelativePerformance,Status,datasize
Python,0.027,1.00,Success,256x256
C++,0.593,21.96,Success,256x256
Rust,0.232,8.59,Success,256x256
Language,Time(ms),RelativePerformance,Status,datasize
Python,0.099,1.00,Success,512x512
C++,0.982,9.92,Success,512x512
Rust,9.359,94.54,Success,512x512
Language,Time(ms),RelativePerformance,Status,datasize
Python,0.607,1.00,Success,1024x1024
C++,0.991,1.63,Success,1024x1024
Rust,11.820,19.47,Success,1024x1024
Language,Time(ms),RelativePerformance,Status,datasize
Python,1.745,1.00,Success,2048x2048
C++,4.355,2.50,Success,2048x2048
Rust,22.137,12.69,Success,2048x2048
Language,Time(ms),RelativePerformance,Status,datasize
Python,10.088,1.32,Success,4096x4096
C++,7.634,1.00,Success,4096x4096
Rust,63.456,8.31,Success,4096x4096
Language,Time(ms),RelativePerformance,Status,datasize
Python,46.953,5.23,Success,8192x8192
C++,8.982,1.00,Success,8192x8192
Rust,253.828,28.26,Success,8192x8192
Language,Time(ms),RelativePerformance,Status,datasize
Python,184.804,1.00,Success,16384x16384
C++,N/A,N/A,Failed,16384x16384
Rust,N/A,N/A,Failed,16384x16384
"""

# 读取数据
df = pd.read_csv(StringIO(data_str))
df = df[df["Status"] == "Success"]
df["Time(ms)"] = pd.to_numeric(df["Time(ms)"], errors="coerce")
df["RelativePerformance"] = pd.to_numeric(df["RelativePerformance"], errors="coerce")

# 排序图像大小（datasize）方便后续图表顺序一致
df["datasize"] = pd.Categorical(df["datasize"], 
                                categories=sorted(df["datasize"].unique(), key=lambda x: int(x.split('x')[0])), 
                                ordered=True)

# --- 折线图：每种语言在不同图像大小下的时间 ---
plt.figure(figsize=(10, 6))
for lang in df["Language"].unique():
    subset = df[df["Language"] == lang]
    plt.plot(subset["datasize"], subset["Time(ms)"], marker='o', label=lang)
plt.yscale("log")
plt.xlabel("Image Size")
plt.ylabel("Time (ms, log scale)")
plt.title("Execution Time per Language (log scale)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# --- 柱状图：每种图像大小，展示三种语言 ---
plt.figure(figsize=(12, 6))
pivot_time = df.pivot(index="datasize", columns="Language", values="Time(ms)")
pivot_time.plot(kind="bar", logy=True)
plt.ylabel("Time (ms, log scale)")
plt.title("Time Comparison across Languages per Image Size")
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()

# --- 加速比图（使用 RelativePerformance） ---
plt.figure(figsize=(10, 6))
for lang in df["Language"].unique():
    if lang == "Python":
        continue  # Skip baseline
    subset = df[df["Language"] == lang]
    plt.plot(subset["datasize"], subset["RelativePerformance"], marker='o', label=lang)
plt.yscale("log")
plt.xlabel("Image Size")
plt.ylabel("Relative Performance vs Python (log scale)")
plt.title("Relative Performance (Speedup) Compared to Python")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
