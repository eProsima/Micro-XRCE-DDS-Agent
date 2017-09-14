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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <poll.h>

#include "ddsxrce_serial_transport.h"
#include "ddsxrce_transport_common.h"

static serial_channel_t* g_channels[MAX_NUM_SERIAL_CHANNELS];
static struct pollfd g_poll_fds[MAX_NUM_SERIAL_CHANNELS] = {};
static channel_id_t g_num_channels = 0;

uint16_t crc16_byte(uint16_t crc, const uint8_t data);
uint16_t crc16(uint8_t const *buffer, size_t len);

serial_channel_t* get_serial_channel(const channel_id_t ch_id);
channel_id_t create_serial(const locator_t* locator);
int destroy_serial(const channel_id_t channel_id);
int open_serial(const channel_id_t channel_id);
int close_serial(const channel_id_t channel_id);
int read_serial(void *buffer, const size_t len, const serial_channel_t* channel);
int receive_serial(octet* out_buffer, const size_t buffer_len, const channel_id_t channel_id);
int write_serial(const void* buffer, const size_t len, const serial_channel_t* channel);
int send_serial(const octet* in_buffer, const size_t length, const channel_id_t channel_id);

serial_channel_t* get_serial_channel(const channel_id_t ch_id)
{
    if (0 > ch_id || MAX_NUM_SERIAL_CHANNELS <= ch_id)
    {
        return NULL;
    }

    return g_channels[ch_id];
}

channel_id_t create_serial(const locator_t* locator)
{
    if (NULL == locator || MAX_NUM_SERIAL_CHANNELS <= g_num_channels)
    {
        return TRANSPORT_ERROR;
    }

    serial_channel_t* channel = malloc(sizeof(serial_channel_t));
    if (NULL == channel)
    {
        return TRANSPORT_ERROR;
    }

    memcpy(channel->uart_name, locator->data, sizeof(locator->data));
    channel->uart_fd = -1;
    memset(channel->rx_buffer, 0, RX_BUFFER_LENGTH);
    channel->rx_buff_pos = 0;
    channel->baudrate = DFLT_BAUDRATE;
    channel->poll_ms = DFLT_POLL_MS;

    for (int i = 0; i < MAX_NUM_SERIAL_CHANNELS; ++i)
    {
        if (NULL == g_channels[i])
        {
            channel->id = i;
            g_channels[i] = channel;
            g_num_channels++;
            break;
        }
    }

    return channel->id;
}

int destroy_serial(const channel_id_t channel_id)
{
    serial_channel_t* channel = get_serial_channel(channel_id);
    if (NULL == channel)
    {
        return TRANSPORT_ERROR;
    }

    if (channel->open)
    {
        close_serial(channel_id);
    }

    free(channel);

    return TRANSPORT_OK;
}

