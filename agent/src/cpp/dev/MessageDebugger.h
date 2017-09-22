#ifndef _MESSAGE_DEBUGGER_H_
#define _MESSAGE_DEBUGGER_H_

#include <array>
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

namespace debug{

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

template <typename T, size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<T, N>& v);

template <size_t N>
std::ostream& operator<<(std::ostream& stream, const std::array<unsigned char, N>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream << std::noshowbase << "0x" << std::internal << std::setfill('0') << std::hex;
    for (auto& number : values)
    {
        stream << std::setw(2) << +number;
    }
    return stream;
}

std::ostream& operator<<(std::ostream& stream, const std::vector<unsigned char>& values)
{
    StreamScopedFlags flag_backup{stream};
    stream << std::noshowbase << "0x" << std::internal << std::setfill('0') << std::hex;
    for (auto& number : values)
    {
        stream << std::setw(2) << +number;
    }
    return stream;
}

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
std::ostream& short_print(std::ostream& stream, const Status& status);

/*
 * Inserts ResultStatus short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const ResultStatus& status);

/*
 * Inserts create payload on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const CREATE_PAYLOAD& create_payload);

/*
* Inserts create short representation on the stream.
*/
std::ostream& short_print(std::ostream& stream, const CREATE_PAYLOAD& create_payload);

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
std::ostream& short_print(std::ostream& stream, const DELETE_PAYLOAD& delete_data);

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
std::ostream& short_print(std::ostream& stream, const WRITE_DATA_PAYLOAD& write_data);

/*
 * Inserts Read data short representation on the stream.
 */
std::ostream& short_print(std::ostream& stream, const READ_DATA_PAYLOAD& read_data);

    // void print_delete_submessage(const DeletePayloadSpec* payload);
    // void print_write_data_submessage(const WriteDataPayloadSpec* payload);
    // void print_read_data_submessage(const ReadDataPayloadSpec* payload);
    // void print_data_submessage(const DataPayloadSpec* payload);
    


    // void printl_read_data_submessage(const ReadDataPayloadSpec* payload);
    // void printl_data_submessage(const DataPayloadSpec* payload);
    // //UTIL
    // const char* data_to_string(const uint8_t* data, uint32_t size);

} // namespace debug
} // namespace micrortps
} // namespace eprosima



#endif //_MESSAGE_DEBUGGER_H_
