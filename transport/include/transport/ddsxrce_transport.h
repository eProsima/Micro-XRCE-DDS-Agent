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

#ifndef _DDSXRCE_TRANSPORT_H_
#define _DDSXRCE_TRANSPORT_H_

#include "ddsxrce_transport_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

channel_id_t add_locator(const locator_t* locator);
int rm_locator(const locator_t* locator);
int send_data(const octet* in_buffer, const size_t buffer_len, const locator_kind_t kind, const channel_id_t channel_id);
int receive_data(octet* out_buffer, const size_t buffer_len, const locator_kind_t kind, const channel_id_t channel_id);

#ifdef __cplusplus
}
#endif

#endif
