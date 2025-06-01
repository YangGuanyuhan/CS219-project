import matplotlib.pyplot as plt
import numpy as np

# Image sizes
image_sizes = ["512x512", "1024x1024", "2048x2048", "4096x4096", "8192x8192"]

# OptimalImage performance data (in milliseconds)
optimal = {
    "Read":        [6.6679, 38.3700, 119.5381, 406.6183, 1670.5484],
    "Shallow Copy": [0.0014, 0.0002, 0.0002, 0.0004, 0.0003],
    "Deep Copy":   [0.3235, 1.2264, 3.1958, 16.9679, 73.9026],
    "Brightness":  [11.9692, 0.5451, 9.2021, 7.7592, 13.1041],
    "Gaussian Blur": [18.7313, 42.6129, 149.0329, 531.1278, 2103.5645],
}

# StandardImage performance data (in milliseconds)
standard = {
    "Read":        [10.1676, 33.0363, 109.7805, 418.1309, 1834.7151],
    "Copy Ctor":   [0.3886, 1.3023, 4.9715, 21.9153, 90.2779],
    "Deep Copy":   [0.2777, 0.8732, 3.9483, 22.5307, 91.8657],
    "Brightness":  [7.8507, 24.7069, 88.8659, 363.2795, 1425.4458],
}

def plot_comparison(title, optimal_data, standard_data, ylabel, log_scale=False):
    x = np.arange(len(image_sizes))
    width = 0.35

    fig, ax = plt.subplots()
    ax.bar(x - width/2, optimal_data, width, label='OptimalImage', color='skyblue')
    ax.bar(x + width/2, standard_data, width, label='StandardImage', color='salmon')

    ax.set_xlabel('Image Size')
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.set_xticks(x)
    ax.set_xticklabels(image_sizes)
    ax.legend()
    if log_scale:
        ax.set_yscale('log')
        ax.set_ylabel(f'{ylabel} (log scale)')
    ax.grid(True, linestyle='--', alpha=0.5)
    plt.tight_layout()
    plt.show()

# Generate plots
plot_comparison("Read Time Comparison", optimal["Read"], standard["Read"], "Time (ms)")
plot_comparison("Deep Copy Time Comparison", optimal["Deep Copy"], standard["Deep Copy"], "Time (ms)")
plot_comparison("Brightness Adjustment Time", optimal["Brightness"], standard["Brightness"], "Time (ms)")
plot_comparison("Gaussian Blur Time", optimal["Gaussian Blur"], [0]*len(image_sizes), "Time (ms)", log_scale=True)
plot_comparison("Shallow Copy vs Copy Constructor", optimal["Shallow Copy"], standard["Copy Ctor"], "Time (ms)")
