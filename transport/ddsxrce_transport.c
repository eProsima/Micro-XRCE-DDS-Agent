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

#include "ddsxrce_serial_transport.h"
#include "ddsxrce_transport.h"

channel_id_t ch_id = -1;

uint16_t crc16_byte(uint16_t crc, const octet data);
uint16_t crc16(octet const *buffer, size_t len);

channel_id_t add_locator(const locator_t* locator)
{
    if (NULL ==  locator)
    {
        return TRANSPORT_ERROR;
    }

    switch (locator->kind)
    {
        case LOC_SERIAL:
        {
            channel_id_t id = create_serial(locator);
            if (0 > id || 0 > open_serial(id))
            {
                return TRANSPORT_ERROR;
            }
            ch_id = id;
            return id;
        }
        break;
        default:
            return TRANSPORT_ERROR;
        break;
    }

    return  TRANSPORT_OK;
}

int rm_locator(const locator_t* locator)
{
    // TODO

    return  TRANSPORT_OK;
}


int send(const octet* in_buffer, const size_t buffer_len, const locator_kind_t kind, const channel_id_t channel_id)
{
    if (NULL == in_buffer)
    {
        return TRANSPORT_ERROR;
    }

    switch (kind)
    {
        case LOC_SERIAL: return send_serial(in_buffer, buffer_len, channel_id);
        default:         return TRANSPORT_ERROR;
    }
}

int receive(octet* out_buffer, const size_t buffer_len, const locator_kind_t kind, const channel_id_t channel_id)
{
    if (NULL == out_buffer)
    {
        return TRANSPORT_ERROR;
    }

    switch (kind)
    {
        case LOC_SERIAL: return receive_serial(out_buffer, buffer_len, channel_id);
        default:         return TRANSPORT_ERROR;
    }
}
