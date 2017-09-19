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

#include <agent/ObjectVariant.h>
#include <agent/datareader/DataReader.h>
#include <agent/Payloads.h>

#include <map>

namespace eprosima{
namespace micrortps{

class ProxyClient: public ReaderListener
{
public:
    ProxyClient() = default;
    ProxyClient(const OBJK_CLIENT_Representation& client);
    ~ProxyClient();

    Status create(const CreationMode& creation_mode, const CREATE_PAYLOAD& create_payload);
    Status delete_object(const DELETE_PAYLOAD& delete_payload);
    Status update(const ObjectId& object_id, const ObjectVariant& representation);
    Status read(const ObjectId& object_id, const READ_DATA_PAYLOAD& data_payload);
    Status write(const ObjectId& object_id, const WRITE_DATA_PAYLOAD& data_payload);
    Info get_info(const ObjectId& object_id);
    
    void on_read_data(const ObjectId& object_id, const RequestId& req_id, const octet* data, const size_t length){;}
    
private:
    using InternalObjectId = std::array<uint8_t, 4>;
    
    OBJK_CLIENT_Representation representation_;
    
    std::map<InternalObjectId, XRCEObject*> _objects;
    
    bool create(const InternalObjectId& internal_id, const ObjectVariant& representation);
    bool delete_object(const ObjectId& object_id);
    InternalObjectId generate_object_id(const ObjectId& id, uint8_t suffix) const;

};
} // eprosima
} // micrortps

#endif //_PROXY_CLIENT_H
