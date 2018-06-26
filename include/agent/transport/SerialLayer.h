// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _MICRORTPS_AGENT_TRANSPORT_SERIAL_LAYER_H_
#define _MICRORTPS_AGENT_TRANSPORT_SERIAL_LAYER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MICRORTPS_FRAMING_END_FLAG 0x7E
#define MICRORTPS_FRAMING_ESC_FLAG 0x7D
#define MICRORTPS_FRAMING_XOR_FLAG 0x20

#define MICRORTPS_SERIAL_MTU 255
#define MICRORTPS_SERIAL_OVERHEAD 5
#define MICRORTPS_SERIAL_BUFFER_SIZE 2 * (MICRORTPS_SERIAL_MTU + MICRORTPS_SERIAL_OVERHEAD)

typedef enum InputBufferState
{
    SERIAL_BUFFER_EMPTY,
    SERIAL_DATA_AVAILABLE,
    SERIAL_MESSAGE_INIT,
    SERIAL_MESSAGE_INCOMPLETE,
    SERIAL_MESSAGE_AVAILABLE

} InputBufferState;

typedef struct SerialInputBuffer SerialInputBuffer;
struct SerialInputBuffer
{
    uint8_t buffer[MICRORTPS_SERIAL_BUFFER_SIZE];
    uint16_t head;
    uint16_t marker;
    uint16_t tail;
    InputBufferState state;
};

typedef struct SerialOutputBuffer SerialOutputBuffer;
struct SerialOutputBuffer
{
    uint8_t buffer[MICRORTPS_SERIAL_BUFFER_SIZE];
};

typedef struct SerialIO SerialIO;
struct SerialIO
{
    SerialInputBuffer input;
    SerialOutputBuffer output;
};

typedef uint16_t (*read_cb)(void*, uint8_t*, size_t, int);

void init_serial_io(SerialIO* serial_io);
uint16_t write_serial_msg(SerialIO* serial_io, const uint8_t* buf, size_t len, uint8_t addr);
uint8_t read_serial_msg(SerialIO* serial_io, read_cb cb, void* cb_arg,
                        uint8_t* buf, size_t len, uint8_t* addr, int timeout);

#ifdef __cplusplus
}
#endif

#endif //_MICRORTPS_AGENT_TRANSPORT_SERIAL_LAYER_H_
