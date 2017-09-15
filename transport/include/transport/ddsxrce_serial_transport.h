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

#ifndef _DDSXRCE_SERIAL_TRANSPORT_H_
#define _DDSXRCE_SERIAL_TRANSPORT_H_

#include "ddsxrce_transport_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

channel_id_t create_serial(const locator_t* locator);
int destroy_serial(const channel_id_t channel_id);
int open_serial(const channel_id_t channel_id);
int close_serial(const channel_id_t channel_id);
int send_serial(const octet* in_buffer, const size_t length, const channel_id_t channel_id);
int receive_serial(octet* out_buffer, const size_t buffer_len, const channel_id_t channel_id);

#ifdef __cplusplus
}
#endif

#endif
