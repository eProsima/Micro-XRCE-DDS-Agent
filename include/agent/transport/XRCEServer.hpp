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

#ifndef _MICRORTPS_AGENT_TRANSPORT_XRCE_SERVER_HPP_
#define _MICRORTPS_AGENT_TRANSPORT_XRCE_SERVER_HPP_

#include <stdint.h>
#include <stddef.h>

namespace eprosima {
namespace micrortps {

/******************************************************************************
 * Abstract Client class.
 ******************************************************************************/
class TransportClient
{
public:
    TransportClient() {}
    virtual ~TransportClient() {}
};

/******************************************************************************
 * Server interface.
 ******************************************************************************/
class XRCEServer
{
public:
    XRCEServer() {}
    virtual ~XRCEServer() {}

    virtual bool send_msg(const uint8_t* buf, size_t len, TransportClient* client) = 0;
    virtual bool recv_msg(uint8_t** buf, size_t* len, int timeout, TransportClient** client) = 0;
    virtual int get_error() = 0;
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_TRANSPORT_XRCE_SERVER_HPP_
