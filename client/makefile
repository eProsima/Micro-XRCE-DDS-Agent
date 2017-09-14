DEFINES=-D DEBUG
CFLAGS=-Wall -O3 -Iinclude
COMPILER=gcc $(CFLAGS)

OBJECTS=build/mini_cdr.o build/serialization.o build/message.o

message_test:
	$(COMPILER) -c -o build/mini_cdr.o src/mini_cdr.c
	$(COMPILER) -c -o build/serialization.o src/serialization.c
	$(COMPILER) -c -o build/message.o src/message.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS)

mini_cdr_test:
	$(COMPILER) -c -o build/mini_cdr.o src/mini_cdr.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS)

mini_micro_cdr_performance_comparative_test:
	$(COMPILER) -c -o build/mini_cdr.o src/mini_cdr.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS) -lmicrocdr
