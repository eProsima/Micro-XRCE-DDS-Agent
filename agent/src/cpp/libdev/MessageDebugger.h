#ifndef _MESSAGE_DEBUGGER_H_
#define _MESSAGE_DEBUGGER_H_

#include <array>
#include <functional>
#include <iomanip>
#include <iostream>
#include <vector>

namespace eprosima{
namespace micrortps{

class MessageHeader;
class SubmessageHeader;
class Status;
class ResultStatus;
class CREATE_PAYLOAD;
class OBJK_PUBLISHER_Representation;
class OBJK_SUBSCRIBER_Representation;
class OBJK_DATAWRITER_Representation;
class OBJK_DATAREADER_Representation;
class ObjectVariant;
class DELETE_PAYLOAD;
class WRITE_DATA_PAYLOAD;
class RT_Data;
class SampleData;
class READ_DATA_PAYLOAD;
class DATA_PAYLOAD;

namespace debug{

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
    const std::string RED = "\e[1;31m";
    const std::string GREEN = "\e[1;32m";
    const std::string YELLOW = "\e[1;33m";
    const std::string BLUE = "\e[1;34m";
    const std::string WHITE = "\e[1;37m";
    const std::string RESTORE_COLOR = "\e[0m";
};

class StreamScopedFlags
{
public:
    StreamScopedFlags(std::ostream& stream) : stream_(stream)
    {
        oldFlags_ = stream.flags();
        oldPrec_ = stream.precision();
        oldFill_ = stream.fill();
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
    std::streamsize         oldPrec_;
    char                    oldFill_;
};

template <size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<unsigned char, N>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream  << std::setfill('0') << std::setw(2) << +values[0];
    return stream;
}

template <size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<char, N>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream  << std::setfill('0') << std::setw(2) << +values[0];
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
std::ostream& operator<<(std::ostream& stream, const Status& status);

/*
 * Inserts ResultStatus on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const ResultStatus& status);


/*
* Inserts Status short representation on the stream.
*/
std::ostream& short_print(std::ostream& stream, const Status& status, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts ResultStatus short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const ResultStatus& status, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Prints string
 */
std::ostream& short_print(std::ostream& stream, const std::string& text, const STREAM_COLOR color = STREAM_COLOR::BLUE);

/*
 * Inserts create payload on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const CREATE_PAYLOAD& create_payload);

/*
* Inserts create short representation on the stream.
*/
std::ostream& short_print(std::ostream& stream, const CREATE_PAYLOAD& create_payload, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

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
 * Inserts OBJK_DATAREADER_Representation on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const OBJK_DATAREADER_Representation& data);

/*
 * Inserts ObjectVariant on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const ObjectVariant& object_representation);

/*
 * Inserts Delete info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const DELETE_PAYLOAD& delete_data);

/*
* Inserts delete short representation on the stream.
*/
std::ostream& short_print(std::ostream& stream, const DELETE_PAYLOAD& delete_data, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts SampleData data info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const SampleData& data);

/*
 * Inserts RT_Data data info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const RT_Data& data);

/*
 * Inserts Write data info on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const WRITE_DATA_PAYLOAD& write_data);

/*
 * Inserts Write data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const WRITE_DATA_PAYLOAD& write_data, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts Read data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const READ_DATA_PAYLOAD& read_data, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

/*
 * Inserts DATA_PAYLOAD short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const DATA_PAYLOAD& data, const STREAM_COLOR color = STREAM_COLOR::YELLOW);

} // namespace debug
} // namespace micrortps
} // namespace eprosima



#endif //_MESSAGE_DEBUGGER_H_
