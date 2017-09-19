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

class MessageHeader;
class SubmessageHeader;
class CREATE_PAYLOAD;
class DELETE_PAYLOAD;
class WRITE_DATA_PAYLOAD;
class READ_DATA_PAYLOAD;

class XRCEListener {
public:
    XRCEListener() = default;
    virtual ~XRCEListener() = default;

    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const CREATE_PAYLOAD& create_payload) = 0;
    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const DELETE_PAYLOAD& create_payload) = 0;
    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const WRITE_DATA_PAYLOAD&  write_payload) = 0;
    virtual void on_message(const MessageHeader& header, const SubmessageHeader& sub_header, const READ_DATA_PAYLOAD&   read_payload) = 0;
};

class XRCEParser
{
public:
    XRCEParser(char* buffer, size_t size, XRCEListener* listener)
        : deserializer_(buffer, size),
        listener_(listener)
    {

    }
    bool parse();
private:

    bool process_create(const MessageHeader& header, const SubmessageHeader& sub_header);
    bool process_delete(const MessageHeader& header, const SubmessageHeader& sub_header);
    bool process_write_data(const MessageHeader& header, const SubmessageHeader& sub_header);
    bool process_read_data(const MessageHeader& header, const SubmessageHeader& sub_header);

    void print(const MessageHeader& message_header);
    void print(const SubmessageHeader& submessage_header);
    void print(const CREATE_PAYLOAD& create_message);

    XRCEListener* listener_ = nullptr;
    Serializer deserializer_;
};

#endif // !_XRCE_PARSER_H_
