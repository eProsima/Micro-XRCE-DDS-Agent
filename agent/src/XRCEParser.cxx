#include "XRCEParser.h"

#include "ObjectVariant.h"
#include "Payloads.h"
#include "MessageHeader.h"
#include "SubMessageHeader.h"

#include <iostream>

class Listener {

};

bool XRCEParser::parse()
{
    MessageHeader message_header;
    if (deserializer_.deserialize(message_header))
    {
        SubmessageHeader submessage_header;
        bool valid_submessage = false;
        do
        {
            if (valid_submessage = deserializer_.deserialize(submessage_header))
            {
                switch (submessage_header.submessage_id())
                {
                case CREATE:
                    if (!process_create())
                        std::cerr << "Error processing create" << std::endl;
                    break;
                case WRITE_DATA:
                    if (!process_write_data())
                        std::cerr << "Error processing write" << std::endl;
                    break;
                case READ_DATA:
                    if (!process_read_data())
                        std::cerr << "Error processing read" << std::endl;
                    break;
                case GET_INFO:
                case DELETE:
                case STATUS:
                case INFO:
                case DATA:
                case ACKNACK:
                case HEARTBEAT:
                case RESET:
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

bool XRCEParser::process_create()
{
    CREATE_PAYLOAD create_payload;
    if (deserializer_.deserialize(create_payload))
    {
        if (create_callback_)
        {
            create_callback_();
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool XRCEParser::process_read_data()
{
    READ_DATA_PAYLOAD read_data_payload;
    if (deserializer_.deserialize(read_data_payload))
    {
        if (read_data_callback_)
        {
            read_data_callback_();
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool XRCEParser::process_write_data()
{
    WRITE_DATA_PAYLOAD write_data_payload;
    if (deserializer_.deserialize(write_data_payload))
    {
        if (write_data_callback_)
        {
            write_data_callback_();
            return true;
        }
        else
        {
            return false;
        }
    }
    return false;
}
