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

#ifndef _MESSAGE_DEBUGGER_H_
#define _MESSAGE_DEBUGGER_H_

#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <vector>

namespace eprosima {
namespace micrortps {

class MessageHeader;
class SubmessageHeader;
class RESOURCE_STATUS_Payload;
class CREATE_Payload;
class DELETE_RESOURCE_Payload;
class WRITE_DATA_Payload;
class READ_DATA_Payload;
class OBJK_PUBLISHER_Representation;
class OBJK_SUBSCRIBER_Representation;
class ResultStatus;
class OBJK_DATAWRITER_Representation;
class OBJK_DATA_READER_Representation;
class ObjectVariant;
class DATA_Payload_Data;

namespace debug {

enum class STREAM_COLOR
{
    YELLOW,
    RED,
    BLUE,
    GREEN,
    WHITE
};

class ColorStream
{
  public:
    ColorStream(std::ostream& stream, STREAM_COLOR color) : stream_(stream)
    {
        switch(color)
        {
            case STREAM_COLOR::YELLOW:
            {
                stream_ << YELLOW;
                break;
            }
            case STREAM_COLOR::RED:
            {
                stream_ << RED;
                break;
            }
            case STREAM_COLOR::BLUE:
            {
                stream_ << BLUE;
                break;
            }
            case STREAM_COLOR::GREEN:
            {
                stream_ << GREEN;
                break;
            }
            case STREAM_COLOR::WHITE:
            {
                stream_ << WHITE;
                break;
            }
            default:
                break;
        }
    }

    ~ColorStream()
    {
        stream_ << RESTORE_COLOR;
    }

  private:
    std::ostream& stream_;
    const std::string RED           = "\x1b[1;31m";
    const std::string GREEN         = "\x1b[1;32m";
    const std::string YELLOW        = "\x1b[1;33m";
    const std::string BLUE          = "\x1b[1;34m";
    const std::string WHITE         = "\x1b[1;37m";
    const std::string RESTORE_COLOR = "\x1b[0m";
};

class StreamScopedFlags
{
  public:
    StreamScopedFlags(std::ostream& stream) : stream_(stream)
    {
        oldFlags_ = stream.flags();
        oldPrec_  = stream.precision();
        oldFill_  = stream.fill();
    }

    ~StreamScopedFlags()
    {
        stream_.flags(oldFlags_);
        stream_.precision(oldPrec_);
        stream_.fill(oldFill_);
    }

  private:
    std::ostream& stream_;
    std::ios_base::fmtflags oldFlags_;
    std::streamsize oldPrec_;
    char oldFill_;
};

template <size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<unsigned char, N>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream << std::setfill('0') << std::setw(2);
    for(auto value : values)
    {
        stream << +value;
    }
    return stream;
}

template <size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<char, N>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream << std::setfill('0') << std::setw(2);
    for(auto value : values)
    {
        stream << +value;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::vector<unsigned char>& values);

std::ostream& operator<<(std::ostream& stream, const std::vector<unsigned char>& values);

/*
 * Inserts MessageHeader on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const MessageHeader& header);

/*
 * Inserts SubMessageHeader on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const SubmessageHeader& submessage_header);

/*
 * Inserts Status on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const RESOURCE_STATUS_Payload& status);

/*
 * Inserts Status short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const RESOURCE_STATUS_Payload& status,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts ResultStatus on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const ResultStatus& status);

/*
 * Inserts ResultStatus short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const ResultStatus& status,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Prints string
 */
std::ostream& short_print(std::ostream& stream, const std::string& text, const STREAM_COLOR color = STREAM_COLOR::BLUE);

/*
 * Inserts create payload on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const CREATE_Payload& create_payload);

/*
 * Inserts create short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const CREATE_Payload& create_payload,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts OBJK_PUBLISHER_Representation on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const OBJK_PUBLISHER_Representation& data);

/*
 * Inserts OBJK_SUBSCRIBER on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const OBJK_SUBSCRIBER_Representation& data);

/*
 * Inserts OBJK_DATAWRITER_Representation on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const OBJK_DATAWRITER_Representation& data);

/*
 * Inserts OBJK_DATA_READER_Representation on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const OBJK_DATA_READER_Representation& data);

/*
 * Inserts ObjectVariant on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const ObjectVariant& object_representation);

/*
 * Inserts Delete info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const DELETE_RESOURCE_Payload& delete_data);

/*
 * Inserts delete short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const DELETE_RESOURCE_Payload& delete_data,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts Write data info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const WRITE_DATA_Payload& write_data);

/*
 * Inserts Write data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const WRITE_DATA_Payload& write_data,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts READ_DATA_Payload info on the stream.
 */
void operator<<(std::ostream& stream, const READ_DATA_Payload& data);

/*
 * Inserts Read data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const READ_DATA_Payload& read_data,
                          const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts DATA_Payload_Data data info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const DATA_Payload_Data& data);

/*
 * Inserts DATA_Payload_Data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const DATA_Payload_Data& data, const STREAM_COLOR color =
STREAM_COLOR::YELLOW);

} // namespace debug
} // namespace micrortps
} // namespace eprosima

#endif //_MESSAGE_DEBUGGER_H_
