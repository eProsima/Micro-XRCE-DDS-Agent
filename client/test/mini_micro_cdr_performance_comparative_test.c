#include "mini_cdr.h"
#include "microcdr/microCdr.h"

#include <stdio.h>
#include <time.h>

/*
    Diferencias de implementacion:

                     mini                                  micro
    no comprueba si se sale de rango     |  comprueba si se sale de rango
    no comprueba alineaciones            |  comprueba alineaciones incluso en char
    no usa malloc nunca                  |  usa malloc en los arrays
    no usa for para serlializar en tipos |  usa for en los tipos de tamaño cte
*/

#define BUFFER_SIZE 40000

void test(int endian);
int main(int args, char** argv)
{
    test(LITTLE_ENDIAN_MODE);
    test(BIG_ENDIAN_MODE);

    return 0;
}

void test(int endian)
{
    //init variables
    clock_t t1, t2;
    uint8_t data[BUFFER_SIZE];
    SerializedBufferHandle buffer_handle;

    struct microBuffer * microBufferWriter = NULL;
    struct microBuffer * microBufferReader = NULL;
    struct microCDR * microCDRWriter = NULL;
    struct microCDR * microCDRReader = NULL;

    uint8_t byte;
    uint32_t byte_4;
    uint64_t byte_8;
    uint8_t array[100];
    uint8_t* micro_output_array;


    //init buffers
    init_serialized_buffer(&buffer_handle, data, BUFFER_SIZE);

    newStaticAlignedBuffer((char*)data, BUFFER_SIZE, &microBufferWriter);
    newDeserializedAlignedBuffer((char*)data, BUFFER_SIZE, &microBufferReader);

    newMicroCDR(&microCDRWriter, microBufferWriter);
    newMicroCDR(&microCDRReader, microBufferReader);

    //init test
    printf("Testing...\n\n");

    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j++)
        {
            serialize_byte(&buffer_handle, byte);
        }
        reset_buffer_iterator(&buffer_handle);

    }
    t2 = clock();
    float new_serialize_byte_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 100)
        {
            serialize_array(&buffer_handle, array, 100);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_serialize_array_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 4)
        {
            serialize_byte_4_endian(&buffer_handle, byte_4, endian);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_serialize_byte_4_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 8)
        {
            serialize_byte_8_endian(&buffer_handle, byte_8, endian);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_serialize_byte_8_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j++)
        {
            deserialize_byte(&buffer_handle, &byte);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_deserialize_byte_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 100)
        {
            deserialize_array(&buffer_handle, array, 100);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_deserialize_array_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 4)
        {
            deserialize_byte_4_endian(&buffer_handle, &byte_4, endian);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_deserialize_byte_4_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 8)
        {
            deserialize_byte_8_endian(&buffer_handle, &byte_8, endian);
        }
        reset_buffer_iterator(&buffer_handle);
    }
    t2 = clock();
    float new_deserialize_byte_8_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j++)
        {
            serializeUnsignedChar(byte, microCDRWriter);
        }
        resetStaticMicroCDRForSerialize(microCDRWriter);
    }
    t2 = clock();
    float micro_serialize_byte_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 100)
        {
            serializeUnsignedCharArray(array, 100, microCDRWriter);
        }
        resetStaticMicroCDRForSerialize(microCDRWriter);
    }
    t2 = clock();
    float micro_serialize_array_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 4)
        {
            serializeUnsignedIntEndianness(byte_4, endian, microCDRWriter);
        }
        resetStaticMicroCDRForSerialize(microCDRWriter);
    }
    t2 = clock();
    float micro_serialize_byte_4_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 8)
        {
            serializeUnsignedLongEndianness(byte_8, endian, microCDRWriter);
        }
        resetStaticMicroCDRForSerialize(microCDRWriter);
    }
    t2 = clock();
    float micro_serialize_byte_8_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j++)
        {
            deserializeUnsignedChar(&byte, microCDRReader);
        }
        resetStaticMicroCDRForDeserialize(microCDRReader);
    }
    t2 = clock();
    float micro_deserialize_byte_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 100)
        {
            deserializeUnsignedCharArray(&micro_output_array, 100, microCDRReader);
        }
        resetStaticMicroCDRForDeserialize(microCDRReader);
    }
    t2 = clock();
    float micro_deserialize_array_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;



    t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 4)
        {
            deserializeUnsignedIntEndianness(&byte_4, endian, microCDRReader);
        }
        resetStaticMicroCDRForDeserialize(microCDRReader);
    }
    t2 = clock();
    float micro_deserialize_byte_4_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;

        t1 = clock();
    for(unsigned i = 0; i < 10000; i++)
    {
        for(unsigned j = 0; j < BUFFER_SIZE; j += 8)
        {
            deserializeUnsignedLongEndianness(&byte_8, endian, microCDRReader);
        }
        resetStaticMicroCDRForDeserialize(microCDRReader);
    }
    t2 = clock();
    float micro_deserialize_byte_8_time = ((float)(t2 - t1) / CLOCKS_PER_SEC) * 1000;


    if(endian)
    printf("               Test (LITTLE_ENDIAN)              |     micro         |        new        |      diff\n");
    else
    printf("                Test (BIG_ENDIAN)                |     micro         |        new        |      diff\n");
    printf("\n");
    printf("Serialize byte                                   |     % 8.2f      |     % 8.2f      |     % 3.0f%%\n", micro_serialize_byte_time, new_serialize_byte_time, 100 * micro_serialize_byte_time / new_serialize_byte_time);
    printf("Serialize int                                    |     % 8.2f      |     % 8.2f      | =>  % 3.0f%%\n", micro_serialize_byte_4_time, new_serialize_byte_4_time, 100 * micro_serialize_byte_4_time/ new_serialize_byte_4_time);
    printf("Serialize long                                   |     % 8.2f      |     % 8.2f      | =>  % 3.0f%%\n", micro_serialize_byte_8_time, new_serialize_byte_8_time, 100 * micro_serialize_byte_8_time/ new_serialize_byte_8_time);
    printf("Serialize array                                  |     % 8.2f      |     % 8.2f      |     % 3.0f%%\n", micro_serialize_array_time, new_serialize_array_time, 100 * micro_serialize_array_time / new_serialize_array_time);
    printf("Deserialize byte                                 |     % 8.2f      |     % 8.2f      |     % 3.0f%%\n", micro_deserialize_byte_time, new_deserialize_byte_time, 100 * micro_deserialize_byte_time / new_deserialize_byte_time);
    printf("Deserialize int                                  |     % 8.2f      |     % 8.2f      | =>  % 3.0f%%\n", micro_deserialize_byte_4_time, new_deserialize_byte_4_time, 100 * micro_deserialize_byte_4_time / new_deserialize_byte_4_time);
    printf("Deserialize long                                 |     % 8.2f      |     % 8.2f      | =>  % 3.0f%%\n", micro_deserialize_byte_8_time, new_deserialize_byte_8_time, 100 * micro_deserialize_byte_8_time / new_deserialize_byte_8_time);
    printf("Deserialize array                                |     % 8.2f      |     % 8.2f      |     % 3.0f%%\n", micro_deserialize_array_time, new_deserialize_array_time, 100 * micro_deserialize_array_time / new_deserialize_array_time);

}