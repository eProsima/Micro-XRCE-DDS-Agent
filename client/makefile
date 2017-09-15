DEFINES=-D DEBUG
CFLAGS=-Wall -O3 -Iinclude/client
COMPILER=gcc $(CFLAGS) $(DEFINES)

OBJECTS=build/mini_cdr.o build/serialization.o build/message.o

message_test:
	$(COMPILER) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(COMPILER) -c -o build/serialization.o src/C/client/serialization.c
	$(COMPILER) -c -o build/message.o src/C/client/message.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS)

mini_cdr_test:
	$(COMPILER) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS)

mini_micro_cdr_performance_comparative_test:
	$(COMPILER) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(COMPILER) -c -o build/$@.o test/$@.c
	$(COMPILER) -o    build/$@ build/$@.o $(OBJECTS) -lmicrocdr
