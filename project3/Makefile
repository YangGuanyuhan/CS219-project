# Makefile for BMP image processing project

# Compiler and options
CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm
OPT_LDFLAGS = -lm -lpthread

# Targets and source files
STANDARD_TARGET = standard_bmpedit
STANDARD_SRC = src/standard_bmpedit_combined.c

OPTIMAL_TARGET = optimal_bmpedit
OPTIMAL_SRC = src/optimal_bmpedit.c

GENERATOR_TARGET = bmp_generator
GENERATOR_SRC = src/bmp_generator.c

# Default target
all: $(STANDARD_TARGET) $(OPTIMAL_TARGET) $(GENERATOR_TARGET)

# Build rule for standard version
$(STANDARD_TARGET): $(STANDARD_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	mkdir -p build
	mv $(STANDARD_TARGET) build/

# Build rule for optimized version
$(OPTIMAL_TARGET): $(OPTIMAL_SRC)
	$(CC) $(CFLAGS) -march=native -O3 -o $@ $^ $(OPT_LDFLAGS)
	mkdir -p build
	mv $(OPTIMAL_TARGET) build/

# Build rule for image generator
$(GENERATOR_TARGET): $(GENERATOR_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Clean target
clean:
	rm -f build/$(STANDARD_TARGET) build/$(OPTIMAL_TARGET) $(GENERATOR_TARGET)

.PHONY: all clean