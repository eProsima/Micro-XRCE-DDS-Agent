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

// #include "libdev/MessageDebugger.h"

#include "MessageHeader.h"
#include "SubMessageHeader.h"
#include "Payloads.h"

#include <iostream>

namespace eprosima {
namespace micrortps {

bool XRCEParser::parse()
{
    MessageHeader message_header;
    if (deserializer_.deserialize(message_header))
    {
        SubmessageHeader submessage_header;
        bool valid_submessage = false;
        do
        {
            if ((valid_submessage = deserializer_.deserialize(submessage_header)))
            {
                switch (submessage_header.submessage_id())
                {
                case CREATE_CLIENT:
                    if (!process_create_client(message_header, submessage_header))
                    {
                        std::cerr << "Error processing create client" << std::endl;
                    }
                    break;
                case CREATE:
                    if (!process_create(message_header, submessage_header))
                    {
                        std::cerr << "Error processing create" << std::endl;
                    }
                    break;
                case WRITE_DATA:
                    if (!process_write_data(message_header, submessage_header))
                    {
                        std::cerr << "Error processing write" << std::endl;
                    }
                    break;
                case READ_DATA:
                    if (!process_read_data(message_header, submessage_header))
                    {
                        std::cerr << "Error processing read" << std::endl;
                    }
                    break;
               case DELETE:
                   if(!process_delete(message_header, submessage_header))
                   {
                       std::cerr << "Error processing delete" << std::endl;
                   }
                   break;
                case GET_INFO:
                case STATUS:
                case INFO:
                case DATA:
                case ACKNACK:
                case HEARTBEAT:
                case FRAGMENT:
                case FRAGMENT_END:
                default:
                    std::cerr << "Error submessage ID not recognized" << std::endl;
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

bool XRCEParser::process_create_client(const MessageHeader& header, const SubmessageHeader& sub_header)
{
    CREATE_CLIENT_Payload create_payload;
    if (deserializer_.deserialize(create_payload))
    {
        listener_->on_message(header, sub_header, create_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_create(const MessageHeader& header, const SubmessageHeader& sub_header)
{
    CREATE_Payload create_payload;
    if (deserializer_.deserialize(create_payload))
    {
        listener_->on_message(header, sub_header, create_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_delete(const MessageHeader& header, const SubmessageHeader& sub_header)
{
    DELETE_RESOURCE_Payload delete_payload;
    if (deserializer_.deserialize(delete_payload))
    {
        listener_->on_message(header, sub_header, delete_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_read_data(const MessageHeader& header, const SubmessageHeader& sub_header)
{
    READ_DATA_Payload read_data_payload;
    if (deserializer_.deserialize(read_data_payload))
    {
        listener_->on_message(header, sub_header, read_data_payload);
        return true;
    }
    return false;
}

bool XRCEParser::process_write_data(const MessageHeader& header, const SubmessageHeader& sub_header)
{
    WRITE_DATA_Payload write_data_payload;
    if (deserializer_.deserialize(write_data_payload))
    {
        listener_->on_message(header, sub_header, write_data_payload);
        return true;
    }
    return false;
}

} /* namespace micrortps */
} /* namespace eprosima */
