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

#ifndef XRC_UTILS_CONVERTION_HPP_
#define XRC_UTILS_CONVERTION_HPP_

#include <uxr/agent/types/XRCETypes.hpp>

namespace eprosima {
namespace uxr {
namespace conversion {

inline uint16_t objectid_to_raw(const dds::xrce::ObjectId& object_id)
{
    return uint16_t((object_id[1] >> 4) + (object_id[0] << 4));
}

inline dds::xrce::ObjectId raw_to_objectid(uint16_t raw, dds::xrce::ObjectKind kind)
{
    return dds::xrce::ObjectId{uint8_t(raw >> 4), uint8_t((raw << 4) | kind)};
}

inline dds::xrce::ObjectId raw_to_objectprefix(uint16_t raw)
{
    return dds::xrce::ObjectId{uint8_t(raw >> 4), uint8_t(raw << 4)};
}

inline uint32_t clientkey_to_raw(const dds::xrce::ClientKey& client_key)
{
    return uint32_t(client_key[0] << 24)
         + uint32_t(client_key[1] << 16)
         + uint32_t(client_key[2] << 8)
         + uint32_t(client_key[3]);
}

inline dds::xrce::ClientKey raw_to_clientkey(uint32_t key)
{
    dds::xrce::ClientKey client_key{
        uint8_t(key >> 24),
        uint8_t(key >> 16),
        uint8_t(key >> 8),
        uint8_t(key)};
    return client_key;
}

} // namespace conversion
} // namespace uxr
} // namespace eprosima

#endif // XRC_UTILS_CONVERTION_HPP_
