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

#include "agent/XRCEParser.h"

#include "XRCETypes.h"
#include "MessageHeader.h"
#include "SubMessageHeader.h"

#include <iostream>

namespace eprosima {
namespace micrortps {

bool XRCEParser::parse()
{
    dds::xrce::MessageHeader message_header;
    if (deserializer_.deserialize(message_header))
    {
        dds::xrce::SubmessageHeader submessage_header;
        bool valid_submessage = false;
        do
        {
            deserializer_.force_new_submessage_align();

            if ((valid_submessage = deserializer_.deserialize(submessage_header)))
            {
                switch (submessage_header.submessage_id())
                {
                case dds::xrce::CREATE_CLIENT:
                    if (!process_create_client(message_header, submessage_header))
                    {
                        std::cerr << "Error processing create client" << std::endl;
                    }
                    break;
                case dds::xrce::CREATE:
                    if (!process_create(message_header, submessage_header))
                    {
                        std::cerr << "Error processing create" << std::endl;
                    }
                    break;
                case dds::xrce::WRITE_DATA:
                    if (!process_write_data(message_header, submessage_header))
                    {
                        std::cerr << "Error processing write" << std::endl;
                    }
                    break;
                case dds::xrce::READ_DATA:
                    if (!process_read_data(message_header, submessage_header))
                    {
                        std::cerr << "Error processing read" << std::endl;
                    }
                    break;
               case dds::xrce::DELETE:
                   if(!process_delete(message_header, submessage_header))
                   {
                       std::cerr << "Error processing delete" << std::endl;
                   }
                   break;
                /* TODO (Julian): add support for the rest of the messages. */
                case dds::xrce::GET_INFO:
                case dds::xrce::STATUS:
                case dds::xrce::DATA:
                case dds::xrce::ACKNACK:
                case dds::xrce::HEARTBEAT:
                    if (!process_heartbeat(message_header, submessage_header))
                    {
                        std::cerr << "Error procession heartbeat";
                    }
                    break;
                case dds::xrce::FRAGMENT:
                case dds::xrce::FRAGMENT_END:
                default:
                    std::cerr << "Error submessage ID not recognized " << submessage_header.submessage_id() << std::endl;
                    return false;
                    break;
                }
            }
            else
            {
                std::cerr << "Error reading submessage header" << std::endl;
                return false;
            }
        } while (valid_submessage && !deserializer_.bufferEnd());
    }
    else
    {
        std::cerr << "Error reading message header" << std::endl;
        return false;
    }
    return true;
}

bool XRCEParser::process_create_client(const dds::xrce::MessageHeader& header,
                                       const dds::xrce::SubmessageHeader& sub_header)
{
    dds::xrce::CREATE_CLIENT_Payload create_payload;
    if (deserializer_.deserialize(create_payload))
    {
        listener_->on_message(header, sub_header, create_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_create(const dds::xrce::MessageHeader& header,
                                const dds::xrce::SubmessageHeader& sub_header)
{
    dds::xrce::CREATE_Payload create_payload;
    if (deserializer_.deserialize(create_payload))
    {
        listener_->on_message(header, sub_header, create_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_delete(const dds::xrce::MessageHeader& header,
                                const dds::xrce::SubmessageHeader& sub_header)
{
    dds::xrce::DELETE_Payload delete_payload;
    if (deserializer_.deserialize(delete_payload))
    {
        listener_->on_message(header, sub_header, delete_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_read_data(const dds::xrce::MessageHeader& header,
                                   const dds::xrce::SubmessageHeader& sub_header)
{
    dds::xrce::READ_DATA_Payload read_data_payload;
    if (deserializer_.deserialize(read_data_payload))
    {
        listener_->on_message(header, sub_header, read_data_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_write_data(const dds::xrce::MessageHeader& header,
                                    const dds::xrce::SubmessageHeader& sub_header)
{
    dds::xrce::WRITE_DATA_Payload_Data write_data_payload;
    if (deserializer_.deserialize(write_data_payload))
    {
        listener_->on_message(header, sub_header, write_data_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_heartbeat(const dds::xrce::MessageHeader &header,
                                   const dds::xrce::SubmessageHeader &sub_header)
{
    dds::xrce::HEARTBEAT_Payload heartbeat_payload;
    if (deserializer_.deserialize(heartbeat_payload))
    {
        listener_->on_message(header, sub_header, heartbeat_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_acknack(const dds::xrce::MessageHeader &header,
                                 const dds::xrce::SubmessageHeader &sub_header)
{
    dds::xrce::ACKNACK_Payload acknack_payload;
    if (deserializer_.deserialize(acknack_payload))
    {
        listener_->on_message(header, sub_header, acknack_payload);
        return true;
    }
    return false;
}

} } // namespace eprosima::micrortps
