CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I../src -DTESTING
LDFLAGS = -lm

# Build targets
all: test_dot_product vector_dot_product 

# Compile unit tests
test_dot_product: test_dot_product.c
	$(CC) $(CFLAGS) -o test_dot_product test_dot_product.c $(LDFLAGS)

# Compile main program
vector_dot_product: ../src/vector_dot_product.c
	$(CC) -Wall -Wextra -std=c99 -I../src -o vector_dot_product ../src/vector_dot_product.c $(LDFLAGS)

# Run tests
run: test_dot_product vector_dot_product
	@echo "Running unit tests..."
	./test_dot_product
	@echo "\nRunning main program tests..."
	./vector_dot_product < test_input.txt
	@echo "\nRunning error handling tests..."
	./vector_dot_product < test_error_input.txt || true

# Valgrind memory analysis
valgrind_check: test_dot_product vector_dot_product
	@echo "Running Valgrind memory analysis on unit tests..."
	valgrind --leak-check=full ./test_dot_product
	@echo "\nRunning Valgrind memory analysis on vector_dot_product with test input..."
	valgrind --leak-check=full ./vector_dot_product < test_input.txt
	@echo "\nRunning Valgrind memory analysis on vector_dot_product with error input..."
	valgrind --leak-check=full ./vector_dot_product < test_error_input.txt || true

# Clean up
clean:
	rm -f test_dot_product vector_dot_product performance_test