int open_serial(const channel_id_t channel_id)
{
    serial_channel_t* channel = get_serial_channel(channel_id);
    if (NULL == channel)
    {
        return TRANSPORT_ERROR;
    }

    // Open a serial port
    channel->uart_fd = open(channel->uart_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (channel->uart_fd < 0)
    {
        printf("failed to open device: %s (%d)\n", channel->uart_name, errno);
        return -errno;
    }

    // Try to set baud rate
    struct termios uart_config;
    int termios_state;

    // Back up the original uart configuration to restore it after exit
    if ((termios_state = tcgetattr(channel->uart_fd, &uart_config)) < 0)
    {
        int errno_bkp = errno;
        printf("ERR GET CONF %s: %d (%d)\n", channel->uart_name, termios_state, errno);
        close_serial(channel_id);
        return -errno_bkp;
    }

    // Clear ONLCR flag (which appends a CR for every LF)
    uart_config.c_oflag &= ~ONLCR;

    // USB serial is indicated by /dev/ttyACM0
    if (strcmp(channel->uart_name, "/dev/ttyACM0") != 0 && strcmp(channel->uart_name, "/dev/ttyACM1") != 0)
    {
        // Set baud rate
        if (cfsetispeed(&uart_config, channel->baudrate) < 0 || cfsetospeed(&uart_config, channel->baudrate) < 0)
        {
            int errno_bkp = errno;
            printf("ERR SET BAUD %s: %d (%d)\n", channel->uart_name, termios_state, errno);
            close_serial(channel_id);
            return -errno_bkp;
        }
    }

    if ((termios_state = tcsetattr(channel->uart_fd, TCSANOW, &uart_config)) < 0)
    {
        int errno_bkp = errno;
        printf("ERR SET CONF %s (%d)\n", channel->uart_name, errno);
        close_serial(channel_id);
        return -errno_bkp;
    }

    char aux[64];
    bool flush = false;
    while (0 < read(channel->uart_fd, (void *)&aux, 64))
    {
        printf("%s", aux);
        flush = true;
        usleep(1000);
    }

    if (flush)
    {
        printf("flush\n");

    } else
    {
        printf("no flush\n");
    }

    channel->open = true;
    g_poll_fds[channel->id].fd = channel->uart_fd;
    g_poll_fds[channel->id].events = POLLIN;

    return channel->uart_fd;
}

int close_serial(const channel_id_t channel_id)
{
    serial_channel_t* channel = get_serial_channel(channel_id);
    if (NULL == channel || 0 > channel->uart_fd)
    {
        return TRANSPORT_ERROR;
    }

    printf("Close UART\n");
    close(channel->uart_fd);
    channel->uart_fd = -1;
    channel->open = false;
    memset(&g_poll_fds[channel->id], 0, sizeof(struct pollfd));

    return 0;
}

int read_serial(void *buffer, const size_t len, const serial_channel_t* channel)
{
    if (NULL == buffer       ||
        NULL == channel      ||
        0 > channel->uart_fd ||
        (!channel->open && 0 > open_serial(channel->id)))
    {
        return TRANSPORT_ERROR;
    }

    // TODO: for several channels this can be optimized
    int ret = 0;
    int r = poll(g_poll_fds, g_num_channels, channel->poll_ms);
    if (r > 0 && (g_poll_fds[channel->id].revents & POLLIN))
    {
        ret = read(channel->uart_fd, buffer, len);
    }

    return ret;
}


int receive_serial(octet* out_buffer, const size_t buffer_len, const channel_id_t channel_id)
{
    if (NULL == out_buffer)
    {
        return TRANSPORT_ERROR;
    }

    serial_channel_t* ch = get_serial_channel(channel_id);
    if (NULL == ch      ||
        0 > ch->uart_fd ||
        (!ch->open && 0 > open_serial(ch->id)))
    {
        return TRANSPORT_ERROR;
    }

    int len = read_serial((void *) (ch->rx_buffer + ch->rx_buff_pos), sizeof(ch->rx_buffer) - ch->rx_buff_pos, ch);
    if (len <= 0)
    {
        int errsv = errno;

        if (errsv && EAGAIN != errsv && ETIMEDOUT != errsv)
        {
            printf("Read fail %d\n", errsv);
        }

        return len;
    }

    ch->rx_buff_pos += len;

    // We read some
    size_t header_size = sizeof(header_t);

    // but not enough
    if (ch->rx_buff_pos < header_size)
    {
        return 0;
    }

    uint32_t msg_start_pos = 0;

    for (msg_start_pos = 0; msg_start_pos <= ch->rx_buff_pos - header_size; ++msg_start_pos)
    {
        if ('>' == ch->rx_buffer[msg_start_pos] && memcmp(ch->rx_buffer + msg_start_pos, ">>>", 3) == 0)
        {
            break;
        }
    }

    // Start not found
    if (msg_start_pos > ch->rx_buff_pos - header_size)
    {
        printf("                                 (↓↓ %u)\n", msg_start_pos);
        // All we've checked so far is garbage, drop it - but save unchecked bytes
        memmove(ch->rx_buffer, ch->rx_buffer + msg_start_pos, ch->rx_buff_pos - msg_start_pos);
        ch->rx_buff_pos = ch->rx_buff_pos - msg_start_pos;
        return -1;
    }

    /*
     * [>,>,>,length_H,length_L,CRC_H,CRC_L,payloadStart, ... ,payloadEnd]
     */

    header_t* header = (header_t*) &ch->rx_buffer[msg_start_pos];
    uint32_t payload_len = ((uint32_t) header->payload_len_h << 8) | header->payload_len_l;

    // The message won't fit the buffer.
    if (buffer_len < header_size + payload_len)
    {
        return -EMSGSIZE;
    }

    // We do not have a complete message yet
    if (msg_start_pos + header_size + payload_len > ch->rx_buff_pos)
    {
        // If there's garbage at the beginning, drop it
        if (msg_start_pos > 0)
        {
            printf("                                 (↓ %u)\n", msg_start_pos);
            memmove(ch->rx_buffer, ch->rx_buffer + msg_start_pos, ch->rx_buff_pos - msg_start_pos);
            ch->rx_buff_pos -= msg_start_pos;
        }

        return 0;
    }

    uint16_t read_crc = ((uint16_t) header->crc_h << 8) | header->crc_l;
    uint16_t calc_crc = crc16((uint8_t *) ch->rx_buffer + msg_start_pos + header_size, payload_len);

    if (read_crc != calc_crc)
    {
        printf("BAD CRC %u != %u\n", read_crc, calc_crc);
        printf("                                 (↓ %lu)\n", (unsigned long) (header_size + payload_len));
        len = -1;

    }
    else
    {
        // copy message to outbuffer and set other return values
        memmove(out_buffer, ch->rx_buffer + msg_start_pos + header_size, payload_len);
        len = payload_len;
    }

    // discard message from rx_buffer
    ch->rx_buff_pos -= header_size + payload_len;
    memmove(ch->rx_buffer, ch->rx_buffer + msg_start_pos + header_size + payload_len, ch->rx_buff_pos);

    return len;
}

int write_serial(const void* buffer, const size_t len, const serial_channel_t* channel)
{
    if (NULL == buffer       ||
        NULL == channel      ||
        0 > channel->uart_fd ||
        (!channel->open && 0 > open_serial(channel->id)))
    {
        return TRANSPORT_ERROR;
    }

    return write(channel->uart_fd, buffer, len);
}

int send_serial(const octet* in_buffer, const size_t length, const channel_id_t channel_id)
{
    if (NULL == in_buffer)
    {
        return TRANSPORT_ERROR;
    }

    serial_channel_t* ch = get_serial_channel(channel_id);
    if (NULL == ch      ||
        0 > ch->uart_fd ||
        (!ch->open && 0 > open_serial(ch->id)))
    {
        return TRANSPORT_ERROR;
    }

    static struct Header header = {
        .marker = {'>', '>', '>'},
        .payload_len_h = 0u,
        .payload_len_l = 0u,
        .crc_h = 0u,
        .crc_l = 0u

    };

    // [>,>,>,topic_ID,seq,payload_length,CRCHigh,CRCLow,payload_start, ... ,payload_end]

    uint16_t crc = crc16(in_buffer, length);
    header.payload_len_h = (length >> 8) & 0xff;
    header.payload_len_l = length & 0xff;
    header.crc_h = (crc >> 8) & 0xff;
    header.crc_l = crc & 0xff;

    int len = write_serial(&header, sizeof(header), ch);
    if (len != sizeof(header))
    {
        return len;
    }

    len = write_serial(in_buffer, length, ch);
    if (len != length)
    {
        return len;
    }

    return len + sizeof(header);
}
