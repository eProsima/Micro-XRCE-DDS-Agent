// Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _UXR_AGENT_TYPES_MESSAGE_HEADER_HPP_
#define _UXR_AGENT_TYPES_MESSAGE_HEADER_HPP_

#include <uxr/agent/types/XRCETypes.hpp>
#include <cstdint>
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

namespace dds {
namespace xrce {
/*!
 * @brief This class represents the structure MessageHeader defined by the user in the IDL file.
 * @ingroup MESSAGEHEADER
 */
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
     * @param x Reference to the object dds::xrce::MessageHeader that will be copied.
     */
    MessageHeader(const MessageHeader &x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object dds::xrce::MessageHeader that will be copied.
     */
    MessageHeader(MessageHeader &&x);

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object dds::xrce::MessageHeader that will be copied.
     */
    MessageHeader &operator=(const MessageHeader &x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object dds::xrce::MessageHeader that will be copied.
     */
    MessageHeader &operator=(MessageHeader &&x);

    /*!
     * @brief This function sets a value in member session_id
     * @param _session_id New value for member session_id
     */
    inline void session_id(uint8_t _session_id)
    {
        m_session_id = _session_id;
    }

    /*!
     * @brief This function returns the value of member session_id
     * @return Value of member session_id
     */
    inline uint8_t session_id() const
    {
        return m_session_id;
    }

    /*!
     * @brief This function returns a reference to member session_id
     * @return Reference to member session_id
     */
    inline uint8_t &session_id()
    {
        return m_session_id;
    }
    /*!
     * @brief This function sets a value in member stream_id
     * @param _stream_id New value for member stream_id
     */
    inline void stream_id(uint8_t _stream_id)
    {
        m_stream_id = _stream_id;
    }

    /*!
     * @brief This function returns the value of member stream_id
     * @return Value of member stream_id
     */
    inline uint8_t stream_id() const
    {
        return m_stream_id;
    }

    /*!
     * @brief This function returns a reference to member stream_id
     * @return Reference to member stream_id
     */
    inline uint8_t &stream_id()
    {
        return m_stream_id;
    }
    /*!
     * @brief This function sets a value in member sequence_nr
     * @param _sequence_nr New value for member sequence_nr
     */
    inline void sequence_nr(uint16_t _sequence_nr)
    {
        m_sequence_nr = _sequence_nr;
    }

    /*!
     * @brief This function returns the value of member sequence_nr
     * @return Value of member sequence_nr
     */
    inline uint16_t sequence_nr() const
    {
        return m_sequence_nr;
    }

    /*!
     * @brief This function returns a reference to member sequence_nr
     * @return Reference to member sequence_nr
     */
    inline uint16_t &sequence_nr()
    {
        return m_sequence_nr;
    }
    /*!
     * @brief This function copies the value in member client_key
     * @param _client_key New value to be copied in member client_key
     */
    inline void client_key(const ClientKey &_client_key)
    {
        m_client_key = _client_key;
    }

    /*!
     * @brief This function moves the value in member client_key
     * @param _client_key New value to be moved in member client_key
     */
    inline void client_key(ClientKey &&_client_key)
    {
        m_client_key = std::move(_client_key);
    }

    /*!
     * @brief This function returns a constant reference to member client_key
     * @return Constant reference to member client_key
     */
    inline const ClientKey &client_key() const
    {
        return m_client_key;
    }

    /*!
     * @brief This function returns a reference to member client_key
     * @return Reference to member client_key
     */
    inline ClientKey &client_key()
    {
        return m_client_key;
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
    size_t getCdrSerializedSize(size_t current_alignment = 0) const;

    /*!
     * @brief This function serializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    virtual void serialize(eprosima::fastcdr::Cdr &scdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    virtual void deserialize(eprosima::fastcdr::Cdr &dcdr);

  private:
    uint8_t m_session_id;
    uint8_t m_stream_id;
    uint16_t m_sequence_nr;
    ClientKey m_client_key;
};

} // namespace xrce
} // namespace dds

#endif //_UXR_AGENT_TYPES_MESSAGE_HEADER_HPP_
