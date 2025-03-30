#!/bin/bash

# Ensure programs are compiled with debug information
make clean
make CFLAGS="-Wall -Wextra -std=c99 -I../src -DTESTING -g" test_dot_product vector_dot_product

# Run Valgrind memory check
make valgrind_check

echo "Valgrind memory check completed!"
