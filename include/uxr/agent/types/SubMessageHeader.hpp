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

#ifndef _UXR_AGENT_TYPES_SUBMESSAGE_HEADER_HPP_
#define _UXR_AGENT_TYPES_SUBMESSAGE_HEADER_HPP_

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

enum SubmessageHeaderFlags : uint8_t
{
    FLAG_BIG_ENDIANNESS = 0x00,
    FLAG_LITTLE_ENDIANNESS = 0x01 << 0,
    FLAG_REUSE = 0x01 << 1,
    FLAG_REPLACE = 0x01 << 2,
    FLAG_LAST_FRAGMENT = 0x01 << 1,
    FLAG_ECHO = 0x01 << 7,
    FORMAT_DATA_FLAG = 0x00,
    FORMAT_SAMPLE_FLAG = 0x02,
    FORMAT_DATA_SEQ_FLAG = 0x08,
    FORMAT_SAMPLE_SEQ_FLAG = 0x0A,
    FORMAT_PACKED_SAMPLES_FLAG = 0x0E
};

/*!
 * @brief This class represents the structure SubmessageHeader defined by the user in the IDL file.
 * @ingroup SUBMESSAGEHEADER
 */
class SubmessageHeader
{
  public:
    /*!
     * @brief Default constructor.
     */
    SubmessageHeader();

    /*!
     * @brief Default destructor.
     */
    ~SubmessageHeader();

    /*!
     * @brief Copy constructor.
     * @param x Reference to the object dds::xrce::SubmessageHeader that will be copied.
     */
    SubmessageHeader(const SubmessageHeader &x);

    /*!
     * @brief Move constructor.
     * @param x Reference to the object dds::xrce::SubmessageHeader that will be copied.
     */
    SubmessageHeader(SubmessageHeader &&x);

    /*!
     * @brief Copy assignment.
     * @param x Reference to the object dds::xrce::SubmessageHeader that will be copied.
     */
    SubmessageHeader &operator=(const SubmessageHeader &x);

    /*!
     * @brief Move assignment.
     * @param x Reference to the object dds::xrce::SubmessageHeader that will be copied.
     */
    SubmessageHeader &operator=(SubmessageHeader &&x);

    /*!
     * @brief This function sets a value in member submessage_id
     * @param _submessage_id New value for member submessage_id
     */
    inline void submessage_id(dds::xrce::SubmessageId _submessage_id)
    {
        m_submessage_id = _submessage_id;
    }

    /*!
     * @brief This function returns the value of member submessage_id
     * @return Value of member submessage_id
     */
    inline dds::xrce::SubmessageId submessage_id() const
    {
        return m_submessage_id;
    }

    /*!
     * @brief This function returns a reference to member submessage_id
     * @return Reference to member submessage_id
     */
    inline dds::xrce::SubmessageId &submessage_id()
    {
        return m_submessage_id;
    }
    /*!
     * @brief This function sets a value in member flags
     * @param _flags New value for member flags
     */
    inline void flags(uint8_t _flags)
    {
        m_flags = _flags;
    }

    /*!
     * @brief This function returns the value of member flags
     * @return Value of member flags
     */
    inline uint8_t flags() const
    {
        return m_flags;
    }

    /*!
     * @brief This function returns a reference to member flags
     * @return Reference to member flags
     */
    inline uint8_t &flags()
    {
        return m_flags;
    }
    /*!
     * @brief This function sets a value in member submessage_length
     * @param _submessage_length New value for member submessage_length
     */
    inline void submessage_length(uint16_t _submessage_length)
    {
        m_submessage_length = _submessage_length;
    }

    /*!
     * @brief This function returns the value of member submessage_length
     * @return Value of member submessage_length
     */
    inline uint16_t submessage_length() const
    {
        return m_submessage_length;
    }

    /*!
     * @brief This function returns a reference to member submessage_length
     * @return Reference to member submessage_length
     */
    inline uint16_t &submessage_length()
    {
        return m_submessage_length;
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
    virtual void serialize(eprosima::fastcdr::Cdr &cdr) const;

    /*!
     * @brief This function deserializes an object using CDR serialization.
     * @param cdr CDR serialization object.
     */
    virtual void deserialize(eprosima::fastcdr::Cdr &cdr);

  private:
    dds::xrce::SubmessageId m_submessage_id;
    uint8_t m_flags;
    uint16_t m_submessage_length;
};

} // namespace xrce
} // namespace dds

#endif //_UXR_AGENT_TYPES_SUBMESSAGE_HEADER_HPP_
