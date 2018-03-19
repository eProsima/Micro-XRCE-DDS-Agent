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
#include <agent/client/StreamsManager.h>

#include <array>
#include <map>

namespace eprosima {
namespace micrortps {

class ProxyClient : public ReaderListener
{
public:
    ProxyClient() = default;

    /*!
     * \brief Creates a ProxyClient from a given object representation.
     * \param client
     * \param header
     */
    ProxyClient(dds::xrce::CLIENT_Representation client, const dds::xrce::MessageHeader& header);

    ~ProxyClient();

    /*!
     * \brief Deletes the copy constructor.
     */
    ProxyClient(const ProxyClient&) = delete;

    /*!
     * \brief Default move constructor.
     * \param x Object to move.
     */
    ProxyClient(ProxyClient&& x) noexcept;

    /*!
     * \brief Deletes the assignment operator.
     */
    ProxyClient& operator=(const ProxyClient&) = delete;

    /*!
     * \brief Default move assignment operator.
     * \param x Object to move.
     * \return New reference where the object is moved.
     */
    ProxyClient& operator=(ProxyClient&& x) noexcept;

    /*!
     * \brief Creates a DdsXrce object from a given CREATE submessage payload.
     * \param creation_mode 	Controls the behavior of the operation when there is an existing
     * 							object that partially matches the description of the object that
     * 							the client  wants to create (see Table 4 in DDS-XRCE manual).
     * \param create_payload	The CREATE submessage payload.
     * \return If the creation succeeds the Agent shall return {STATUS_LAST_OP_CREATE, STATUS_OK}.
     */
    dds::xrce::ResultStatus create(const dds::xrce::CreationMode& creation_mode,
                                   const dds::xrce::CREATE_Payload& create_payload);
    /* TODO (Julian): add comments for API */
    dds::xrce::ResultStatus delete_object(const dds::xrce::DELETE_Payload& delete_payload);
    dds::xrce::ResultStatus update(const dds::xrce::ObjectId& object_id,
                                   const dds::xrce::ObjectVariant& representation);
    dds::xrce::ResultStatus read(const dds::xrce::ObjectId& object_id,
                                 const dds::xrce::READ_DATA_Payload& data_payload);
    dds::xrce::ResultStatus write(const dds::xrce::ObjectId& object_id,
                                  dds::xrce::WRITE_DATA_Payload_Data& data_payload);
    dds::xrce::ObjectInfo get_info(const dds::xrce::ObjectId& object_id);
    void on_read_data(const dds::xrce::ObjectId& object_id,
                      const dds::xrce::RequestId& req_id,
                      const std::vector<unsigned char>& buffer);
    XRCEObject* get_object(const dds::xrce::ObjectId& object_id);

private:
    dds::xrce::CLIENT_Representation representation_;

    std::mutex objects_mutex_;
    std::map<dds::xrce::ObjectId, std::unique_ptr<XRCEObject>> objects_;

    dds::xrce::ClientKey client_key;
    dds::xrce::SessionId session_id;

    StreamsManager streams_manager_;

    bool create(const dds::xrce::ObjectId& id, const dds::xrce::ObjectVariant& representation);
    bool delete_object(const dds::xrce::ObjectId& id);
};
} // namespace micrortps
} // namespace eprosima

#endif //_PROXY_CLIENT_H
