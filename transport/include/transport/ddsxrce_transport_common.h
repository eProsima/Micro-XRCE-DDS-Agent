/****************************************************************************
 *
 * Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

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
