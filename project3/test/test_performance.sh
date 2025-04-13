#!/bin/bash
# BMP Image Processing Performance Test Script
# For comparing standard and optimized BMP processing programs

# Default parameters
WIDTH=1024
HEIGHT=1024
MODE=0
BRIGHTNESS=50
OUTPUT_DIR="../"
GENERATOR="../bmp_generator"
STANDARD_PROG="../build/standard_bmpedit"
OPTIMAL_PROG="../build/optimal_bmpedit"
TEST_IMG="test_image.bmp"
STD_OUTPUT="std_output.bmp"
OPT_OUTPUT="opt_output.bmp"
VERBOSE=0   # Verbose output mode flag

# Display usage
show_usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -w WIDTH    Image width (default: 1024)"
    echo "  -h HEIGHT   Image height (default: 1024)"
    echo "  -m MODE     Image generation mode (0-6, default: 0)"
    echo "  -b VALUE    Brightness adjustment value (default: 50)"
    echo "  -d DIR      Output directory (default: ../)"
    echo "  -s PROG     Standard program path (default: ../build/standard_bmpedit)"
    echo "  -o PROG     Optimized program path (default: ../build/optimal_bmpedit)"
    echo "  -g GEN      Image generator path (default: ../bmp_generator)"
    echo "  -v          Verbose output mode"
    echo ""
    echo "Image generation modes:"
    echo "  0 = Random pixels"
    echo "  1 = Gradient effect"
    echo "  2 = Checker pattern"
    echo "  3 = Concentric circles"
    echo "  4 = Noise effect"
    echo "  5 = Stripes"
    echo "  6 = Mandelbrot fractal"
    exit 1
}

# Parse command line arguments
while getopts ":w:h:m:b:d:s:o:g:v" opt; do
    case $opt in
        w) WIDTH="$OPTARG" ;;
        h) HEIGHT="$OPTARG" ;;
        m) MODE="$OPTARG" ;;
        b) BRIGHTNESS="$OPTARG" ;;
        d) OUTPUT_DIR="$OPTARG" ;;
        s) STANDARD_PROG="$OPTARG" ;;
        o) OPTIMAL_PROG="$OPTARG" ;;
        g) GENERATOR="$OPTARG" ;;
        v) VERBOSE=1 ;;
        \?) echo "Invalid option: -$OPTARG" >&2; show_usage ;;
        :) echo "Option -$OPTARG requires an argument" >&2; show_usage ;;
    esac
done

# Create temporary file paths
TEST_IMG_PATH="${OUTPUT_DIR}/${TEST_IMG}"
STD_OUTPUT_PATH="${OUTPUT_DIR}/${STD_OUTPUT}"
OPT_OUTPUT_PATH="${OUTPUT_DIR}/${OPT_OUTPUT}"

# Check if programs exist
if [ ! -f "$GENERATOR" ]; then
    echo "Error: Image generator program not found: $GENERATOR"
    exit 1
fi

if [ ! -f "$STANDARD_PROG" ]; then
    echo "Error: Standard program not found: $STANDARD_PROG"
    exit 1
fi

if [ ! -f "$OPTIMAL_PROG" ]; then
    echo "Error: Optimized program not found: $OPTIMAL_PROG"
    exit 1
fi

# Ensure output directory exists and is writable
if [ ! -d "$OUTPUT_DIR" ]; then
    echo "Error: Output directory does not exist: $OUTPUT_DIR"
    exit 1
fi

