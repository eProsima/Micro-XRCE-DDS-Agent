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

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MICRORTPS_FRAMING_END_FLAG 0x7E
#define MICRORTPS_FRAMING_ESC_FLAG 0x7D
#define MICRORTPS_FRAMING_XOR_FLAG 0x20

#define MICRORTPS_SERIAL_MTU 256
#define MICRORTPS_SERIAL_OVERHEAD 5
#define MICRORTPS_SERIAL_BUFFER_SIZE 2 * (MICRORTPS_SERIAL_MTU + MICRORTPS_SERIAL_OVERHEAD)

namespace eprosima {
namespace micrortps {

typedef struct SerialInputBuffer SerialInputBuffer;
struct SerialInputBuffer
{
    uint8_t buffer[MICRORTPS_SERIAL_BUFFER_SIZE];
    uint16_t head;
    uint16_t marker;
    uint16_t tail;
    bool stream_init;
};

typedef struct SerialOutputBuffer SerialOutputBuffer;
struct SerialOutputBuffer
{
    uint8_t buffer[MICRORTPS_SERIAL_BUFFER_SIZE];
};

typedef uint16_t (*read_cb)(void*, uint8_t*, size_t, int);

class SerialIO
{
public:
    SerialIO() = default;

    void init();
    uint16_t write_serial_msg(const uint8_t* buf, size_t len, const uint8_t src_addr, const uint8_t rmt_addr);
    uint16_t read_serial_msg(read_cb cb,
                             void* cb_arg,
                             uint8_t* buf,
                             size_t len,
                             uint8_t* src_addr,
                             uint8_t* rmt_addr,
                             int timeout);
    uint8_t* get_output_buffer() { return output_buffer_.buffer; }


private:
    static void update_crc(uint16_t* crc, const uint8_t data);
    uint8_t process_serial_message(uint8_t* buf, size_t len, uint8_t* src_addr, uint8_t* rmt_addr);
    bool init_serial_stream();
    bool find_serial_message();
    uint8_t get_next_octet(uint16_t* relative_position);
    bool add_next_octet(uint8_t octet, uint16_t* position);

private:
    SerialInputBuffer input_buffer_;
    SerialOutputBuffer output_buffer_;
    static const uint16_t crc16_table_[256];
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_SERIAL_LAYER_H_
