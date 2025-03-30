CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O1
LDFLAGS = -lm
JAVAC = javac

all: c_vector_dot_product java_vector_dot_product testcase_generator

c_vector_dot_product: C_vector_dot_product.c
	$(CC) $(CFLAGS) -o c_vector_dot_product C_vector_dot_product.c $(LDFLAGS)

java_vector_dot_product: JavaVectorDotProduct.java
	$(JAVAC) JavaVectorDotProduct.java

testcase_generator: testcase_generator.c
	$(CC) $(CFLAGS) -o testcase_generator testcase_generator.c $(LDFLAGS)

clean:
	rm -f c_vector_dot_product testcase_generator *.class

