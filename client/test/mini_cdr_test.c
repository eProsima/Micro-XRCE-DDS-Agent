#include "mini_cdr.h"

#include <stdio.h>

#define BUFFER_SIZE 200

// Test example
int main(int args, char** argv)
{
    uint8_t data[BUFFER_SIZE];
    SerializedBufferHandle buffer_handle;

    init_serialized_buffer(&buffer_handle, data, BUFFER_SIZE);

    printf("SERIALIZATION: \n");


    serialize_byte(&buffer_handle, 0xAB);


    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    serialize_byte_2(&buffer_handle, 0xCDEF);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    serialize_byte_2(&buffer_handle, 0xCDEF);

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    serialize_byte_4(&buffer_handle, 0x12345678);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    serialize_byte_4(&buffer_handle, 0x12345678);

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    serialize_byte_8(&buffer_handle, 0x1234567890ABCDEF);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    serialize_byte_8(&buffer_handle, 0x1234567890ABCDEF);

    uint32_t sequence = 0x0000ABCD;

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    serialize_block(&buffer_handle, (uint8_t*)&sequence, 4);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    serialize_block(&buffer_handle, (uint8_t*)&sequence, 4);

    uint8_t array[] = {0x12, 0x34, 0x56, 0x78};
    serialize_array(&buffer_handle, array, 4);

    for(uint8_t* i = buffer_handle.data; i < buffer_handle.iterator; i++)
        printf("%02X ", *i);

    printf("\n\n");


    reset_buffer_iterator(&buffer_handle);
    printf("DESERIALIZATION: \n");

    uint8_t output_byte;
    uint16_t output_byte_2;
    uint32_t output_byte_4;
    uint64_t output_byte_8;
    uint8_t output_array[4];
    uint32_t output_sequence;


    deserialize_byte(&buffer_handle, &output_byte);
    printf("%02X ", output_byte);

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    deserialize_byte_2(&buffer_handle, &output_byte_2);
    printf("%04X ", output_byte_2);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    deserialize_byte_2(&buffer_handle, &output_byte_2);
    printf(" %04X ", output_byte_2);

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    deserialize_byte_4(&buffer_handle, &output_byte_4);
    printf("   %08X ", output_byte_4);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    deserialize_byte_4(&buffer_handle, &output_byte_4);
    printf("   %08X ", output_byte_4);

    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    deserialize_byte_8(&buffer_handle, &output_byte_8);
    printf("   %016lX ", output_byte_8);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    deserialize_byte_8(&buffer_handle, &output_byte_8);
    printf("   %016lX ", output_byte_8);


    buffer_handle.endian_mode = BIG_ENDIAN_MODE;
    deserialize_block(&buffer_handle, (uint8_t*)&output_sequence, 4);
    printf("%08X ", output_sequence);

    buffer_handle.endian_mode = LITTLE_ENDIAN_MODE;
    deserialize_block(&buffer_handle, (uint8_t*)&output_sequence, 4);
    printf("   %08X ", output_sequence);

    deserialize_array(&buffer_handle, output_array, 4);
    printf("%02X %02X %02X %02X ", output_array[0], output_array[1], output_array[2], output_array[3]);

    printf("\n");

    return 0;
}

/*
// ---------------------------------------------------------
//                  MINIMAL EXAMPLE: Same handler
// ---------------------------------------------------------
#define BUFFER_SIZE 200

// Test example
int main(int args, char** argv)
{
    uint8_t data[BUFFER_SIZE];

    SerializedBufferHandle handle;

    init_serialized_buffer(&handle, data, BUFFER_SIZE);

    uint8_t in_byte = 0xFF;
    serialize_byte(&buffer_handle, in_byte);

    reset_buffer_iterator(&buffer_handle);

    uint8_t out_byte;
    deserialize_byte(&buffer_handle, &out_byte);

    return 0;
}

// ---------------------------------------------------------
//                  MINIMAL EXAMPLE: 2 Handlesr
// ---------------------------------------------------------
#define BUFFER_SIZE 200

// Test example
int main(int args, char** argv)
{
    uint8_t data[BUFFER_SIZE];

    SerializedBufferHandle writer;
    SerializedBufferHandle reader;

    init_serialized_buffer(&writer, data, BUFFER_SIZE);
    init_serialized_buffer(&reader, data, BUFFER_SIZE);

    uint8_t in_byte = 0xFF;
    serialize_byte(&buffer_handle, in_byte);

    uint8_t out_byte;
    deserialize_byte(&buffer_handle, &out_byte);

    return 0;
}

*/