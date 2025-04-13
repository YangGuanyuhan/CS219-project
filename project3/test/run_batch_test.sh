#!/bin/bash
# BMP Image Processing Batch Performance Test Script
# Automatically tests different image sizes and saves results to a CSV file

# Results file
RESULTS_FILE="../test_results.csv"

# Test image size list
IMAGE_SIZES=(512 1024 2048 4096 8192)

# Test mode (gradient image)
MODE=1

# Brightness adjustment value
BRIGHTNESS=50

# Clear results file and write header
echo "image_size,std_read,std_process,std_write,std_total,opt_read,opt_process,opt_write,opt_total" > $RESULTS_FILE

# Function to safely extract numeric value with default
extract_time() {
    local pattern="$1"
    local file="$2"
    
    # Use grep with extended regex and extract the value
    local value=$(grep -E "$pattern" "$file" | awk '{print $4}')
    
    # Check if value is empty or not a number
    if [ -z "$value" ] || ! [[ "$value" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
        echo "0.001" # Default small value to avoid division by zero
    else
        echo "$value"
    fi
}

# Test different image sizes
for size in "${IMAGE_SIZES[@]}"; do
    echo "Testing image size: ${size}x${size}"
    
    # Run test script and capture standard output
    ./test_performance.sh -w $size -h $size -m $MODE -b $BRIGHTNESS -v > temp_output.txt
    
    # Extract timing data with error checking
    std_read=$(extract_time "Standard reading time" temp_output.txt)
    std_process=$(extract_time "Standard processing time" temp_output.txt)
    std_write=$(extract_time "Standard writing time" temp_output.txt)
    std_total=$(extract_time "Standard total time" temp_output.txt)
    
    opt_read=$(extract_time "Optimized reading time" temp_output.txt)
    opt_process=$(extract_time "Optimized processing time" temp_output.txt)
    opt_write=$(extract_time "Optimized writing time" temp_output.txt)
    opt_total=$(extract_time "Optimized total time" temp_output.txt)
    
    # Print extracted values for verification
    echo "Extracted timing values:"
    echo "Standard: read=$std_read ms, process=$std_process ms, write=$std_write ms, total=$std_total ms"
    echo "Optimized: read=$opt_read ms, process=$opt_process ms, write=$opt_write ms, total=$opt_total ms"
    
    # Write results to CSV file
    echo "$size,$std_read,$std_process,$std_write,$std_total,$opt_read,$opt_process,$opt_write,$opt_total" >> $RESULTS_FILE
    
    echo "Completed test for ${size}x${size}"
    echo "----------------------------------------"
done

# Clean up temporary files
rm -f temp_output.txt

echo "All tests completed. Results saved to $RESULTS_FILE"