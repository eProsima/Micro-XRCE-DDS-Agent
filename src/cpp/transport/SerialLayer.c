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

#include <micrortps/agent/transport/SerialLayer.h>
#include <string.h>

/*******************************************************************************
 * Static members.
 *******************************************************************************/
// CRC-16 table for POLY 0x8005 (x^16 + x^15 + x^2 + 1).
static const uint16_t crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

/*******************************************************************************
 * Private function declarations.
 *******************************************************************************/
static void update_crc(uint16_t* crc, const uint8_t data);
static uint8_t process_serial_message(SerialInputBuffer* input, uint8_t* buf, size_t len, uint8_t* addr);
static bool init_serial_stream(SerialInputBuffer* input);
static bool find_serial_message(SerialInputBuffer* input);
static inline uint8_t get_next_octet(SerialInputBuffer* input, uint16_t* relative_position);
static inline bool add_next_octet(SerialOutputBuffer* output, uint8_t octet, uint16_t* position);

/*******************************************************************************
 * Private function definitions.
 *******************************************************************************/
void update_crc(uint16_t* crc, const uint8_t data)
{
    *crc = (*crc >> 8) ^ crc16_table[(*crc ^ data) & 0xFF];
}

uint8_t process_serial_message(SerialInputBuffer* input, uint8_t* buf, size_t len, uint8_t* addr)
{
    bool cond = true;
    uint16_t msg_len = 0;
    uint16_t msg_marker = 0;
    uint8_t payload_len = 0;
    uint8_t payload_marker = 0;
    uint16_t crc = 0;

    /* Check raw message length. */
    msg_len = (input->head < input->marker) ? (input->marker - input->head) :
                                              (MICRORTPS_SERIAL_BUFFER_SIZE - input->marker + input->head);
    if (MICRORTPS_SERIAL_OVERHEAD > msg_len)
    {
        cond = false;
    }
    else
    {
        /* Read address and payload length. */
        *addr = get_next_octet(input, &msg_marker); // TODO (julian): implement simple routing method.
        payload_len = get_next_octet(input, &msg_marker);

        /* Check message length. */
        if (payload_len > len || payload_len > msg_len - MICRORTPS_SERIAL_OVERHEAD)
        {
            cond = false;
        }
        else
        {
            /* Read payload. */
            uint8_t octet;
            while (payload_marker < payload_len && cond)
            {
                octet = get_next_octet(input, &msg_marker);
                update_crc(&crc, octet);
                *(buf + payload_marker) = octet;
                ++payload_marker;
                cond = (msg_marker < msg_len - 2);
            }

            if (cond)
            {
                /* Check CRC. */
                uint16_t msg_crc;
                octet = get_next_octet(input, &msg_marker);
                msg_crc = (uint16_t)octet << 8;
                octet = get_next_octet(input, &msg_marker);
                msg_crc = msg_crc | (uint16_t)octet;

                if (msg_crc != crc)
                {
                    cond = false;
                }
            }
        }
    }
    input->head = input->marker;

    return cond ? payload_len : 0;
}

bool init_serial_stream(SerialInputBuffer* input)
{
    bool rv = true;

    /* Find BEGIN_FLAG. */
    while (MICRORTPS_FRAMING_END_FLAG != input->buffer[input->head])
    {
        input->head = (input->head + 1) % sizeof(input->buffer);
        if (input->head == input->tail)
        {
            rv = false;
            break;
        }
    }

    /* If BEGIN_FLAG was found, move ahead. */
    if (rv)
    {
        input->head = (input->head + 1) % sizeof(input->buffer);
        input->marker = input->head;
    }

    return rv;
}

bool find_serial_message(SerialInputBuffer* input)
{
    bool rv = true;

    /* Find END_FLAG. */
    while (MICRORTPS_FRAMING_END_FLAG != input->buffer[input->marker])
    {
        input->marker = (input->marker + 1) % sizeof(input->buffer);
        if (input->marker == input->tail)
        {
            rv = false;
            break;
        }
    }

    /* If END_FLAG was found, move ahead. */
    if (rv)
    {
        input->marker = (input->marker + 1) % sizeof(input->buffer);
    }

    return rv;
}

uint8_t get_next_octet(SerialInputBuffer* input, uint16_t* relative_position)
{
    uint8_t rv = input->buffer[(input->head + *relative_position) % sizeof(input->buffer)];

    *relative_position += 1;
    if (MICRORTPS_FRAMING_END_FLAG == rv || MICRORTPS_FRAMING_ESC_FLAG == rv)
    {
        rv = input->buffer[(input->head + *relative_position) % sizeof(input->buffer)] ^ MICRORTPS_FRAMING_XOR_FLAG;
        *relative_position += 1;
    }

    return rv;
}

