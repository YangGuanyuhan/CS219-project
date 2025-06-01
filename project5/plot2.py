import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
from io import StringIO

# 输入数据
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

# 数据预处理
df = pd.read_csv(StringIO(data_str))
df = df[df['Status'] == 'Success']
df['Time(ms)'] = pd.to_numeric(df['Time(ms)'], errors='coerce')

# 保持图像尺寸排序
df['datasize'] = pd.Categorical(df['datasize'], 
    categories=sorted(df['datasize'].unique(), key=lambda x: int(x.split('x')[0])), 
    ordered=True)

# ========================
# ✅ 1. Delta Time Plot
# ========================
plt.figure(figsize=(10, 6))
for lang in df['Language'].unique():
    subset = df[df['Language'] == lang].sort_values('datasize')
    times = subset['Time(ms)'].values
    sizes = subset['datasize'].values
    delta = np.divide(times[1:], times[:-1])
    plt.plot(sizes[1:], delta, marker='o', label=lang)

plt.xlabel("Image Size")
plt.ylabel("Time Ratio (Current / Previous)")
plt.title("Delta Time Increase per Image Size")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

# ========================
# ✅ 2. Area Plot
# ========================
pivot = df.pivot(index="datasize", columns="Language", values="Time(ms)")
pivot = pivot.fillna(0)

pivot.plot(kind='area', figsize=(10, 6), alpha=0.6)
plt.title("Area Plot of Execution Time per Language")
plt.xlabel("Image Size")
plt.ylabel("Time (ms)")
plt.grid(True)
plt.tight_layout()
plt.show()

# ========================
# ✅ 3. 3D Plot
# ========================
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

languages = df['Language'].unique()
lang_map = {lang: i for i, lang in enumerate(languages)}
df['lang_index'] = df['Language'].map(lang_map)
df['size_index'] = df['datasize'].apply(lambda x: int(x.split('x')[0]))

# 删除无效时间值
valid_df = df.dropna(subset=["Time(ms)"])

xs = valid_df['lang_index']
ys = valid_df['size_index']
zs = valid_df['Time(ms)']

ax.bar3d(xs, ys, np.zeros_like(zs), 0.4, 200, zs, shade=True)
ax.set_xticks(list(lang_map.values()))
ax.set_xticklabels(list(lang_map.keys()))
ax.set_xlabel('Language')
ax.set_ylabel('Image Size')
ax.set_zlabel('Time (ms)')
ax.set_title("3D Execution Time: Language vs Image Size")

plt.tight_layout()
plt.show()
