// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/transport/stream_framing/StreamFramingProtocol.hpp>
#include <chrono>

namespace eprosima {
namespace uxr {

constexpr uint16_t FramingIO::crc16_table[256];

FramingIO::FramingIO(
        uint8_t local_addr,
        WriteCallback write_callback,
        ReadCallback read_callback)
    : state_(InputState::UXR_FRAMING_UNINITIALIZED)
    , local_addr_(local_addr)
    , remote_addr_(0)
    , read_buffer_()
    , read_buffer_head_(0)
    , read_buffer_tail_(0)
    , read_callback_(read_callback)
    , msg_len_(0)
    , msg_pos_(0)
    , msg_crc_(0)
    , cmp_crc_(0)
    , write_buffer_()
    , write_buffer_pos_(0)
    , write_callback_(write_callback)
{
}

size_t FramingIO::write_framed_msg(
        const uint8_t* buf,
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
    add_next_octet(static_cast<uint8_t>(len & 0xFF));
    add_next_octet(static_cast<uint8_t>(len >> 8));

    /* Write payload. */
    uint8_t octet = 0;
    uint16_t written_len = 0;
    uint16_t crc = 0;
    bool cond = true;
    while (written_len < len && cond)
    {
        octet = static_cast<uint8_t>(*(buf + written_len));
        if (add_next_octet(octet))
        {
            update_crc(crc, octet);
            ++written_len;
        }
        else
        {
            cond = transport_write(transport_rc);
        }
    }

    /* Write CRC. */
    uint8_t tmp_crc[2];
    tmp_crc[0] = static_cast<uint8_t>(crc & 0xFF);
    tmp_crc[1] = static_cast<uint8_t>(crc >> 8);
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
            cond = transport_write(transport_rc);
        }
    }

    /* Flus write buffer. */
    if (cond && (0 < write_buffer_pos_))
    {
        cond = transport_write(transport_rc);
    }

    return cond ? len : 0;
}