bool add_next_octet(SerialOutputBuffer* output, uint8_t octet, uint16_t* position)
{
    bool rv = false;

    if (MICRORTPS_FRAMING_END_FLAG == octet || MICRORTPS_FRAMING_ESC_FLAG == octet)
    {
        if (*position < sizeof(output->buffer))
        {
            output->buffer[*position] = MICRORTPS_FRAMING_ESC_FLAG;
            output->buffer[*position + 1] = octet ^ MICRORTPS_FRAMING_XOR_FLAG;
            *position += 2;
            rv = true;
        }
    }
    else
    {
        if (*position <= sizeof(output->buffer))
        {
            output->buffer[*position] = octet;
            *position += 1;
            rv = true;
        }
    }

    return rv;
}

/*******************************************************************************
 * Public function definitions.
 *******************************************************************************/
void init_serial_io(SerialIO* serial_io)
{
    serial_io->input.stream_init = false;
}

uint16_t write_serial_msg(SerialIO* serial_io, const uint8_t* buf, size_t len, uint8_t addr)
{
    bool cond = true;
    uint16_t crc = 0;
    uint16_t position = 0;
    uint16_t writing_len = 0;

    /* Check output buffer size. */
    if (MICRORTPS_SERIAL_MTU < len || sizeof(serial_io->output.buffer) - MICRORTPS_SERIAL_OVERHEAD < len)
    {
        cond = false;
    }

    if (cond)
    {
        /* Write header. */
        cond = add_next_octet(&serial_io->output, addr, &position) &&
               add_next_octet(&serial_io->output, (uint8_t)len, &position);

        /* Write payload. */
        uint8_t octet = 0;
        while (writing_len < len && cond)
        {
            octet = *(buf + writing_len);
            cond &= add_next_octet(&serial_io->output, octet, &position);
            update_crc(&crc, octet);
            ++writing_len;
        }

        /* Write CRC and end flag. */
        if (cond)
        {
            cond = add_next_octet(&serial_io->output, (uint8_t)(crc >> 8), &position) &&
                   add_next_octet(&serial_io->output, (uint8_t)(crc & 0xFF), &position);

            /* Write end flag. */
            if (cond)
            {
                serial_io->output.buffer[position] = MICRORTPS_FRAMING_END_FLAG;
            }
        }
    }

    return cond ? (position + 1) : 0;
}

uint8_t read_serial_msg(SerialIO* serial_io, read_cb cb, void* cb_arg,
                        uint8_t* buf, size_t len, uint8_t* addr, int timeout)
{
    uint8_t rv = 0;
    bool data_available = false;

    if (!serial_io->input.stream_init)
    {
        /* Init buffer. */
        serial_io->input.head = 0;
        serial_io->input.marker = 0;
        serial_io->input.tail = 0;

        /* Read from callback. */
        uint16_t bytes_read = cb(cb_arg, serial_io->input.buffer, sizeof(serial_io->input.buffer), timeout);
        serial_io->input.tail = (serial_io->input.tail + bytes_read) % sizeof(serial_io->input.buffer);
        if (0 < bytes_read)
        {
            /* Init stream. */
            if (init_serial_stream(&serial_io->input))
            {
                serial_io->input.stream_init = true;
                /* Check if there is data available. */
                data_available = serial_io->input.head != serial_io->input.tail;
            }
        }
    }
    else
    {
        /* Get available buffer size. */
        size_t av_len = 0;
        if (serial_io->input.head <= serial_io->input.tail)
        {
            av_len = sizeof(serial_io->input.buffer) - serial_io->input.tail;
        }
        else
        {
            av_len = serial_io->input.head - serial_io->input.tail;
        }

        /* Read data from callback and update tail. */
        uint16_t bytes_read = cb(cb_arg, &serial_io->input.buffer[serial_io->input.tail], av_len, timeout);
        serial_io->input.tail = (serial_io->input.tail + bytes_read) % sizeof(serial_io->input.buffer);

        /* Check if data available. */
        data_available = (0 < bytes_read || serial_io->input.head != serial_io->input.tail);
    }

    /* Process data in case. */
    if (data_available)
    {
        /* Look for available messages. */
        bool message_found = find_serial_message(&serial_io->input);
        while (message_found)
        {
            /* Process available message from head to marker and update head. */
            rv = process_serial_message(&serial_io->input, buf, len, addr);
            if (0 < rv)
            {
                break;
            }
            message_found = find_serial_message(&serial_io->input);
        }

        if (serial_io->input.marker == serial_io->input.tail)
        {
            if (0 == rv)
            {
                /* Buffer full, reset stream. */
                serial_io->input.stream_init = false;
            }
            else
            {
                /* Reset buffer to gain continuous free space. */
                serial_io->input.head = 0;
                serial_io->input.marker = 0;
                serial_io->input.tail = 0;
            }
        }
    }

    return rv;
}