# Function to safely extract value with default
extract_time() {
    local pattern="$1"
    local file="$2"
    local value=$(grep -E "$pattern" "$file" | awk -F': ' '{print $2}' | awk '{print $1}')
    
    # Check if value is empty or not a number
    if [ -z "$value" ] || ! [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        # Use measured time instead or default to 0.001
        if [ -n "$3" ] && (echo "$3" | grep -q -E '^[0-9]+(\.[0-9]+)?$'); then
            echo "scale=3; $3 * 1000" | bc 2>/dev/null || echo "0.001" # Convert seconds to ms
        else
            echo "0.001"
        fi
    else
        echo "$value"
    fi
}

# Display test configuration
echo "========== BMP Processing Performance Test =========="
echo "Image size: ${WIDTH}x${HEIGHT}"
echo "Image generation mode: $MODE"
echo "Brightness adjustment value: $BRIGHTNESS"
echo "=================================================="

# Step 1: Generate test image
echo "Generating test image..."
$GENERATOR -o "$TEST_IMG_PATH" -w "$WIDTH" -h "$HEIGHT" -m "$MODE"
if [ $? -ne 0 ]; then
    echo "Error: Failed to generate test image"
    exit 1
fi
echo "Test image generated: $TEST_IMG_PATH"

# Step 2: Process image with standard program using measured time
echo -e "\nRunning standard program..."
# Use /usr/bin/time for more precise timing
/usr/bin/time -f "%e" $STANDARD_PROG -i "$TEST_IMG_PATH" -o "$STD_OUTPUT_PATH" -op add "$BRIGHTNESS" > .std_output.tmp 2> .std_time.tmp
std_time=$(cat .std_time.tmp)

# Try to extract internal timing data from the output
std_read=$(extract_time "image reading time|图像读取时间" .std_output.tmp "$std_time")
std_process=$(extract_time "image processing time|图像处理时间" .std_output.tmp "$std_time")
std_write=$(extract_time "image writing time|图像写入时间" .std_output.tmp "$std_time")
std_total=$(extract_time "total execution time|总执行时间" .std_output.tmp "$std_time")

# Step 3: Process image with optimized program using measured time
echo -e "\nRunning optimized program..."
# Use /usr/bin/time for more precise timing
/usr/bin/time -f "%e" $OPTIMAL_PROG -i "$TEST_IMG_PATH" -o "$OPT_OUTPUT_PATH" -op add "$BRIGHTNESS" > .opt_output.tmp 2> .opt_time.tmp
opt_time=$(cat .opt_time.tmp)

# Try to extract internal timing data from the output
opt_read=$(extract_time "image reading time|图像读取时间" .opt_output.tmp "$opt_time")
opt_process=$(extract_time "image processing time|图像处理时间" .opt_output.tmp "$opt_time")
opt_write=$(extract_time "image writing time|图像写入时间" .opt_output.tmp "$opt_time")
opt_total=$(extract_time "total execution time|总执行时间" .opt_output.tmp "$opt_time")

# Step 4: Compare results
echo -e "\n========== Performance Comparison Results =========="
echo "Standard version execution time: $std_time seconds"
echo "Optimized version execution time: $opt_time seconds"

# Output detailed timing data if in verbose mode
if [ $VERBOSE -eq 1 ]; then
    echo -e "\nDetailed timing breakdown:"
    echo "Standard reading time:    $std_read ms"
    echo "Standard processing time: $std_process ms"
    echo "Standard writing time:    $std_write ms" 
    echo "Standard total time:      $std_total ms"
    echo ""
    echo "Optimized reading time:    $opt_read ms"
    echo "Optimized processing time: $opt_process ms"
    echo "Optimized writing time:    $opt_write ms"
    echo "Optimized total time:      $opt_total ms"
fi

# Calculate performance improvement using awk for better floating point handling
# Check if both times are valid numeric values before calculation
if [[ "$std_time" =~ ^[0-9]+(\.[0-9]+)?$ ]] && [[ "$opt_time" =~ ^[0-9]+(\.[0-9]+)?$ ]] && (echo "$std_time" | grep -q -v "^0$") && (echo "$opt_time" | grep -q -v "^0$"); then
    speedup=$(awk -v std="$std_time" -v opt="$opt_time" 'BEGIN {printf "%.2f", std/opt}')
    improvement=$(awk -v std="$std_time" -v opt="$opt_time" 'BEGIN {printf "%.2f", (std-opt)*100/std}')
    echo -e "\nSpeed-up ratio: ${speedup}x"
    echo "Performance improvement: ${improvement}%"
else
    echo "Cannot calculate performance improvement (time values are invalid)"
fi

echo -e "\nStandard version output: $STD_OUTPUT_PATH"
echo "Optimized version output: $OPT_OUTPUT_PATH"
echo "=================================================="

# Clean up temporary files
rm -f .std_output.tmp .opt_output.tmp .std_time.tmp .opt_time.tmp

exit 0