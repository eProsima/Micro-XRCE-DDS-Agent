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

#ifndef _XRCE_PARSER_H_
#define _XRCE_PARSER_H_

#include "Serializer.h"

namespace dds { namespace xrce {

class MessageHeader;
class SubmessageHeader;
class CREATE_CLIENT_Payload;
class CREATE_Payload;
class DELETE_Payload;
class WRITE_DATA_Payload_Data;
class READ_DATA_Payload;
class ACKNACK_Payload;
class HEARTBEAT_Payload;

} } //namespace dds::xrce

namespace eprosima {
namespace micrortps {


class XRCEListener {
public:
    XRCEListener() = default;
    virtual ~XRCEListener() = default;

    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::CREATE_CLIENT_Payload& create_client_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::CREATE_Payload& create_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::DELETE_Payload& create_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            dds::xrce::WRITE_DATA_Payload_Data&  write_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::READ_DATA_Payload&   read_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::ACKNACK_Payload&   acknack_payload) = 0;
    virtual void on_message(const dds::xrce::MessageHeader& header,
                            const dds::xrce::SubmessageHeader& sub_header,
                            const dds::xrce::HEARTBEAT_Payload& heartbeat_payload) = 0;
};

class XRCEParser
{
public:
    XRCEParser(char* buffer, size_t size, XRCEListener* listener)
        : listener_(listener),
          deserializer_(buffer, size)
    {

    }
    bool parse();
private:

    bool process_create_client(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_create(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_delete(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_write_data(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_read_data(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_heartbeat(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);
    bool process_acknack(const dds::xrce::MessageHeader& header, const dds::xrce::SubmessageHeader& sub_header);

    XRCEListener* listener_ = nullptr;
    Serializer deserializer_;
};


} /* namespace micrortps */
} /* namespace eprosima */

#endif // !_XRCE_PARSER_H_
