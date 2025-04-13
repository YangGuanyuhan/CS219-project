#!/usr/bin/env python3
"""
BMP Processing Performance Visualization Script
For plotting performance comparison charts of different image sizes
- This version only depends on Python standard library and matplotlib
"""

import matplotlib.pyplot as plt
import csv
import os
from math import log2

# Read test result data
results_file = 'test_results.csv'

if not os.path.exists(results_file):
    print(f"Error: Result file not found: {results_file}")
    print("Please run the test script first: test/run_batch_test.sh")
    exit(1)

# Use CSV module to read data
image_sizes = []
std_read = []
std_process = []
std_write = []
std_total = []
opt_read = []
opt_process = []
opt_write = []
opt_total = []

try:
    with open(results_file, 'r') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            image_sizes.append(int(row['image_size']))
            std_read.append(float(row['std_read']))
            std_process.append(float(row['std_process']))
            std_write.append(float(row['std_write']))
            std_total.append(float(row['std_total']))
            opt_read.append(float(row['opt_read']))
            opt_process.append(float(row['opt_process']))
            opt_write.append(float(row['opt_write']))
            opt_total.append(float(row['opt_total']))
except Exception as e:
    print(f"Error reading data: {e}")
    exit(1)

# Print data table
print("Test Result Data:")
print("Image Size | Std Read   | Std Process| Std Write  | Std Total  | Opt Read   | Opt Process| Opt Write  | Opt Total")
print("-" * 100)
for i in range(len(image_sizes)):
    print(f"{image_sizes[i]:8d} | {std_read[i]:10.2f} | {std_process[i]:10.2f} | {std_write[i]:10.2f} | {std_total[i]:10.2f} | {opt_read[i]:10.2f} | {opt_process[i]:10.2f} | {opt_write[i]:10.2f} | {opt_total[i]:10.2f}")
print()

# Set chart style
plt.figure(figsize=(15, 10))

# Create layout
fig, axes = plt.subplots(2, 2, figsize=(15, 10))
fig.suptitle('BMP Image Processing Performance Comparison', fontsize=16)

# Create subplot 1: Reading time comparison
ax1 = axes[0, 0]
ax1.plot(image_sizes, std_read, 'o-', label='Standard', linewidth=2, markersize=8)
ax1.plot(image_sizes, opt_read, 's-', label='Optimized', linewidth=2, markersize=8)
ax1.set_title('Image Reading Time Comparison')
ax1.set_xlabel('Image Size (pixels)')
ax1.set_ylabel('Time (ms)')
ax1.set_xscale('log')
ax1.set_yscale('log')
ax1.legend()
ax1.grid(True)

# Create subplot 2: Processing time comparison
ax2 = axes[0, 1]
ax2.plot(image_sizes, std_process, 'o-', label='Standard', linewidth=2, markersize=8)
ax2.plot(image_sizes, opt_process, 's-', label='Optimized', linewidth=2, markersize=8)
ax2.set_title('Image Processing Time Comparison')
ax2.set_xlabel('Image Size (pixels)')
ax2.set_ylabel('Time (ms)')
ax2.set_xscale('log')
ax2.set_yscale('log')
ax2.legend()
ax2.grid(True)

# Create subplot 3: Writing time comparison
ax3 = axes[1, 0]
ax3.plot(image_sizes, std_write, 'o-', label='Standard', linewidth=2, markersize=8)
ax3.plot(image_sizes, opt_write, 's-', label='Optimized', linewidth=2, markersize=8)
ax3.set_title('Image Writing Time Comparison')
ax3.set_xlabel('Image Size (pixels)')
ax3.set_ylabel('Time (ms)')
ax3.set_xscale('log')
ax3.set_yscale('log')
ax3.legend()
ax3.grid(True)

# Create subplot 4: Total execution time comparison
ax4 = axes[1, 1]
ax4.plot(image_sizes, std_total, 'o-', label='Standard', linewidth=2, markersize=8)
ax4.plot(image_sizes, opt_total, 's-', label='Optimized', linewidth=2, markersize=8)
ax4.set_title('Total Execution Time Comparison')
ax4.set_xlabel('Image Size (pixels)')
ax4.set_ylabel('Time (ms)')
ax4.set_xscale('log')
ax4.set_yscale('log')
ax4.legend()
ax4.grid(True)

# Calculate speedup ratios
speedup_read = [s / o for s, o in zip(std_read, opt_read)]
speedup_process = [s / o for s, o in zip(std_process, opt_process)]
speedup_write = [s / o for s, o in zip(std_write, opt_write)]
speedup_total = [s / o for s, o in zip(std_total, opt_total)]

# Display speedup statistics
print("Speedup Statistics:")
print(f"Reading phase speedup: Min={min(speedup_read):.2f}x, Max={max(speedup_read):.2f}x, Avg={sum(speedup_read)/len(speedup_read):.2f}x")
print(f"Processing phase speedup: Min={min(speedup_process):.2f}x, Max={max(speedup_process):.2f}x, Avg={sum(speedup_process)/len(speedup_process):.2f}x")
print(f"Writing phase speedup: Min={min(speedup_write):.2f}x, Max={max(speedup_write):.2f}x, Avg={sum(speedup_write)/len(speedup_write):.2f}x")
print(f"Overall speedup: Min={min(speedup_total):.2f}x, Max={max(speedup_total):.2f}x, Avg={sum(speedup_total)/len(speedup_total):.2f}x")

# Create speedup ratio chart
plt.figure(figsize=(10, 6))
plt.plot(image_sizes, speedup_read, 'o-', label='Reading Phase', linewidth=2)
plt.plot(image_sizes, speedup_process, 's-', label='Processing Phase', linewidth=2)
plt.plot(image_sizes, speedup_write, '^-', label='Writing Phase', linewidth=2)
plt.plot(image_sizes, speedup_total, 'D-', label='Overall', linewidth=2)
plt.title('Speedup Ratios for Different Image Sizes')
plt.xlabel('Image Size (pixels)')
plt.ylabel('Speedup Ratio (x times)')
plt.xscale('log')
plt.grid(True)
plt.legend()

# Save charts
plt.tight_layout(rect=[0, 0.03, 1, 0.95])
fig.savefig('performance_comparison.png')
plt.figure(1).savefig('speedup_comparison.png')

print("\nCharts have been saved as 'performance_comparison.png' and 'speedup_comparison.png'")

# Display charts
plt.show()