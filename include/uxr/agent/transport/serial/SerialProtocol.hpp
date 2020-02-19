// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_SERIAL_PROTOCOL_HPP_
#define UXR_AGENT_TRANSPORT_SERIAL_PROTOCOL_HPP_

#include <uxr/agent/transport/TransportRc.hpp>

#include <cstdint>
#include <cstddef>
#include <functional>

namespace eprosima {
namespace uxr {

typedef const std::function<size_t (uint8_t*, size_t, TransportRc&)> WriteCallback;
typedef const std::function<size_t (uint8_t*, size_t, int, TransportRc&)> ReadCallback;

class SerialIO
{
    static constexpr uint16_t crc16_table[256] = {
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
    static constexpr uint8_t framing_begin_flag = 0x7E;
    static constexpr uint8_t framing_esc_flag = 0x7D;
    static constexpr uint8_t framing_xor_flag = 0x20;

    enum class InputState : uint8_t
    {
        UXR_SERIAL_UNINITIALIZED,
        UXR_SERIAL_READING_SRC_ADDR,
        UXR_SERIAL_READING_DST_ADDR,
        UXR_SERIAL_READING_LEN_LSB,
        UXR_SERIAL_READING_LEN_MSB,
        UXR_SERIAL_READING_PAYLOAD,
        UXR_SERIAL_READING_CRC_LSB,
        UXR_SERIAL_READING_CRC_MSB,

    };

public:
    SerialIO(
            uint8_t local_addr,
            WriteCallback write_callback,
            ReadCallback read_callback);

    size_t write_msg(
            const uint8_t* buf,
            size_t len,
            uint8_t remote_addr,
            TransportRc& transport_rc);

    size_t read_msg(
            uint8_t* buf,
            size_t len,
            uint8_t& remote_addr,
            int timeout,
            TransportRc& transport_rc);

private:
    static void update_crc(
            uint16_t& crc,
            const uint8_t data);

    bool get_next_octet(
            uint8_t& octet);

    bool add_next_octet(
            uint8_t octet);

private:
    InputState state_;
    uint8_t local_addr_;
    uint8_t read_buffer_[42];
    uint8_t read_buffer_head_;
    uint8_t read_buffer_tail_;
    uint8_t remote_addr_;
    uint16_t msg_len_;
    uint16_t msg_pos_;
    uint16_t msg_crc_;
    uint16_t cmp_crc_;
    uint8_t write_buffer_[42];
    uint8_t write_buffer_pos_;
    WriteCallback write_callback_;
    ReadCallback read_callback_;
};

inline SerialIO::SerialIO(
        uint8_t local_addr,
        WriteCallback write_callback,
        ReadCallback read_callback)
    : state_{InputState::UXR_SERIAL_UNINITIALIZED}
    , local_addr_{local_addr}
    , write_callback_{write_callback}
    , read_callback_{read_callback}
{}

inline void SerialIO::update_crc(
        uint16_t& crc,
        const uint8_t data)
{
    crc = (crc >> 8) ^ SerialIO::crc16_table[(crc ^ data) & 0xFF];
}

inline bool SerialIO::get_next_octet(
        uint8_t& octet)
{
    bool rv = false;
    octet = 0;
    if (read_buffer_head_ != read_buffer_tail_)
    {
        if (framing_esc_flag != read_buffer_[read_buffer_tail_])
        {
            octet = read_buffer_[read_buffer_tail_];
            read_buffer_tail_ = uint8_t(size_t(read_buffer_tail_ + 1) % sizeof(read_buffer_));
            rv = (framing_begin_flag != octet);
        }
        else
        {
            uint8_t temp_tail = uint8_t(size_t(read_buffer_tail_ + 1) % sizeof(read_buffer_));
            if (temp_tail != read_buffer_head_)
            {
                octet = read_buffer_[temp_tail];
                read_buffer_tail_ = uint8_t(size_t(read_buffer_tail_ + 2) % sizeof(read_buffer_));
                if (framing_begin_flag != octet)
                {
                    octet ^= framing_xor_flag;
                    rv = true;
                }
            }
        }
    }
    return rv;
}

inline bool SerialIO::add_next_octet(
        uint8_t octet)
{
    bool rv = false;

    if (framing_begin_flag == octet || framing_esc_flag == octet)
    {
        if (uint8_t(write_buffer_pos_ + 1) < sizeof(write_buffer_))
        {
            write_buffer_[write_buffer_pos_] = framing_esc_flag;
            write_buffer_[write_buffer_pos_ + 1] = octet ^ framing_xor_flag;
            write_buffer_pos_ += 2;
            rv = true;
        }
    }
    else
    {
        if (write_buffer_pos_ < sizeof(write_buffer_))
        {
            write_buffer_[write_buffer_pos_] = octet;
            ++write_buffer_pos_;
            rv = true;
        }
    }

    return rv;
}

inline size_t SerialIO::write_msg(
        const uint8_t *buf,
        size_t len,
        uint8_t remote_addr,
        TransportRc& transport_rc)
{
    /* Buffer being flag. */
    write_buffer_[0] = framing_begin_flag;
    write_buffer_pos_ = 1;

    /* Buffer header. */
    add_next_octet(local_addr_);
    add_next_octet(remote_addr);
    add_next_octet(uint8_t(len & 0xFF));
    add_next_octet(uint8_t(len >> 8));

    /* Write payload. */
    uint8_t octet = 0;
    uint16_t written_len = 0;
    uint16_t crc = 0;
    bool cond = true;
    while (written_len < len && cond)
    {
        octet = *(buf + written_len);
        if (add_next_octet(octet))
        {
            update_crc(crc, octet);
            ++written_len;
        }
        else
        {
            size_t bytes_written = write_callback_(write_buffer_, write_buffer_pos_, transport_rc);
            if (0 < bytes_written)
            {
                cond = true;
                write_buffer_pos_ = uint8_t(write_buffer_pos_ - bytes_written);
            }
            else
            {
                cond = false;
            }
        }
    }

    /* Write CRC. */
    uint8_t tmp_crc[2];
    tmp_crc[0] = uint8_t(crc & 0xFF);
    tmp_crc[1] = uint8_t(crc >> 8);
    written_len = 0;
    while (written_len < sizeof(tmp_crc) && cond)
    {
        octet = *(tmp_crc + written_len);
        if (add_next_octet(octet))
        {
            update_crc(crc, octet);
            ++written_len;
        }
        else
        {
            size_t bytes_written = write_callback_(write_buffer_, write_buffer_pos_, transport_rc);
            if (0 < bytes_written)
            {
                cond = true;
                write_buffer_pos_ -= (uint8_t)bytes_written;
            }
            else
            {
                cond = false;
            }
        }
    }

    /* Flus write buffer. */
    if (cond && (0 < write_buffer_pos_))
    {
            size_t bytes_written = write_callback_(write_buffer_, write_buffer_pos_, transport_rc);
            if (0 < bytes_written)
            {
                cond = true;
                write_buffer_pos_ -= (uint8_t)bytes_written;
            }
            else
            {
                cond = false;
            }
    }

    return cond ? uint16_t(len) : 0;
}

inline size_t SerialIO::read_msg(
        uint8_t* buf,
        size_t len,
        uint8_t& remote_addr,
        int timeout,
        TransportRc& transport_rc)
{
    size_t rv = 0;

    /* Compute read-buffer available size. */
    uint8_t av_len[2] = {0, 0};
    if (read_buffer_head_ == read_buffer_tail_)
    {
        read_buffer_head_ = 0;
        read_buffer_tail_ = 0;
        av_len[0] = sizeof(read_buffer_) - 1;
    }
    else if (read_buffer_head_ > read_buffer_tail_)
    {
        if (0 < read_buffer_tail_)
        {
            av_len[0] = uint8_t(sizeof(read_buffer_) - read_buffer_head_);
            av_len[1] = uint8_t(read_buffer_tail_ - 1);
        }
        else
        {
            av_len[0] = uint8_t(sizeof(read_buffer_) - read_buffer_head_ - 1);
        }
    }
    else
    {
        av_len[0] = uint8_t(read_buffer_tail_ - read_buffer_head_ - 1);
    }

    /* Read from serial. */
    size_t bytes_read[2] = {0};
    if (0 < av_len[0])
    {
        bytes_read[0] = read_callback_(&read_buffer_[read_buffer_head_], av_len[0], timeout, transport_rc);
        read_buffer_head_ = uint8_t(size_t(read_buffer_head_ + bytes_read[0]) % sizeof(read_buffer_));
        if (0 < bytes_read[0])
        {
            if ((bytes_read[0] == av_len[0]) && (0 < av_len[1]))
            {
                bytes_read[1] = read_callback_(&read_buffer_[read_buffer_head_], av_len[1], 0, transport_rc);
                read_buffer_head_ = uint8_t(size_t(read_buffer_head_ + bytes_read[1]) % sizeof(read_buffer_));
            }
        }
    }

    if (0 < (bytes_read[0] + bytes_read[1]) || (read_buffer_tail_ != read_buffer_head_))
    {
        /* State Machine. */
        bool exit_cond = false;
        while (!exit_cond)
        {
            uint8_t octet = 0;
            switch (state_)
            {
                case InputState::UXR_SERIAL_UNINITIALIZED:
                {
                    octet = 0;
                    while ((framing_begin_flag != octet) && (read_buffer_head_ != read_buffer_tail_))
                    {
                        octet = read_buffer_[read_buffer_tail_];
                        read_buffer_tail_ = uint8_t(size_t(read_buffer_tail_ + 1) % sizeof(read_buffer_));
                    }

                    if (framing_begin_flag == octet)
                    {
                        state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                    }
                    else
                    {
                        exit_cond = true;
                    }
                    break;
                }
                case InputState::UXR_SERIAL_READING_SRC_ADDR:
                {
                    if (get_next_octet(remote_addr_))
                    {
                        state_ = InputState::UXR_SERIAL_READING_DST_ADDR;
                    }
                    else
                    {
                        if (framing_begin_flag != remote_addr_)
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_SERIAL_READING_DST_ADDR:
                    if (get_next_octet(octet))
                    {
                        state_ = (octet == local_addr_)
                                ? InputState::UXR_SERIAL_READING_LEN_LSB
                                : InputState::UXR_SERIAL_UNINITIALIZED;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                case InputState::UXR_SERIAL_READING_LEN_LSB:
                    if (get_next_octet(octet))
                    {
                        msg_len_ = octet;
                        state_ = InputState::UXR_SERIAL_READING_LEN_MSB;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                case InputState::UXR_SERIAL_READING_LEN_MSB:
                    if (get_next_octet(octet))
                    {
                        msg_len_ += (octet << 8);
                        msg_pos_ = 0;
                        cmp_crc_ = 0;
                        if (len < msg_len_)
                        {
                            state_ = InputState::UXR_SERIAL_UNINITIALIZED;
                            exit_cond = true;
                        }
                        else
                        {
                            state_ = InputState::UXR_SERIAL_READING_PAYLOAD;
                        }
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                case InputState::UXR_SERIAL_READING_PAYLOAD:
                {
                    while ((msg_pos_ < msg_len_) && get_next_octet(octet))
                    {
                        buf[size_t(msg_pos_)] = octet;
                        ++msg_pos_;
                        update_crc(cmp_crc_, octet);
                    }

                    if (msg_pos_ == msg_len_)
                    {
                        state_ = InputState::UXR_SERIAL_READING_CRC_LSB;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_SERIAL_READING_CRC_LSB:
                    if (get_next_octet(octet))
                    {
                        msg_crc_ = octet;
                        state_ = InputState::UXR_SERIAL_READING_CRC_MSB;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                case InputState::UXR_SERIAL_READING_CRC_MSB:
                    if (get_next_octet(octet))
                    {
                        msg_crc_ += (octet << 8);
                        state_ = InputState::UXR_SERIAL_UNINITIALIZED;
                        if (cmp_crc_ == msg_crc_)
                        {
                            remote_addr = remote_addr_;
                            rv = msg_len_;
                        }
                        exit_cond = true;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_SERIAL_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
            }
        }
    }

    return rv;
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_SERIAL_PROTOCOL_HPP_
