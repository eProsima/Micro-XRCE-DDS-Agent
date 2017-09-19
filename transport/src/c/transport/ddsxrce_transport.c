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
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>

#include "ddsxrce_serial_transport.h"
#include "ddsxrce_transport.h"

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
