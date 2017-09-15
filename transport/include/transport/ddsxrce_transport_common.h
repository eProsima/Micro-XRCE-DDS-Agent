// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _DDSXRCE_TRANSPORT_COMMON_H_
#define _DDSXRCE_TRANSPORT_COMMON_H_

#include <poll.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif



#define TRANSPORT_ERROR              -1
#define TRANSPORT_OK                  0

typedef unsigned char octet;

typedef struct __attribute__((packed)) Header
{
    char marker[3];
    octet payload_len_h;
    octet payload_len_l;
    octet crc_h;
    octet crc_l;
} header_t;

typedef enum Kind
{
    LOC_NONE,
    LOC_SERIAL,

} locator_kind_t;

typedef struct __attribute__((packed)) Locator
{
    locator_kind_t kind;
    octet data[16];
} locator_t;

typedef int32_t channel_id_t;

/// SERIAL TRANSPORT

#define DFLT_UART             "/dev/ttyACM0"
#define DFLT_BAUDRATE            115200
#define DFLT_POLL_MS                 20
#define RX_BUFFER_LENGTH           1024
#define UART_NAME_MAX_LENGTH         64
#define MAX_NUM_SERIAL_CHANNELS       8


typedef struct
{
    char uart_name[UART_NAME_MAX_LENGTH];
    int uart_fd;
    octet rx_buffer[RX_BUFFER_LENGTH];
    uint32_t rx_buff_pos;
    uint32_t baudrate;
    uint32_t poll_ms;
    uint8_t id;
    bool open;

} serial_channel_t;

uint16_t crc16_byte(uint16_t crc, const uint8_t data);
uint16_t crc16(uint8_t const *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif
