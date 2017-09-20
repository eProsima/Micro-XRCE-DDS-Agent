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

#ifndef _XRCE_FACTORY_H_
#define _XRCE_FACTORY_H_

#include <stdint.h>

#include "Serializer.h"

namespace eprosima {
namespace micrortps {

class Status;
class DATA_PAYLOAD;

class XRCEFactory
{
public:
    XRCEFactory(char* buffer, uint32_t max_size) : serializer_(buffer, max_size) {};
    void header(int32_t client_key, uint8_t session_id, uint8_t stream_id, uint16_t sequence_nr);
    void status(const Status& payload);
    void data(const DATA_PAYLOAD& payload);
    size_t get_total_size();
private:

    void submessage_header(uint8_t submessage_id, uint8_t flags, uint16_t submessage_length);
    Serializer serializer_;
};


} /* namespace micrortps */
} /* namespace eprosima */

#endif // !_XRCE_FACTORY_H