size_t FramingIO::read_framed_msg(
        uint8_t* buf,
        size_t len,
        uint8_t& remote_addr,
        int& timeout,
        TransportRc& transport_rc)
{
    size_t rv = 0;

    if (read_buffer_tail_ == read_buffer_head_)
    {
        transport_read(timeout, transport_rc, 5);
    }

    if (read_buffer_tail_ != read_buffer_head_)
    {
        /**
         * State Machine.
         */
        bool exit_cond = false;
        while (!exit_cond)
        {
            uint8_t octet = 0;
            switch (state_)
            {
                case InputState::UXR_FRAMING_UNINITIALIZED:
                {
                    octet = 0;
                    while ((framing_begin_flag != octet) &&
                           (read_buffer_head_ != read_buffer_tail_))
                    {
                        octet = read_buffer_[read_buffer_tail_];
                        read_buffer_tail_ =
                            static_cast<uint8_t>(
                                static_cast<size_t>(
                                    read_buffer_tail_ + 1) %
                                    sizeof(read_buffer_));
                    }

                    if (framing_begin_flag == octet)
                    {
                        state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                    }
                    else
                    {
                        exit_cond = true;
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_SRC_ADDR:
                {
                    if (get_next_octet(remote_addr_))
                    {
                        state_ = InputState::UXR_FRAMING_READING_DST_ADDR;
                    }
                    else if(0 < transport_read(timeout, transport_rc, 4))
                    {

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
                case InputState::UXR_FRAMING_READING_DST_ADDR:
                {
                    if (get_next_octet(octet))
                    {
                        state_ = (octet == local_addr_)
                                ? InputState::UXR_FRAMING_READING_LEN_LSB
                                : InputState::UXR_FRAMING_UNINITIALIZED;
                    }
                    else if(0 < transport_read(timeout, transport_rc, 3))
                    {

                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_LEN_LSB:
                {
                    if (get_next_octet(octet))
                    {
                        msg_len_ = octet;
                        state_ = InputState::UXR_FRAMING_READING_LEN_MSB;
                    }
                    else if(0 < transport_read(timeout, transport_rc, 2))
                    {

                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_LEN_MSB:
                {
                    if (get_next_octet(octet))
                    {
                        msg_len_ += (octet << 8);
                        msg_pos_ = 0;
                        cmp_crc_ = 0;
                        if (len < msg_len_)
                        {
                            state_ = InputState::UXR_FRAMING_UNINITIALIZED;
                            exit_cond = true;
                        }
                        else
                        {
                            state_ = InputState::UXR_FRAMING_READING_PAYLOAD;
                        }
                    }
                    else if(0 < transport_read(timeout, transport_rc, 1))
                    {

                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_PAYLOAD:
                {
                    while ((msg_pos_ < msg_len_) && get_next_octet(octet))
                    {
                        buf[static_cast<size_t>(msg_pos_)] = octet;
                        ++msg_pos_;
                        update_crc(cmp_crc_, octet);
                    }

                    if (msg_pos_ == msg_len_)
                    {
                        state_ = InputState::UXR_FRAMING_READING_CRC_LSB;
                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                        }
                        else if (0 < transport_read(timeout, transport_rc, (msg_len_ - msg_pos_) + 2))
                        {
                            /* Do nothing */
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_CRC_LSB:
                {
                    if (get_next_octet(octet))
                    {
                        msg_crc_ = octet;
                        state_ = InputState::UXR_FRAMING_READING_CRC_MSB;
                    }
                    else if(0 < transport_read(timeout, transport_rc, 2))
                    {

                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
                        }
                        else
                        {
                            exit_cond = true;
                        }
                    }
                    break;
                }
                case InputState::UXR_FRAMING_READING_CRC_MSB:
                    if (get_next_octet(octet))
                    {
                        msg_crc_ += (octet << 8);
                        state_ = InputState::UXR_FRAMING_UNINITIALIZED;
                        if (cmp_crc_ == msg_crc_)
                        {
                            remote_addr = remote_addr_;
                            rv = msg_len_;
                        }
                        exit_cond = true;
                    }
                    else if(0 < transport_read(timeout, transport_rc, 1))
                    {

                    }
                    else
                    {
                        if (framing_begin_flag == octet)
                        {
                            state_ = InputState::UXR_FRAMING_READING_SRC_ADDR;
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

void FramingIO::update_crc(
        uint16_t& crc,
        const uint8_t data)
{
    crc = (crc >> 8) ^ crc16_table[(crc ^ data) & 0xFF];
}

bool FramingIO::get_next_octet(
        uint8_t& octet)
{
    bool rv = false;
    octet = 0;

    if (read_buffer_head_ != read_buffer_tail_)
    {
        if (framing_esc_flag != read_buffer_[read_buffer_tail_])
        {
            octet = read_buffer_[read_buffer_tail_];
            read_buffer_tail_ = static_cast<uint8_t>(
                static_cast<size_t>(read_buffer_tail_ + 1) % sizeof(read_buffer_));

            rv = (framing_begin_flag != octet);
        }
        else
        {
            uint8_t temp_tail = static_cast<uint8_t>(
                static_cast<size_t>(read_buffer_tail_ + 1) % sizeof(read_buffer_));

            if (temp_tail != read_buffer_head_)
            {
                octet = read_buffer_[temp_tail];
                read_buffer_tail_ = static_cast<uint8_t>(
                    static_cast<size_t>(read_buffer_tail_ + 2) % sizeof(read_buffer_));

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

bool FramingIO::add_next_octet(
        uint8_t octet)
{
    bool rv = false;

    if (framing_begin_flag == octet || framing_esc_flag == octet)
    {
        if (static_cast<uint8_t>(write_buffer_pos_ + 1) < sizeof(write_buffer_))
        {
            write_buffer_[write_buffer_pos_] = framing_esc_flag;
            write_buffer_[write_buffer_pos_ + 1] = octet ^ framing_xor_flag;
            write_buffer_pos_ += 2;
            rv = true;
        }
    }
    else if (write_buffer_pos_ < sizeof(write_buffer_))
    {
        write_buffer_[write_buffer_pos_] = octet;
        ++write_buffer_pos_;
        rv = true;
    }

    return rv;
}

bool FramingIO::transport_write(
        TransportRc& transport_rc)
{
    size_t bytes_written = 0;
    size_t last_written = 0;

    do
    {
        ssize_t write_res = write_callback_(write_buffer_, write_buffer_pos_, transport_rc);
        last_written = (0 < write_res) ? write_res : 0;
        bytes_written += last_written;
    } while (bytes_written < write_buffer_pos_ && 0 < last_written);

    if (write_buffer_pos_ == bytes_written)
    {
        write_buffer_pos_ = 0;
        return true;
    }

    return false;
}

size_t FramingIO::transport_read(
        int& timeout,
        TransportRc& transport_rc,
        size_t max_size)
{
    const auto time_init = std::chrono::system_clock::now();

    /**
     * Compute read-buffer available size.
     * We keep track of two possible fragments to handle the case of
     * some intermediate section of the circular buffer being written,
     * that is, head > tail.
     */
    uint8_t available_length[2] = {0, 0};
    if (read_buffer_head_ == read_buffer_tail_)
    {
        read_buffer_head_ = 0;
        read_buffer_tail_ = 0;
        available_length[0] = sizeof(read_buffer_) - 1;
    }
    else if (read_buffer_head_ > read_buffer_tail_)
    {
        if (0 < read_buffer_tail_)
        {
            available_length[0] =
                static_cast<uint8_t>(sizeof(read_buffer_) - read_buffer_head_);
            available_length[1] =
                static_cast<uint8_t>(read_buffer_tail_ - 1);
        }
        else
        {
            available_length[0] =
                static_cast<uint8_t>(sizeof(read_buffer_) - read_buffer_head_ - 1);
        }
    }
    else
    {
        available_length[0] =
            static_cast<uint8_t>(read_buffer_tail_ - read_buffer_head_ - 1);
    }

    /**
     * Read from serial, according to the fragment size previously calculated.
     */
    size_t bytes_read[2] = {0, 0};

    // Limit the reading size
    if (max_size < available_length[0]){
        available_length[0] = (uint8_t)max_size;
        available_length[1] = 0;
    } else if(max_size < available_length[0] + available_length[1]){
        available_length[1] = (uint8_t)(max_size - available_length[0]);
    }

    if (0 < available_length[0])
    {
        ssize_t read_res = read_callback_(&read_buffer_[read_buffer_head_],
                                       available_length[0],
                                       timeout,
                                       transport_rc);
        bytes_read[0] = (0 < read_res) ? read_res : 0;

        read_buffer_head_ = static_cast<uint8_t>(
            static_cast<size_t>(read_buffer_head_ + bytes_read[0]) % sizeof(read_buffer_));

        if (0 < bytes_read[0])
        {
            if ((bytes_read[0] == available_length[0]) && (0 < available_length[1]))
            {
                read_res = read_callback_(&read_buffer_[read_buffer_head_],
                                       available_length[1],
                                       0,
                                       transport_rc);
                bytes_read[1] = (0 < read_res) ? read_res : 0;

                read_buffer_head_ = static_cast<uint8_t>(
                    static_cast<size_t>(read_buffer_head_ + bytes_read[1]) % sizeof(read_buffer_));
            }
        }
    }

    int time_elapsed = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - time_init)
            .count());

    timeout -= (time_elapsed == 0) ? 1 : time_elapsed;

    return bytes_read[0] + bytes_read[1];
}

} // namespace uxr
} // namespace eprosima