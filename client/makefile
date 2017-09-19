CFLAGS_FAST=-Wall -O3 -Iinclude/ -Itest/
CFLAGS_DEBUG=-Wall -g -Iinclude/ -Itest/ -m32 -D DEBUG
C_FAST=gcc $(CFLAGS_FAST)
C_DEBUG=gcc $(CFLAGS_DEBUG)

prototype:
	$(C_DEBUG) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(C_DEBUG) -c -o build/dynamic_buffer.o src/C/client/dynamic_buffer.c
	$(C_DEBUG) -c -o build/serialization.o src/C/client/serialization.c
	$(C_DEBUG) -c -o build/message.o src/C/client/message.c
	$(C_DEBUG) -c -o build/client.o src/C/client/client.c
	$(C_DEBUG) -c -o build/message_debugger.o src/C/client/debug/message_debugger.c
	$(C_DEBUG) -c -o build/shape_topic.o test/prototype/shape_topic.c
	$(C_DEBUG) -c -o build/$@.o test/prototype/client/$@.c
	$(C_DEBUG) -o    build/$@ build/$@.o build/message_debugger.o build/shape_topic.o build/mini_cdr.o build/dynamic_buffer.o build/serialization.o build/message.o build/client.o

prototype_agent_test:
	$(C_DEBUG) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(C_DEBUG) -c -o build/dynamic_buffer.o src/C/client/dynamic_buffer.c
	$(C_DEBUG) -c -o build/serialization.o src/C/client/serialization.c
	$(C_DEBUG) -c -o build/message.o src/C/client/message.c
	$(C_DEBUG) -c -o build/message_debugger.o src/C/client/debug/message_debugger.c
	$(C_DEBUG) -c -o build/shape_topic.o test/prototype/shape_topic.c
	$(C_DEBUG) -c -o build/$@.o test/prototype/agent/$@.c
	$(C_DEBUG) -o    build/$@ build/$@.o build/message_debugger.o build/shape_topic.o build/mini_cdr.o build/dynamic_buffer.o build/serialization.o build/message.o

message_test:
	$(C_DEBUG) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(C_DEBUG) -c -o build/dynamic_buffer.o src/C/client/dynamic_buffer.c
	$(C_DEBUG) -c -o build/serialization.o src/C/client/serialization.c
	$(C_DEBUG) -c -o build/message.o src/C/client/message.c
	$(C_DEBUG) -c -o build/message_debugger.o src/C/client/debug/message_debugger.c
	$(C_DEBUG) -c -o build/$@.o test/$@.c
	$(C_DEBUG) -o    build/$@ build/$@.o build/message_debugger.o build/mini_cdr.o build/dynamic_buffer.o build/serialization.o build/message.o

mini_cdr_test:
	$(C_DEBUG) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(C_DEBUG) -c -o build/$@.o test/$@.c
	$(C_DEBUG) -o    build/$@ build/$@.o build/mini_cdr.o

mini_micro_cdr_performance_comparative_test:
	$(C_FAST) -c -o build/mini_cdr.o src/C/client/mini_cdr.c
	$(C_FAST) -c -o build/$@.o test/$@.c
	$(C_FAST) -o    build/$@ build/$@.o build/mini_cdr.o -lmicrocdr
