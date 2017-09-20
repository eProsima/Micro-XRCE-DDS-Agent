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

#ifndef _MESSAGE_HEADER_H
#define _MESSAGE_HEADER_H

#include <stdint.h>
#include <array>
#include <string>
#include <vector>

namespace eprosima
{
    namespace fastcdr
    {
        class Cdr;
    }
}

namespace eprosima {
namespace micrortps {

class MessageHeader
{
public:

    /*!
    * @brief Default constructor.
    */
    MessageHeader();

    /*!
    * @brief Default destructor.
    */
    ~MessageHeader();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object MessageHeader that will be copied.
    */
    MessageHeader(const MessageHeader &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object MessageHeader that will be copied.
    */
    MessageHeader(MessageHeader &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object MessageHeader that will be copied.
    */
    MessageHeader& operator=(const MessageHeader &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object MessageHeader that will be copied.
    */
    MessageHeader& operator=(MessageHeader &&x);

    /*!
    * @brief This function sets a value in member client_key
    * @param _client_key New value for member client_key
    */
    inline void client_key(int32_t _client_key)
    {
        client_key_ = _client_key;
    }

    /*!
    * @brief This function returns the value of member client_key
    * @return Value of member client_key
    */
    inline int32_t client_key() const
    {
        return client_key_;
    }

    /*!
    * @brief This function returns a reference to member client_key
    * @return Reference to member client_key
    */
    inline int32_t& client_key()
    {
        return client_key_;
    }
    /*!
    * @brief This function sets a value in member session_id
    * @param _session_id New value for member session_id
    */
    inline void session_id(uint8_t _session_id)
    {
        session_id_ = _session_id;
    }

    /*!
    * @brief This function returns the value of member session_id
    * @return Value of member session_id
    */
    inline uint8_t session_id() const
    {
        return session_id_;
    }

    /*!
    * @brief This function returns a reference to member session_id
    * @return Reference to member session_id
    */
    inline uint8_t& session_id()
    {
        return session_id_;
    }
    /*!
    * @brief This function sets a value in member stream_id
    * @param _stream_id New value for member stream_id
    */
    inline void stream_id(uint8_t _stream_id)
    {
        stream_id_ = _stream_id;
    }

    /*!
    * @brief This function returns the value of member stream_id
    * @return Value of member stream_id
    */
    inline uint8_t stream_id() const
    {
        return stream_id_;
    }

    /*!
    * @brief This function returns a reference to member stream_id
    * @return Reference to member stream_id
    */
    inline uint8_t& stream_id()
    {
        return stream_id_;
    }
    /*!
    * @brief This function sets a value in member sequence_nr
    * @param _sequence_nr New value for member sequence_nr
    */
    inline void sequence_nr(uint16_t _sequence_nr)
    {
        sequence_nr_ = _sequence_nr;
    }

    /*!
    * @brief This function returns the value of member sequence_nr
    * @return Value of member sequence_nr
    */
    inline uint16_t sequence_nr() const
    {
        return sequence_nr_;
    }

    /*!
    * @brief This function returns a reference to member sequence_nr
    * @return Reference to member sequence_nr
    */
    inline uint16_t& sequence_nr()
    {
        return sequence_nr_;
    }

    /*!
    * @brief This function returns the maximum serialized size of an object
    * depending on the buffer alignment.
    * @param current_alignment Buffer alignment.
    * @return Maximum serialized size.
    */
    static size_t getMaxCdrSerializedSize(size_t current_alignment = 0);

    /*!
    * @brief This function returns the serialized size of a data depending on the buffer alignment.
    * @param data Data which is calculated its serialized size.
    * @param current_alignment Buffer alignment.
    * @return Serialized size.
    */
    static size_t getCdrSerializedSize(const MessageHeader& data, size_t current_alignment = 0);


    /*!
    * @brief This function serializes an object using CDR serialization.
    * @param cdr CDR serialization object.
    */
    void serialize(eprosima::fastcdr::Cdr &cdr) const;

    /*!
    * @brief This function deserializes an object using CDR serialization.
    * @param cdr CDR serialization object.
    */
    void deserialize(eprosima::fastcdr::Cdr &cdr);

private:
    int32_t client_key_;
    uint8_t session_id_;
    uint8_t stream_id_;
    uint16_t sequence_nr_;
};

/*
 * Inserts MessageHeader on the stream.
 */
std::ostream& operator<<(std::ostream& stream, const MessageHeader& header);

} /* namespace micrortps */
} /* namespace eprosima */
#endif // !_MESSAGE_HEADER_H
