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

#ifndef _SERIALIZER_H
#define _SERIALIZER_H

#include <stdint.h>

#include "fastcdr/Cdr.h"
#include "fastcdr/exceptions/Exception.h"

namespace eprosima {
namespace micrortps {

class Serializer
{
public:
    Serializer(char* buffer, size_t buffer_size );
    
    template<class T> bool serialize(const T& data);

    template<class T> bool deserialize(T& data);

    size_t get_serialized_size();
    bool bufferEnd();
private:

    void align();

    eprosima::fastcdr::FastBuffer fastbuffer_; // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr serializer_; // Object that serializes the data.

};

} /* namespace micrortps */
} /* namespace eprosima */

#endif // !_SERIALIZER_H
