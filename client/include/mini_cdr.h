#ifndef _DDS_XRCE_MINI_CDR_H_
#define _DDS_XRCE_MINI_CDR_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <string.h>

#define BIG_ENDIAN_MODE 0
#define LITTLE_ENDIAN_MODE 1

typedef struct SerializedBufferHandle
{
    uint8_t* data;
    uint32_t size;
    uint8_t* iterator;
    uint8_t endian_machine;
    uint8_t endian_mode;

} SerializedBufferHandle;

void init_serialized_buffer(SerializedBufferHandle* buffer, uint8_t* buffer_input, uint32_t size);
void reset_buffer_iterator(SerializedBufferHandle* buffer);

uint32_t get_free_space(SerializedBufferHandle* buffer);

void serialize_byte(SerializedBufferHandle* buffer, uint8_t byte);
void serialize_byte_2(SerializedBufferHandle* buffer, uint16_t bytes);
void serialize_byte_4(SerializedBufferHandle* buffer, uint32_t bytes);
void serialize_byte_8(SerializedBufferHandle* buffer, uint64_t bytes);
void serialize_block(SerializedBufferHandle* buffer, const uint8_t* block, uint32_t size);
void serialize_array(SerializedBufferHandle* buffer, const uint8_t* array, uint32_t size);

void deserialize_byte(SerializedBufferHandle* buffer, uint8_t* byte);
void deserialize_byte_2(SerializedBufferHandle* buffer, uint16_t* bytes);
void deserialize_byte_4(SerializedBufferHandle* buffer, uint32_t* bytes);
void deserialize_byte_8(SerializedBufferHandle* buffer, uint64_t* bytes);
void deserialize_block(SerializedBufferHandle* buffer, uint8_t* block, uint32_t size);
void deserialize_array(SerializedBufferHandle* buffer, uint8_t* array, uint32_t size);

void align_to(SerializedBufferHandle* buffer, uint32_t alignment);

//TODO:
// - (future work) deserialize since a character found

#ifdef __cplusplus
}
#endif

#endif //_DDS_XRCE_MINI_CDR_H_