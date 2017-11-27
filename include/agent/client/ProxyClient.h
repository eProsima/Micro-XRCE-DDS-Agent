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

#ifndef _PROXY_CLIENT_H
#define _PROXY_CLIENT_H

#include <MessageHeader.h>
#include <agent/datareader/DataReader.h>

#include <array>
#include <map>

namespace eprosima {
namespace micrortps {

class ProxyClient : public ReaderListener
{
  public:
    ProxyClient() = default;
    ProxyClient(OBJK_CLIENT_Representation client, const MessageHeader& header);
    ~ProxyClient();

    ProxyClient(const ProxyClient& x) = delete;
    ProxyClient(ProxyClient&& x) noexcept;
    ProxyClient& operator=(const ProxyClient& x) = delete;
    ProxyClient& operator=(ProxyClient&& x) noexcept;

    ResultStatus create(const CreationMode& creation_mode, const CREATE_Payload& create_payload);
    ResultStatus delete_object(const DELETE_RESOURCE_Payload& delete_payload);
    ResultStatus update(const ObjectId& object_id, const ObjectVariant& representation);
    ResultStatus read(const ObjectId& object_id, const READ_DATA_Payload& data_payload);
    ResultStatus write(const ObjectId& object_id, const WRITE_DATA_Payload& data_payload);
    Info get_info(const ObjectId& object_id);

    void on_read_data(const ObjectId& object_id, const RequestId& req_id, const std::vector<unsigned char>& buffer);

    XRCEObject* get_object(const ObjectId& object_id);

  private:
    OBJK_CLIENT_Representation representation_;

    std::mutex objects_mutex_;
    std::map<ObjectId, std::unique_ptr<XRCEObject>> objects_;

    ClientKey client_key;
    SessionId session_id;
    uint8_t stream_id;

    bool create(const ObjectId& id, const ObjectVariant& representation);
    bool delete_object(const ObjectId& id);
};
} // namespace micrortps
} // namespace eprosima

#endif //_PROXY_CLIENT_H
