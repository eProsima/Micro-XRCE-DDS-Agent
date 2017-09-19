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

#ifndef _ObjectVariant_H_
#define _ObjectVariant_H_

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

typedef uint8_t subMessageId;
const subMessageId CREATE = 0x01;
const subMessageId GET_INFO = 0x02;
const subMessageId DELETE = 0x03;
const subMessageId STATUS = 0x04;
const subMessageId INFO = 0x05;
const subMessageId WRITE_DATA = 0x06;
const subMessageId READ_DATA = 0x07;
const subMessageId DATA = 0x08;
const subMessageId ACKNACK = 0x09;
const subMessageId HEARTBEAT = 0x10;
const subMessageId RESET = 0x11;
const subMessageId FRAGMENT = 0x12;
const subMessageId FRAGMENT_END = 0x13;

typedef std::array<uint8_t, 4> ClientKey;

typedef uint8_t ObjectKind;
const ObjectKind OBJK_INVALID = 0x0;
const ObjectKind OBJK_PARTICIPANT = 0x1;
const ObjectKind OBJK_TOPIC = 0x4;
const ObjectKind OBJK_DATAWRITER = 0x3;
const ObjectKind OBJK_DATAREADER = 0x7;
const ObjectKind OBJK_SUBSCRIBER = 0x8;
const ObjectKind OBJK_PUBLISHER = 0x9;
const ObjectKind OBJK_TYPE = 0x10;
const ObjectKind OBJK_QOSPROFILE = 0x11;
const ObjectKind OBJK_APPLICATION = 0x20;
const ObjectKind OBJK_CLIENT = 0x21;

typedef std::array<uint8_t, 3> ObjectId;

#define  OBJECTID_INVALID {0xFF,0xFF,0xFF}
#define  OBJECTID_CLIENT  {0xFF,0xFF,0xF0}
#define  OBJECTID_SESSION {0xFF,0xFF,0xF1}

// Spells XRCE
#define XRCE_COOKIE { 0x58, 0x52, 0x43, 0x45 }
#define XRCE_VERSION_MAJOR     0x01
#define XRCE_VERSION_MINOR     0x00
#define XRCE_VERSION  { XRCE_VERSION_MAJOR, XRCE_VERSION_MINOR }

typedef uint8_t SessionId;
const uint8_t SESSIONID_INVALID = 0;

typedef std::array<uint8_t, 4> RequestId;
const uint8_t STATUS_OK = 0x00;
const uint8_t STATUS_OK_MATCHED = 0x01;
const uint8_t STATUS_ERR_DDS_ERROR = 0x80;
const uint8_t STATUS_ERR_MISMATCH = 0x81;
const uint8_t STATUS_ERR_ALREADY_EXISTS = 0x82;
const uint8_t STATUS_ERR_DENIED = 0x83;
const uint8_t STATUS_ERR_UNKNOWN_REFERENCE = 0x84;
const uint8_t STATUS_ERR_INVALID_DATA = 0x85;
const uint8_t STATUS_ERR_INCOMPATIBLE = 0x86;
const uint8_t STATUS_ERR_RESOURCES = 0x87;

const uint8_t STATUS_LAST_OP_NONE = 0;
const uint8_t STATUS_LAST_OP_CREATE = 1;
const uint8_t STATUS_LAST_OP_UPDATE = 2;
const uint8_t STATUS_LAST_OP_DELETE = 3;
const uint8_t STATUS_LAST_OP_LOOKUP = 4;
const uint8_t STATUS_LAST_OP_READ = 5;
const uint8_t STATUS_LAST_OP_WRITE = 6;
/*!
 * @brief This class represents the structure Time_t defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class Time_t
{
public:

    /*!
     * @brief Default constructor.
     */
    Time_t();
    
    /*!
     * @brief Default destructor.
     */
    ~Time_t();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Time_t that will be copied.
     */
    Time_t(const Time_t &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object Time_t that will be copied.
     */
    Time_t(Time_t &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Time_t that will be copied.
     */
    Time_t& operator=(const Time_t &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object Time_t that will be copied.
     */
    Time_t& operator=(Time_t &&x);
    
    /*!
     * @brief This function sets a value in member seconds
     * @param _seconds New value for member seconds
     */
    inline void seconds(int32_t _seconds)
    {
        seconds_ = _seconds;
    }

    /*!
     * @brief This function returns the value of member seconds
     * @return Value of member seconds
     */
    inline int32_t seconds() const
    {
        return seconds_;
    }

    /*!
     * @brief This function returns a reference to member seconds
     * @return Reference to member seconds
     */
    inline int32_t& seconds()
    {
        return seconds_;
    }
    /*!
     * @brief This function sets a value in member nanoseconds
     * @param _nanoseconds New value for member nanoseconds
     */
    inline void nanoseconds(uint32_t _nanoseconds)
    {
        nanoseconds_ = _nanoseconds;
    }

    /*!
     * @brief This function returns the value of member nanoseconds
     * @return Value of member nanoseconds
     */
    inline uint32_t nanoseconds() const
    {
        return nanoseconds_;
    }

    /*!
     * @brief This function returns a reference to member nanoseconds
     * @return Reference to member nanoseconds
     */
    inline uint32_t& nanoseconds()
    {
        return nanoseconds_;
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
    static size_t getCdrSerializedSize(const Time_t& data, size_t current_alignment = 0);


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
    int32_t seconds_;
    uint32_t nanoseconds_;
};

/*!
 * @brief This class represents the structure OBJK_CLIENT_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_CLIENT_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_CLIENT_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_CLIENT_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_CLIENT_Representation that will be copied.
     */
    OBJK_CLIENT_Representation(const OBJK_CLIENT_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_CLIENT_Representation that will be copied.
     */
    OBJK_CLIENT_Representation(OBJK_CLIENT_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_CLIENT_Representation that will be copied.
     */
    OBJK_CLIENT_Representation& operator=(const OBJK_CLIENT_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_CLIENT_Representation that will be copied.
     */
    OBJK_CLIENT_Representation& operator=(OBJK_CLIENT_Representation &&x);
    
    /*!
     * @brief This function copies the value in member xrce_cookie
     * @param _xrce_cookie New value to be copied in member xrce_cookie
     */
    inline void xrce_cookie(const std::array<uint8_t, 4> &_xrce_cookie)
    {
        xrce_cookie_ = _xrce_cookie;
    }

    /*!
     * @brief This function moves the value in member xrce_cookie
     * @param _xrce_cookie New value to be moved in member xrce_cookie
     */
    inline void xrce_cookie(std::array<uint8_t, 4> &&_xrce_cookie)
    {
        xrce_cookie_ = std::move(_xrce_cookie);
    }

    /*!
     * @brief This function returns a constant reference to member xrce_cookie
     * @return Constant reference to member xrce_cookie
     */
    inline const std::array<uint8_t, 4>& xrce_cookie() const
    {
        return xrce_cookie_;
    }

    /*!
     * @brief This function returns a reference to member xrce_cookie
     * @return Reference to member xrce_cookie
     */
    inline std::array<uint8_t, 4>& xrce_cookie()
    {
        return xrce_cookie_;
    }
    /*!
     * @brief This function copies the value in member xrce_version
     * @param _xrce_version New value to be copied in member xrce_version
     */
    inline void xrce_version(const std::array<uint8_t, 2> &_xrce_version)
    {
        xrce_version_ = _xrce_version;
    }

    /*!
     * @brief This function moves the value in member xrce_version
     * @param _xrce_version New value to be moved in member xrce_version
     */
    inline void xrce_version(std::array<uint8_t, 2> &&_xrce_version)
    {
        xrce_version_ = std::move(_xrce_version);
    }

    /*!
     * @brief This function returns a constant reference to member xrce_version
     * @return Constant reference to member xrce_version
     */
    inline const std::array<uint8_t, 2>& xrce_version() const
    {
        return xrce_version_;
    }

    /*!
     * @brief This function returns a reference to member xrce_version
     * @return Reference to member xrce_version
     */
    inline std::array<uint8_t, 2>& xrce_version()
    {
        return xrce_version_;
    }
    /*!
     * @brief This function copies the value in member xrce_vendor_id
     * @param _xrce_vendor_id New value to be copied in member xrce_vendor_id
     */
    inline void xrce_vendor_id(const std::array<uint8_t, 2> &_xrce_vendor_id)
    {
        xrce_vendor_id_ = _xrce_vendor_id;
    }

    /*!
     * @brief This function moves the value in member xrce_vendor_id
     * @param _xrce_vendor_id New value to be moved in member xrce_vendor_id
     */
    inline void xrce_vendor_id(std::array<uint8_t, 2> &&_xrce_vendor_id)
    {
        xrce_vendor_id_ = std::move(_xrce_vendor_id);
    }

    /*!
     * @brief This function returns a constant reference to member xrce_vendor_id
     * @return Constant reference to member xrce_vendor_id
     */
    inline const std::array<uint8_t, 2>& xrce_vendor_id() const
    {
        return xrce_vendor_id_;
    }

    /*!
     * @brief This function returns a reference to member xrce_vendor_id
     * @return Reference to member xrce_vendor_id
     */
    inline std::array<uint8_t, 2>& xrce_vendor_id()
    {
        return xrce_vendor_id_;
    }
    /*!
     * @brief This function copies the value in member client_timestamp
     * @param _client_timestamp New value to be copied in member client_timestamp
     */
    inline void client_timestamp(const Time_t &_client_timestamp)
    {
        client_timestamp_ = _client_timestamp;
    }

    /*!
     * @brief This function moves the value in member client_timestamp
     * @param _client_timestamp New value to be moved in member client_timestamp
     */
    inline void client_timestamp(Time_t &&_client_timestamp)
    {
        client_timestamp_ = std::move(_client_timestamp);
    }

    /*!
     * @brief This function returns a constant reference to member client_timestamp
     * @return Constant reference to member client_timestamp
     */
    inline const Time_t& client_timestamp() const
    {
        return client_timestamp_;
    }

    /*!
     * @brief This function returns a reference to member client_timestamp
     * @return Reference to member client_timestamp
     */
    inline Time_t& client_timestamp()
    {
        return client_timestamp_;
    }
    /*!
     * @brief This function sets a value in member session_id
     * @param _session_id New value for member session_id
     */
    inline void session_id(SessionId _session_id)
    {
        session_id_ = _session_id;
    }

    /*!
     * @brief This function returns the value of member session_id
     * @return Value of member session_id
     */
    inline SessionId session_id() const
    {
        return session_id_;
    }

    /*!
     * @brief This function returns a reference to member session_id
     * @return Reference to member session_id
     */
    inline SessionId& session_id()
    {
        return session_id_;
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
    static size_t getCdrSerializedSize(const OBJK_CLIENT_Representation& data, size_t current_alignment = 0);


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
    std::array<uint8_t, 4> xrce_cookie_;
    std::array<uint8_t, 2> xrce_version_;
    std::array<uint8_t, 2> xrce_vendor_id_;
    Time_t client_timestamp_;
    SessionId session_id_;
};
/*!
 * @brief This class represents the structure OBJK_CommonString_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_CommonString_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_CommonString_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_CommonString_Representation that will be copied.
     */
    OBJK_CommonString_Representation(const OBJK_CommonString_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_CommonString_Representation that will be copied.
     */
    OBJK_CommonString_Representation(OBJK_CommonString_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_CommonString_Representation that will be copied.
     */
    OBJK_CommonString_Representation& operator=(const OBJK_CommonString_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_CommonString_Representation that will be copied.
     */
    OBJK_CommonString_Representation& operator=(OBJK_CommonString_Representation &&x);
    
    /*!
     * @brief This function copies the value in member as_string
     * @param _as_string New value to be copied in member as_string
     */
    inline void as_string(const std::string &_as_string)
    {
        as_string_ = _as_string;
    }

    /*!
     * @brief This function moves the value in member as_string
     * @param _as_string New value to be moved in member as_string
     */
    inline void as_string(std::string &&_as_string)
    {
        as_string_ = std::move(_as_string);
    }

    /*!
     * @brief This function returns a constant reference to member as_string
     * @return Constant reference to member as_string
     */
    inline const std::string& as_string() const
    {
        return as_string_;
    }

    /*!
     * @brief This function returns a reference to member as_string
     * @return Reference to member as_string
     */
    inline std::string& as_string()
    {
        return as_string_;
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
    static size_t getCdrSerializedSize(const OBJK_CommonString_Representation& data, size_t current_alignment = 0);


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
    std::string as_string_;
};
/*!
 * @brief This class represents the structure OBJK_QOSPROFILE_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_QOSPROFILE_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_QOSPROFILE_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_QOSPROFILE_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_QOSPROFILE_Representation that will be copied.
     */
    OBJK_QOSPROFILE_Representation(const OBJK_QOSPROFILE_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_QOSPROFILE_Representation that will be copied.
     */
    OBJK_QOSPROFILE_Representation(OBJK_QOSPROFILE_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_QOSPROFILE_Representation that will be copied.
     */
    OBJK_QOSPROFILE_Representation& operator=(const OBJK_QOSPROFILE_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_QOSPROFILE_Representation that will be copied.
     */
    OBJK_QOSPROFILE_Representation& operator=(OBJK_QOSPROFILE_Representation &&x);
    

    
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
    static size_t getCdrSerializedSize(const OBJK_QOSPROFILE_Representation& data, size_t current_alignment = 0);

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
};
/*!
 * @brief This class represents the structure OBJK_APPLICATION_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_APPLICATION_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_APPLICATION_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_APPLICATION_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_APPLICATION_Representation that will be copied.
     */
    OBJK_APPLICATION_Representation(const OBJK_APPLICATION_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_APPLICATION_Representation that will be copied.
     */
    OBJK_APPLICATION_Representation(OBJK_APPLICATION_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_APPLICATION_Representation that will be copied.
     */
    OBJK_APPLICATION_Representation& operator=(const OBJK_APPLICATION_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_APPLICATION_Representation that will be copied.
     */
    OBJK_APPLICATION_Representation& operator=(OBJK_APPLICATION_Representation &&x);
    
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
    static size_t getCdrSerializedSize(const OBJK_APPLICATION_Representation& data, size_t current_alignment = 0);


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
};
/*!
 * @brief This class represents the structure OBJK_PARTICIPANT_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_PARTICIPANT_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_PARTICIPANT_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_PARTICIPANT_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_PARTICIPANT_Representation that will be copied.
     */
    OBJK_PARTICIPANT_Representation(const OBJK_PARTICIPANT_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_PARTICIPANT_Representation that will be copied.
     */
    OBJK_PARTICIPANT_Representation(OBJK_PARTICIPANT_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_PARTICIPANT_Representation that will be copied.
     */
    OBJK_PARTICIPANT_Representation& operator=(const OBJK_PARTICIPANT_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_PARTICIPANT_Representation that will be copied.
     */
    OBJK_PARTICIPANT_Representation& operator=(OBJK_PARTICIPANT_Representation &&x);

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
    static size_t getCdrSerializedSize(const OBJK_PARTICIPANT_Representation& data, size_t current_alignment = 0);


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
};
/*!
 * @brief This class represents the structure OBJK_TYPE_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_TYPE_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_TYPE_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_TYPE_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_TYPE_Representation that will be copied.
     */
    OBJK_TYPE_Representation(const OBJK_TYPE_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_TYPE_Representation that will be copied.
     */
    OBJK_TYPE_Representation(OBJK_TYPE_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_TYPE_Representation that will be copied.
     */
    OBJK_TYPE_Representation& operator=(const OBJK_TYPE_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_TYPE_Representation that will be copied.
     */
    OBJK_TYPE_Representation& operator=(OBJK_TYPE_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
    }
    /*!
     * @brief This function copies the value in member registered_type_name
     * @param _registered_type_name New value to be copied in member registered_type_name
     */
    inline void registered_type_name(const std::string &_registered_type_name)
    {
        registered_type_name_ = _registered_type_name;
    }

    /*!
     * @brief This function moves the value in member registered_type_name
     * @param _registered_type_name New value to be moved in member registered_type_name
     */
    inline void registered_type_name(std::string &&_registered_type_name)
    {
        registered_type_name_ = std::move(_registered_type_name);
    }

    /*!
     * @brief This function returns a constant reference to member registered_type_name
     * @return Constant reference to member registered_type_name
     */
    inline const std::string& registered_type_name() const
    {
        return registered_type_name_;
    }

    /*!
     * @brief This function returns a reference to member registered_type_name
     * @return Reference to member registered_type_name
     */
    inline std::string& registered_type_name()
    {
        return registered_type_name_;
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
    static size_t getCdrSerializedSize(const OBJK_TYPE_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
    std::string registered_type_name_;
};
/*!
 * @brief This class represents the structure OBJK_TOPIC_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_TOPIC_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_TOPIC_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_TOPIC_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_TOPIC_Representation that will be copied.
     */
    OBJK_TOPIC_Representation(const OBJK_TOPIC_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_TOPIC_Representation that will be copied.
     */
    OBJK_TOPIC_Representation(OBJK_TOPIC_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_TOPIC_Representation that will be copied.
     */
    OBJK_TOPIC_Representation& operator=(const OBJK_TOPIC_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_TOPIC_Representation that will be copied.
     */
    OBJK_TOPIC_Representation& operator=(OBJK_TOPIC_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
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
    static size_t getCdrSerializedSize(const OBJK_TOPIC_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
};
/*!
 * @brief This class represents the structure OBJK_PUBLISHER_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_PUBLISHER_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_PUBLISHER_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_PUBLISHER_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_PUBLISHER_Representation that will be copied.
     */
    OBJK_PUBLISHER_Representation(const OBJK_PUBLISHER_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_PUBLISHER_Representation that will be copied.
     */
    OBJK_PUBLISHER_Representation(OBJK_PUBLISHER_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_PUBLISHER_Representation that will be copied.
     */
    OBJK_PUBLISHER_Representation& operator=(const OBJK_PUBLISHER_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_PUBLISHER_Representation that will be copied.
     */
    OBJK_PUBLISHER_Representation& operator=(OBJK_PUBLISHER_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
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
    static size_t getCdrSerializedSize(const OBJK_PUBLISHER_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
};
/*!
 * @brief This class represents the structure OBJK_SUBSCRIBER_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_SUBSCRIBER_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_SUBSCRIBER_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_SUBSCRIBER_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_SUBSCRIBER_Representation that will be copied.
     */
    OBJK_SUBSCRIBER_Representation(const OBJK_SUBSCRIBER_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_SUBSCRIBER_Representation that will be copied.
     */
    OBJK_SUBSCRIBER_Representation(OBJK_SUBSCRIBER_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_SUBSCRIBER_Representation that will be copied.
     */
    OBJK_SUBSCRIBER_Representation& operator=(const OBJK_SUBSCRIBER_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_SUBSCRIBER_Representation that will be copied.
     */
    OBJK_SUBSCRIBER_Representation& operator=(OBJK_SUBSCRIBER_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
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
    static size_t getCdrSerializedSize(const OBJK_SUBSCRIBER_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
};
/*!
 * @brief This class represents the structure OBJK_DATAWRITER_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_DATAWRITER_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_DATAWRITER_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_DATAWRITER_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_DATAWRITER_Representation that will be copied.
     */
    OBJK_DATAWRITER_Representation(const OBJK_DATAWRITER_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_DATAWRITER_Representation that will be copied.
     */
    OBJK_DATAWRITER_Representation(OBJK_DATAWRITER_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_DATAWRITER_Representation that will be copied.
     */
    OBJK_DATAWRITER_Representation& operator=(const OBJK_DATAWRITER_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_DATAWRITER_Representation that will be copied.
     */
    OBJK_DATAWRITER_Representation& operator=(OBJK_DATAWRITER_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
    }
    /*!
     * @brief This function copies the value in member publisher_id
     * @param _publisher_id New value to be copied in member publisher_id
     */
    inline void publisher_id(const ObjectId &_publisher_id)
    {
        publisher_id_ = _publisher_id;
    }

    /*!
     * @brief This function moves the value in member publisher_id
     * @param _publisher_id New value to be moved in member publisher_id
     */
    inline void publisher_id(ObjectId &&_publisher_id)
    {
        publisher_id_ = std::move(_publisher_id);
    }

    /*!
     * @brief This function returns a constant reference to member publisher_id
     * @return Constant reference to member publisher_id
     */
    inline const ObjectId& publisher_id() const
    {
        return publisher_id_;
    }

    /*!
     * @brief This function returns a reference to member publisher_id
     * @return Reference to member publisher_id
     */
    inline ObjectId& publisher_id()
    {
        return publisher_id_;
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
    static size_t getCdrSerializedSize(const OBJK_DATAWRITER_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
    ObjectId publisher_id_;
};
/*!
 * @brief This class represents the structure OBJK_DATAREADER_Representation defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_DATAREADER_Representation : public OBJK_CommonString_Representation
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_DATAREADER_Representation();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_DATAREADER_Representation();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_DATAREADER_Representation that will be copied.
     */
    OBJK_DATAREADER_Representation(const OBJK_DATAREADER_Representation &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_DATAREADER_Representation that will be copied.
     */
    OBJK_DATAREADER_Representation(OBJK_DATAREADER_Representation &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_DATAREADER_Representation that will be copied.
     */
    OBJK_DATAREADER_Representation& operator=(const OBJK_DATAREADER_Representation &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_DATAREADER_Representation that will be copied.
     */
    OBJK_DATAREADER_Representation& operator=(OBJK_DATAREADER_Representation &&x);
    
    /*!
     * @brief This function copies the value in member participant_id
     * @param _participant_id New value to be copied in member participant_id
     */
    inline void participant_id(const ObjectId &_participant_id)
    {
        participant_id_ = _participant_id;
    }

    /*!
     * @brief This function moves the value in member participant_id
     * @param _participant_id New value to be moved in member participant_id
     */
    inline void participant_id(ObjectId &&_participant_id)
    {
        participant_id_ = std::move(_participant_id);
    }

    /*!
     * @brief This function returns a constant reference to member participant_id
     * @return Constant reference to member participant_id
     */
    inline const ObjectId& participant_id() const
    {
        return participant_id_;
    }

    /*!
     * @brief This function returns a reference to member participant_id
     * @return Reference to member participant_id
     */
    inline ObjectId& participant_id()
    {
        return participant_id_;
    }
    /*!
     * @brief This function copies the value in member subscriber_id
     * @param _subscriber_id New value to be copied in member subscriber_id
     */
    inline void subscriber_id(const ObjectId &_subscriber_id)
    {
        subscriber_id_ = _subscriber_id;
    }

    /*!
     * @brief This function moves the value in member subscriber_id
     * @param _subscriber_id New value to be moved in member subscriber_id
     */
    inline void subscriber_id(ObjectId &&_subscriber_id)
    {
        subscriber_id_ = std::move(_subscriber_id);
    }

    /*!
     * @brief This function returns a constant reference to member subscriber_id
     * @return Constant reference to member subscriber_id
     */
    inline const ObjectId& subscriber_id() const
    {
        return subscriber_id_;
    }

    /*!
     * @brief This function returns a reference to member subscriber_id
     * @return Reference to member subscriber_id
     */
    inline ObjectId& subscriber_id()
    {
        return subscriber_id_;
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
    static size_t getCdrSerializedSize(const OBJK_DATAREADER_Representation& data, size_t current_alignment = 0);


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
    ObjectId participant_id_;
    ObjectId subscriber_id_;
};
/*!
 * @brief This class represents the union ObjectVariant defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class ObjectVariant
{
public:

    /*!
     * @brief Default constructor.
     */
    ObjectVariant();
    
    /*!
     * @brief Default destructor.
     */
    ~ObjectVariant();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object ObjectVariant that will be copied.
     */
    ObjectVariant(const ObjectVariant &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object ObjectVariant that will be copied.
     */
    ObjectVariant(ObjectVariant &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object ObjectVariant that will be copied.
     */
    ObjectVariant& operator=(const ObjectVariant &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object ObjectVariant that will be copied.
     */
    ObjectVariant& operator=(ObjectVariant &&x);
    
    /*!
     * @brief This function sets the discriminator value.
     * @param _discriminator New value for the discriminator.
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    void discriminator(ObjectKind _discriminator);
    
    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    ObjectKind discriminator() const;
    
    /*!
     * @brief This function returns a reference to the discriminator.
     * @return Reference to the discriminator.
     */
    ObjectKind& discriminator();
    
    /*!
     * @brief This function copies the value in member client
     * @param _client New value to be copied in member client
     */
    void client(const OBJK_CLIENT_Representation &_client);

    /*!
     * @brief This function moves the value in member client
     * @param _client New value to be moved in member client
     */
    void client(OBJK_CLIENT_Representation &&_client);

    /*!
     * @brief This function returns a constant reference to member client
     * @return Constant reference to member client
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_CLIENT_Representation& client() const;

    /*!
     * @brief This function returns a reference to member client
     * @return Reference to member client
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_CLIENT_Representation& client();
    /*!
     * @brief This function copies the value in member application
     * @param _application New value to be copied in member application
     */
    void application(const OBJK_APPLICATION_Representation &_application);

    /*!
     * @brief This function moves the value in member application
     * @param _application New value to be moved in member application
     */
    void application(OBJK_APPLICATION_Representation &&_application);

    /*!
     * @brief This function returns a constant reference to member application
     * @return Constant reference to member application
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_APPLICATION_Representation& application() const;

    /*!
     * @brief This function returns a reference to member application
     * @return Reference to member application
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_APPLICATION_Representation& application();
    /*!
     * @brief This function copies the value in member participant
     * @param _participant New value to be copied in member participant
     */
    void participant(const OBJK_PARTICIPANT_Representation &_participant);

    /*!
     * @brief This function moves the value in member participant
     * @param _participant New value to be moved in member participant
     */
    void participant(OBJK_PARTICIPANT_Representation &&_participant);

    /*!
     * @brief This function returns a constant reference to member participant
     * @return Constant reference to member participant
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_PARTICIPANT_Representation& participant() const;

    /*!
     * @brief This function returns a reference to member participant
     * @return Reference to member participant
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_PARTICIPANT_Representation& participant();
    /*!
     * @brief This function copies the value in member qos_profile
     * @param _qos_profile New value to be copied in member qos_profile
     */
    void qos_profile(const OBJK_QOSPROFILE_Representation &_qos_profile);

    /*!
     * @brief This function moves the value in member qos_profile
     * @param _qos_profile New value to be moved in member qos_profile
     */
    void qos_profile(OBJK_QOSPROFILE_Representation &&_qos_profile);

    /*!
     * @brief This function returns a constant reference to member qos_profile
     * @return Constant reference to member qos_profile
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_QOSPROFILE_Representation& qos_profile() const;

    /*!
     * @brief This function returns a reference to member qos_profile
     * @return Reference to member qos_profile
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_QOSPROFILE_Representation& qos_profile();
    /*!
     * @brief This function copies the value in member type
     * @param _type New value to be copied in member type
     */
    void type(const OBJK_TYPE_Representation &_type);

    /*!
     * @brief This function moves the value in member type
     * @param _type New value to be moved in member type
     */
    void type(OBJK_TYPE_Representation &&_type);

    /*!
     * @brief This function returns a constant reference to member type
     * @return Constant reference to member type
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_TYPE_Representation& type() const;

    /*!
     * @brief This function returns a reference to member type
     * @return Reference to member type
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_TYPE_Representation& type();
    /*!
     * @brief This function copies the value in member topic
     * @param _topic New value to be copied in member topic
     */
    void topic(const OBJK_TOPIC_Representation &_topic);

    /*!
     * @brief This function moves the value in member topic
     * @param _topic New value to be moved in member topic
     */
    void topic(OBJK_TOPIC_Representation &&_topic);

    /*!
     * @brief This function returns a constant reference to member topic
     * @return Constant reference to member topic
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_TOPIC_Representation& topic() const;

    /*!
     * @brief This function returns a reference to member topic
     * @return Reference to member topic
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_TOPIC_Representation& topic();
    /*!
     * @brief This function copies the value in member publisher
     * @param _publisher New value to be copied in member publisher
     */
    void publisher(const OBJK_PUBLISHER_Representation &_publisher);

    /*!
     * @brief This function moves the value in member publisher
     * @param _publisher New value to be moved in member publisher
     */
    void publisher(OBJK_PUBLISHER_Representation &&_publisher);

    /*!
     * @brief This function returns a constant reference to member publisher
     * @return Constant reference to member publisher
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_PUBLISHER_Representation& publisher() const;

    /*!
     * @brief This function returns a reference to member publisher
     * @return Reference to member publisher
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_PUBLISHER_Representation& publisher();
    /*!
     * @brief This function copies the value in member subscriber
     * @param _subscriber New value to be copied in member subscriber
     */
    void subscriber(const OBJK_SUBSCRIBER_Representation &_subscriber);

    /*!
     * @brief This function moves the value in member subscriber
     * @param _subscriber New value to be moved in member subscriber
     */
    void subscriber(OBJK_SUBSCRIBER_Representation &&_subscriber);

    /*!
     * @brief This function returns a constant reference to member subscriber
     * @return Constant reference to member subscriber
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_SUBSCRIBER_Representation& subscriber() const;

    /*!
     * @brief This function returns a reference to member subscriber
     * @return Reference to member subscriber
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_SUBSCRIBER_Representation& subscriber();
    /*!
     * @brief This function copies the value in member data_writer
     * @param _data_writer New value to be copied in member data_writer
     */
    void data_writer(const OBJK_DATAWRITER_Representation &_data_writer);

    /*!
     * @brief This function moves the value in member data_writer
     * @param _data_writer New value to be moved in member data_writer
     */
    void data_writer(OBJK_DATAWRITER_Representation &&_data_writer);

    /*!
     * @brief This function returns a constant reference to member data_writer
     * @return Constant reference to member data_writer
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_DATAWRITER_Representation& data_writer() const;

    /*!
     * @brief This function returns a reference to member data_writer
     * @return Reference to member data_writer
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_DATAWRITER_Representation& data_writer();
    /*!
     * @brief This function copies the value in member data_reader
     * @param _data_reader New value to be copied in member data_reader
     */
    void data_reader(const OBJK_DATAREADER_Representation &_data_reader);

    /*!
     * @brief This function moves the value in member data_reader
     * @param _data_reader New value to be moved in member data_reader
     */
    void data_reader(OBJK_DATAREADER_Representation &&_data_reader);

    /*!
     * @brief This function returns a constant reference to member data_reader
     * @return Constant reference to member data_reader
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_DATAREADER_Representation& data_reader() const;

    /*!
     * @brief This function returns a reference to member data_reader
     * @return Reference to member data_reader
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_DATAREADER_Representation& data_reader();
    
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
    static size_t getCdrSerializedSize(const ObjectVariant& data, size_t current_alignment = 0);


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
    ObjectKind discriminator_;
    
    OBJK_CLIENT_Representation client_;
    OBJK_APPLICATION_Representation application_;
    OBJK_PARTICIPANT_Representation participant_;
    OBJK_QOSPROFILE_Representation qos_profile_;
    OBJK_TYPE_Representation type_;
    OBJK_TOPIC_Representation topic_;
    OBJK_PUBLISHER_Representation publisher_;
    OBJK_SUBSCRIBER_Representation subscriber_;
    OBJK_DATAWRITER_Representation data_writer_;
    OBJK_DATAREADER_Representation data_reader_;
};
/*!
 * @brief This class represents the structure CreationMode defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class CreationMode
{
public:

    /*!
     * @brief Default constructor.
     */
    CreationMode();
    
    /*!
     * @brief Default destructor.
     */
    ~CreationMode();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object CreationMode that will be copied.
     */
    CreationMode(const CreationMode &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object CreationMode that will be copied.
     */
    CreationMode(CreationMode &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object CreationMode that will be copied.
     */
    CreationMode& operator=(const CreationMode &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object CreationMode that will be copied.
     */
    CreationMode& operator=(CreationMode &&x);
    
    /*!
     * @brief This function sets a value in member reuse
     * @param _reuse New value for member reuse
     */
    inline void reuse(bool _reuse)
    {
        reuse_ = _reuse;
    }

    /*!
     * @brief This function returns the value of member reuse
     * @return Value of member reuse
     */
    inline bool reuse() const
    {
        return reuse_;
    }

    /*!
     * @brief This function returns a reference to member reuse
     * @return Reference to member reuse
     */
    inline bool& reuse()
    {
        return reuse_;
    }
    /*!
     * @brief This function sets a value in member replace
     * @param _replace New value for member replace
     */
    inline void replace(bool _replace)
    {
        replace_ = _replace;
    }

    /*!
     * @brief This function returns the value of member replace
     * @return Value of member replace
     */
    inline bool replace() const
    {
        return replace_;
    }

    /*!
     * @brief This function returns a reference to member replace
     * @return Reference to member replace
     */
    inline bool& replace()
    {
        return replace_;
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
    static size_t getCdrSerializedSize(const CreationMode& data, size_t current_alignment = 0);


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
    bool reuse_;
    bool replace_;
};
typedef std::array<uint8_t, 4> RequestId;
/*!
 * @brief This class represents the structure ResultStatus defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class ResultStatus
{
public:

    /*!
     * @brief Default constructor.
     */
    ResultStatus();
    
    /*!
     * @brief Default destructor.
     */
    ~ResultStatus();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object ResultStatus that will be copied.
     */
    ResultStatus(const ResultStatus &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object ResultStatus that will be copied.
     */
    ResultStatus(ResultStatus &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object ResultStatus that will be copied.
     */
    ResultStatus& operator=(const ResultStatus &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object ResultStatus that will be copied.
     */
    ResultStatus& operator=(ResultStatus &&x);
    
    /*!
     * @brief This function copies the value in member request_id
     * @param _request_id New value to be copied in member request_id
     */
    inline void request_id(const RequestId &_request_id)
    {
        request_id_ = _request_id;
    }

    /*!
     * @brief This function moves the value in member request_id
     * @param _request_id New value to be moved in member request_id
     */
    inline void request_id(RequestId &&_request_id)
    {
        request_id_ = std::move(_request_id);
    }

    /*!
     * @brief This function returns a constant reference to member request_id
     * @return Constant reference to member request_id
     */
    inline const RequestId& request_id() const
    {
        return request_id_;
    }

    /*!
     * @brief This function returns a reference to member request_id
     * @return Reference to member request_id
     */
    inline RequestId& request_id()
    {
        return request_id_;
    }
    /*!
     * @brief This function sets a value in member status
     * @param _status New value for member status
     */
    inline void status(uint8_t _status)
    {
        status_ = _status;
    }

    /*!
     * @brief This function returns the value of member status
     * @return Value of member status
     */
    inline uint8_t status() const
    {
        return status_;
    }

    /*!
     * @brief This function returns a reference to member status
     * @return Reference to member status
     */
    inline uint8_t& status()
    {
        return status_;
    }
    /*!
     * @brief This function sets a value in member implementation_status
     * @param _implementation_status New value for member implementation_status
     */
    inline void implementation_status(uint8_t _implementation_status)
    {
        implementation_status_ = _implementation_status;
    }

    /*!
     * @brief This function returns the value of member implementation_status
     * @return Value of member implementation_status
     */
    inline uint8_t implementation_status() const
    {
        return implementation_status_;
    }

    /*!
     * @brief This function returns a reference to member implementation_status
     * @return Reference to member implementation_status
     */
    inline uint8_t& implementation_status()
    {
        return implementation_status_;
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
    static size_t getCdrSerializedSize(const ResultStatus& data, size_t current_alignment = 0);


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
    RequestId request_id_;
    uint8_t status_;
    uint8_t implementation_status_;
};

/*!
 * @brief This class represents the structure OBJK_DATAREADER_Status defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_DATAREADER_Status
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_DATAREADER_Status();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_DATAREADER_Status();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_DATAREADER_Status that will be copied.
     */
    OBJK_DATAREADER_Status(const OBJK_DATAREADER_Status &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_DATAREADER_Status that will be copied.
     */
    OBJK_DATAREADER_Status(OBJK_DATAREADER_Status &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_DATAREADER_Status that will be copied.
     */
    OBJK_DATAREADER_Status& operator=(const OBJK_DATAREADER_Status &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_DATAREADER_Status that will be copied.
     */
    OBJK_DATAREADER_Status& operator=(OBJK_DATAREADER_Status &&x);
    
    /*!
     * @brief This function sets a value in member highest_acked_num
     * @param _highest_acked_num New value for member highest_acked_num
     */
    inline void highest_acked_num(int16_t _highest_acked_num)
    {
        highest_acked_num_ = _highest_acked_num;
    }

    /*!
     * @brief This function returns the value of member highest_acked_num
     * @return Value of member highest_acked_num
     */
    inline int16_t highest_acked_num() const
    {
        return highest_acked_num_;
    }

    /*!
     * @brief This function returns a reference to member highest_acked_num
     * @return Reference to member highest_acked_num
     */
    inline int16_t& highest_acked_num()
    {
        return highest_acked_num_;
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
    static size_t getCdrSerializedSize(const OBJK_DATAREADER_Status& data, size_t current_alignment = 0);


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
    int16_t highest_acked_num_;
};
/*!
 * @brief This class represents the structure OBJK_DATAWRITER_Status defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class OBJK_DATAWRITER_Status
{
public:

    /*!
     * @brief Default constructor.
     */
    OBJK_DATAWRITER_Status();
    
    /*!
     * @brief Default destructor.
     */
    ~OBJK_DATAWRITER_Status();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object OBJK_DATAWRITER_Status that will be copied.
     */
    OBJK_DATAWRITER_Status(const OBJK_DATAWRITER_Status &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object OBJK_DATAWRITER_Status that will be copied.
     */
    OBJK_DATAWRITER_Status(OBJK_DATAWRITER_Status &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object OBJK_DATAWRITER_Status that will be copied.
     */
    OBJK_DATAWRITER_Status& operator=(const OBJK_DATAWRITER_Status &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object OBJK_DATAWRITER_Status that will be copied.
     */
    OBJK_DATAWRITER_Status& operator=(OBJK_DATAWRITER_Status &&x);
    
    /*!
     * @brief This function sets a value in member stream_seq_num
     * @param _stream_seq_num New value for member stream_seq_num
     */
    inline void stream_seq_num(int16_t _stream_seq_num)
    {
        stream_seq_num_ = _stream_seq_num;
    }

    /*!
     * @brief This function returns the value of member stream_seq_num
     * @return Value of member stream_seq_num
     */
    inline int16_t stream_seq_num() const
    {
        return stream_seq_num_;
    }

    /*!
     * @brief This function returns a reference to member stream_seq_num
     * @return Reference to member stream_seq_num
     */
    inline int16_t& stream_seq_num()
    {
        return stream_seq_num_;
    }
    /*!
     * @brief This function sets a value in member sample_seq_num
     * @param _sample_seq_num New value for member sample_seq_num
     */
    inline void sample_seq_num(uint32_t _sample_seq_num)
    {
        sample_seq_num_ = _sample_seq_num;
    }

    /*!
     * @brief This function returns the value of member sample_seq_num
     * @return Value of member sample_seq_num
     */
    inline uint32_t sample_seq_num() const
    {
        return sample_seq_num_;
    }

    /*!
     * @brief This function returns a reference to member sample_seq_num
     * @return Reference to member sample_seq_num
     */
    inline uint32_t& sample_seq_num()
    {
        return sample_seq_num_;
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
    static size_t getCdrSerializedSize(const OBJK_DATAWRITER_Status& data, size_t current_alignment = 0);


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
    int16_t stream_seq_num_;
    // HACK uint64_t sample_seq_num_;
    uint32_t sample_seq_num_;
};
/*!
 * @brief This class represents the union StatusVariant defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class StatusVariant
{
public:

    /*!
     * @brief Default constructor.
     */
    StatusVariant();
    
    /*!
     * @brief Default destructor.
     */
    ~StatusVariant();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object StatusVariant that will be copied.
     */
    StatusVariant(const StatusVariant &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object StatusVariant that will be copied.
     */
    StatusVariant(StatusVariant &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object StatusVariant that will be copied.
     */
    StatusVariant& operator=(const StatusVariant &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object StatusVariant that will be copied.
     */
    StatusVariant& operator=(StatusVariant &&x);
    
    /*!
     * @brief This function sets the discriminator value.
     * @param _discriminator New value for the discriminator.
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
     */
    void discriminator(ObjectKind _discriminator);
    
    /*!
     * @brief This function returns the value of the discriminator.
     * @return Value of the discriminator
     */
    ObjectKind discriminator() const;
    
    /*!
     * @brief This function returns a reference to the discriminator.
     * @return Reference to the discriminator.
     */
    ObjectKind& discriminator();
    
    /*!
     * @brief This function copies the value in member data_writer
     * @param _data_writer New value to be copied in member data_writer
     */
    void data_writer(const OBJK_DATAWRITER_Status &_data_writer);

    /*!
     * @brief This function moves the value in member data_writer
     * @param _data_writer New value to be moved in member data_writer
     */
    void data_writer(OBJK_DATAWRITER_Status &&_data_writer);

    /*!
     * @brief This function returns a constant reference to member data_writer
     * @return Constant reference to member data_writer
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_DATAWRITER_Status& data_writer() const;

    /*!
     * @brief This function returns a reference to member data_writer
     * @return Reference to member data_writer
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_DATAWRITER_Status& data_writer();
    /*!
     * @brief This function copies the value in member data_reader
     * @param _data_reader New value to be copied in member data_reader
     */
    void data_reader(const OBJK_DATAREADER_Status &_data_reader);

    /*!
     * @brief This function moves the value in member data_reader
     * @param _data_reader New value to be moved in member data_reader
     */
    void data_reader(OBJK_DATAREADER_Status &&_data_reader);

    /*!
     * @brief This function returns a constant reference to member data_reader
     * @return Constant reference to member data_reader
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    const OBJK_DATAREADER_Status& data_reader() const;

    /*!
     * @brief This function returns a reference to member data_reader
     * @return Reference to member data_reader
     * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
     */
    OBJK_DATAREADER_Status& data_reader();
    
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
    static size_t getCdrSerializedSize(const StatusVariant& data, size_t current_alignment = 0);


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
    ObjectKind discriminator_;
    
    OBJK_DATAWRITER_Status data_writer_;
    OBJK_DATAREADER_Status data_reader_;
};
/*!
 * @brief This class represents the structure Status defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class Status
{
public:

    /*!
     * @brief Default constructor.
     */
    Status();
    
    /*!
     * @brief Default destructor.
     */
    ~Status();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Status that will be copied.
     */
    Status(const Status &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object Status that will be copied.
     */
    Status(Status &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Status that will be copied.
     */
    Status& operator=(const Status &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object Status that will be copied.
     */
    Status& operator=(Status &&x);
    
    /*!
     * @brief This function copies the value in member result
     * @param _result New value to be copied in member result
     */
    inline void result(const ResultStatus &_result)
    {
        result_ = _result;
    }

    /*!
     * @brief This function moves the value in member result
     * @param _result New value to be moved in member result
     */
    inline void result(ResultStatus &&_result)
    {
        result_ = std::move(_result);
    }

    /*!
     * @brief This function returns a constant reference to member result
     * @return Constant reference to member result
     */
    inline const ResultStatus& result() const
    {
        return result_;
    }

    /*!
     * @brief This function returns a reference to member result
     * @return Reference to member result
     */
    inline ResultStatus& result()
    {
        return result_;
    }
    /*!
     * @brief This function copies the value in member object_id
     * @param _object_id New value to be copied in member object_id
     */
    inline void object_id(const ObjectId &_object_id)
    {
        object_id_ = _object_id;
    }

    /*!
     * @brief This function moves the value in member object_id
     * @param _object_id New value to be moved in member object_id
     */
    inline void object_id(ObjectId &&_object_id)
    {
        object_id_ = std::move(_object_id);
    }

    /*!
     * @brief This function returns a constant reference to member object_id
     * @return Constant reference to member object_id
     */
    inline const ObjectId& object_id() const
    {
        return object_id_;
    }

    /*!
     * @brief This function returns a reference to member object_id
     * @return Reference to member object_id
     */
    inline ObjectId& object_id()
    {
        return object_id_;
    }
    /*!
     * @brief This function copies the value in member status
     * @param _status New value to be copied in member status
     */
    inline void status(const StatusVariant &_status)
    {
        status_ = _status;
    }

    /*!
     * @brief This function moves the value in member status
     * @param _status New value to be moved in member status
     */
    inline void status(StatusVariant &&_status)
    {
        status_ = std::move(_status);
    }

    /*!
     * @brief This function returns a constant reference to member status
     * @return Constant reference to member status
     */
    inline const StatusVariant& status() const
    {
        return status_;
    }

    /*!
     * @brief This function returns a reference to member status
     * @return Reference to member status
     */
    inline StatusVariant& status()
    {
        return status_;
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
    static size_t getCdrSerializedSize(const Status& data, size_t current_alignment = 0);


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
    ResultStatus result_;
    ObjectId object_id_;
    StatusVariant status_;
};
/*!
 * @brief This class represents the structure Info defined by the user in the IDL file.
 * @ingroup OBJECTVARIANT
 */
class Info
{
public:

    /*!
     * @brief Default constructor.
     */
    Info();
    
    /*!
     * @brief Default destructor.
     */
    ~Info();
    
    /*!
     * @brief Copy constructor.
     * @param x Reference to the object Info that will be copied.
     */
    Info(const Info &x);
    
    /*!
     * @brief Move constructor.
     * @param x Reference to the object Info that will be copied.
     */
    Info(Info &&x);
    
    /*!
     * @brief Copy assignment.
     * @param x Reference to the object Info that will be copied.
     */
    Info& operator=(const Info &x);
    
    /*!
     * @brief Move assignment.
     * @param x Reference to the object Info that will be copied.
     */
    Info& operator=(Info &&x);
    
    /*!
     * @brief This function copies the value in member result
     * @param _result New value to be copied in member result
     */
    inline void result(const ResultStatus &_result)
    {
        result_ = _result;
    }

    /*!
     * @brief This function moves the value in member result
     * @param _result New value to be moved in member result
     */
    inline void result(ResultStatus &&_result)
    {
        result_ = std::move(_result);
    }

    /*!
     * @brief This function returns a constant reference to member result
     * @return Constant reference to member result
     */
    inline const ResultStatus& result() const
    {
        return result_;
    }

    /*!
     * @brief This function returns a reference to member result
     * @return Reference to member result
     */
    inline ResultStatus& result()
    {
        return result_;
    }
    /*!
     * @brief This function copies the value in member object_id
     * @param _object_id New value to be copied in member object_id
     */
    inline void object_id(const ObjectId &_object_id)
    {
        object_id_ = _object_id;
    }

    /*!
     * @brief This function moves the value in member object_id
     * @param _object_id New value to be moved in member object_id
     */
    inline void object_id(ObjectId &&_object_id)
    {
        object_id_ = std::move(_object_id);
    }

    /*!
     * @brief This function returns a constant reference to member object_id
     * @return Constant reference to member object_id
     */
    inline const ObjectId& object_id() const
    {
        return object_id_;
    }

    /*!
     * @brief This function returns a reference to member object_id
     * @return Reference to member object_id
     */
    inline ObjectId& object_id()
    {
        return object_id_;
    }
    /*!
     * @brief This function copies the value in member info
     * @param _info New value to be copied in member info
     */
    inline void info(const ObjectVariant &_info)
    {
        info_ = _info;
    }

    /*!
     * @brief This function moves the value in member info
     * @param _info New value to be moved in member info
     */
    inline void info(ObjectVariant &&_info)
    {
        info_ = std::move(_info);
    }

    /*!
     * @brief This function returns a constant reference to member info
     * @return Constant reference to member info
     */
    inline const ObjectVariant& info() const
    {
        return info_;
    }

    /*!
     * @brief This function returns a reference to member info
     * @return Reference to member info
     */
    inline ObjectVariant& info()
    {
        return info_;
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
    static size_t getCdrSerializedSize(const Info& data, size_t current_alignment = 0);


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
    ResultStatus result_;
    ObjectId object_id_;
    ObjectVariant info_;
};

/*!
* @brief This class represents the structure DataReaderReadeSpec defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class DataReaderReadeSpec
{
public:

    /*!
    * @brief Default constructor.
    */
    DataReaderReadeSpec();

    /*!
    * @brief Default destructor.
    */
    ~DataReaderReadeSpec();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object DataReaderReadeSpec that will be copied.
    */
    DataReaderReadeSpec(const DataReaderReadeSpec &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object DataReaderReadeSpec that will be copied.
    */
    DataReaderReadeSpec(DataReaderReadeSpec &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object DataReaderReadeSpec that will be copied.
    */
    DataReaderReadeSpec& operator=(const DataReaderReadeSpec &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object DataReaderReadeSpec that will be copied.
    */
    DataReaderReadeSpec& operator=(DataReaderReadeSpec &&x);

    /*!
    * @brief This function sets a value in member max_samples
    * @param _max_samples New value for member max_samples
    */
    inline void max_samples(int16_t _max_samples)
    {
        max_samples_ = _max_samples;
    }

    /*!
    * @brief This function returns the value of member max_samples
    * @return Value of member max_samples
    */
    inline int16_t max_samples() const
    {
        return max_samples_;
    }

    /*!
    * @brief This function returns a reference to member max_samples
    * @return Reference to member max_samples
    */
    inline int16_t& max_samples()
    {
        return max_samples_;
    }
    /*!
    * @brief This function sets a value in member include_info
    * @param _include_info New value for member include_info
    */
    inline void include_info(bool _include_info)
    {
        include_info_ = _include_info;
    }

    /*!
    * @brief This function returns the value of member include_info
    * @return Value of member include_info
    */
    inline bool include_info() const
    {
        return include_info_;
    }

    /*!
    * @brief This function returns a reference to member include_info
    * @return Reference to member include_info
    */
    inline bool& include_info()
    {
        return include_info_;
    }
    /*!
    * @brief This function copies the value in member content_filter
    * @param _content_filter New value to be copied in member content_filter
    */
    inline void content_filter(const std::string &_content_filter)
    {
        content_filter_ = _content_filter;
    }

    /*!
    * @brief This function moves the value in member content_filter
    * @param _content_filter New value to be moved in member content_filter
    */
    inline void content_filter(std::string &&_content_filter)
    {
        content_filter_ = std::move(_content_filter);
    }

    /*!
    * @brief This function returns a constant reference to member content_filter
    * @return Constant reference to member content_filter
    */
    inline const std::string& content_filter() const
    {
        return content_filter_;
    }

    /*!
    * @brief This function returns a reference to member content_filter
    * @return Reference to member content_filter
    */
    inline std::string& content_filter()
    {
        return content_filter_;
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
    static size_t getCdrSerializedSize(const DataReaderReadeSpec& data, size_t current_alignment = 0);


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
    int16_t max_samples_;
    bool include_info_;
    std::string content_filter_;
};
typedef uint8_t ReadMode;
const ReadMode READM_DATA = 0x00;
const ReadMode READM_DATA_SEQ = 0x01;
const ReadMode READM_SAMPLE = 0x02;
const ReadMode READM_SAMPLE_SEQ = 0x03;
const ReadMode READM_PACKED_SAMPLE_SEQ = 0x04;
/*!
* @brief This class represents the structure SampleInfo defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class SampleInfo
{
public:

    /*!
    * @brief Default constructor.
    */
    SampleInfo();

    /*!
    * @brief Default destructor.
    */
    ~SampleInfo();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object SampleInfo that will be copied.
    */
    SampleInfo(const SampleInfo &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object SampleInfo that will be copied.
    */
    SampleInfo(SampleInfo &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object SampleInfo that will be copied.
    */
    SampleInfo& operator=(const SampleInfo &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object SampleInfo that will be copied.
    */
    SampleInfo& operator=(SampleInfo &&x);

    /*!
    * @brief This function sets a value in member state
    * @param _state New value for member state
    */
    inline void state(uint8_t _state)
    {
        state_ = _state;
    }

    /*!
    * @brief This function returns the value of member state
    * @return Value of member state
    */
    inline uint8_t state() const
    {
        return state_;
    }

    /*!
    * @brief This function returns a reference to member state
    * @return Reference to member state
    */
    inline uint8_t& state()
    {
        return state_;
    }
    /*!
    * @brief This function sets a value in member sequence_number
    * @param _sequence_number New value for member sequence_number
    */
    inline void sequence_number(uint32_t _sequence_number)
    {
        sequence_number_ = _sequence_number;
    }

    /*!
    * @brief This function returns the value of member sequence_number
    * @return Value of member sequence_number
    */
    inline uint32_t sequence_number() const
    {
        return sequence_number_;
    }

    /*!
    * @brief This function returns a reference to member sequence_number
    * @return Reference to member sequence_number
    */
    inline uint32_t& sequence_number()
    {
        return sequence_number_;
    }
    /*!
    * @brief This function sets a value in member session_time_offset
    * @param _session_time_offset New value for member session_time_offset
    */
    inline void session_time_offset(uint32_t _session_time_offset)
    {
        session_time_offset_ = _session_time_offset;
    }

    /*!
    * @brief This function returns the value of member session_time_offset
    * @return Value of member session_time_offset
    */
    inline uint32_t session_time_offset() const
    {
        return session_time_offset_;
    }

    /*!
    * @brief This function returns a reference to member session_time_offset
    * @return Reference to member session_time_offset
    */
    inline uint32_t& session_time_offset()
    {
        return session_time_offset_;
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
    static size_t getCdrSerializedSize(const SampleInfo& data, size_t current_alignment = 0);


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
    uint8_t state_;
    // HACK uint64_t sequence_number_;
    uint32_t sequence_number_;
    uint32_t session_time_offset_;
};
/*!
* @brief This class represents the structure SampleInfoDelta defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class SampleInfoDelta
{
public:

    /*!
    * @brief Default constructor.
    */
    SampleInfoDelta();

    /*!
    * @brief Default destructor.
    */
    ~SampleInfoDelta();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object SampleInfoDelta that will be copied.
    */
    SampleInfoDelta(const SampleInfoDelta &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object SampleInfoDelta that will be copied.
    */
    SampleInfoDelta(SampleInfoDelta &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object SampleInfoDelta that will be copied.
    */
    SampleInfoDelta& operator=(const SampleInfoDelta &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object SampleInfoDelta that will be copied.
    */
    SampleInfoDelta& operator=(SampleInfoDelta &&x);

    /*!
    * @brief This function sets a value in member state
    * @param _state New value for member state
    */
    inline void state(uint8_t _state)
    {
        state_ = _state;
    }

    /*!
    * @brief This function returns the value of member state
    * @return Value of member state
    */
    inline uint8_t state() const
    {
        return state_;
    }

    /*!
    * @brief This function returns a reference to member state
    * @return Reference to member state
    */
    inline uint8_t& state()
    {
        return state_;
    }
    /*!
    * @brief This function sets a value in member info_seq_number_delta
    * @param _info_seq_number_delta New value for member info_seq_number_delta
    */
    inline void info_seq_number_delta(uint8_t _info_seq_number_delta)
    {
        info_seq_number_delta_ = _info_seq_number_delta;
    }

    /*!
    * @brief This function returns the value of member info_seq_number_delta
    * @return Value of member info_seq_number_delta
    */
    inline uint8_t info_seq_number_delta() const
    {
        return info_seq_number_delta_;
    }

    /*!
    * @brief This function returns a reference to member info_seq_number_delta
    * @return Reference to member info_seq_number_delta
    */
    inline uint8_t& info_seq_number_delta()
    {
        return info_seq_number_delta_;
    }
    /*!
    * @brief This function sets a value in member info_timestamp_delta
    * @param _info_timestamp_delta New value for member info_timestamp_delta
    */
    inline void info_timestamp_delta(uint16_t _info_timestamp_delta)
    {
        info_timestamp_delta_ = _info_timestamp_delta;
    }

    /*!
    * @brief This function returns the value of member info_timestamp_delta
    * @return Value of member info_timestamp_delta
    */
    inline uint16_t info_timestamp_delta() const
    {
        return info_timestamp_delta_;
    }

    /*!
    * @brief This function returns a reference to member info_timestamp_delta
    * @return Reference to member info_timestamp_delta
    */
    inline uint16_t& info_timestamp_delta()
    {
        return info_timestamp_delta_;
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
    static size_t getCdrSerializedSize(const SampleInfoDelta& data, size_t current_alignment = 0);


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
    uint8_t state_;
    uint8_t info_seq_number_delta_;
    uint16_t info_timestamp_delta_;
};
/*!
* @brief This class represents the structure SampleData defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class SampleData
{
public:

    /*!
    * @brief Default constructor.
    */
    SampleData();

    /*!
    * @brief Default destructor.
    */
    ~SampleData();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object SampleData that will be copied.
    */
    SampleData(const SampleData &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object SampleData that will be copied.
    */
    SampleData(SampleData &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object SampleData that will be copied.
    */
    SampleData& operator=(const SampleData &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object SampleData that will be copied.
    */
    SampleData& operator=(SampleData &&x);

    /*!
    * @brief This function copies the value in member serialized_data
    * @param _serialized_data New value to be copied in member serialized_data
    */
    inline void serialized_data(const std::vector<uint8_t> &_serialized_data)
    {
        serialized_data_ = _serialized_data;
    }

    /*!
    * @brief This function moves the value in member serialized_data
    * @param _serialized_data New value to be moved in member serialized_data
    */
    inline void serialized_data(std::vector<uint8_t> &&_serialized_data)
    {
        serialized_data_ = std::move(_serialized_data);
    }

    /*!
    * @brief This function returns a constant reference to member serialized_data
    * @return Constant reference to member serialized_data
    */
    inline const std::vector<uint8_t>& serialized_data() const
    {
        return serialized_data_;
    }

    /*!
    * @brief This function returns a reference to member serialized_data
    * @return Reference to member serialized_data
    */
    inline std::vector<uint8_t>& serialized_data()
    {
        return serialized_data_;
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
    static size_t getCdrSerializedSize(const SampleData& data, size_t current_alignment = 0);


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
    std::vector<uint8_t> serialized_data_;
};
typedef std::vector<SampleData> SampleDataSeq;
/*!
* @brief This class represents the structure Sample defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class Sample
{
public:

    /*!
    * @brief Default constructor.
    */
    Sample();

    /*!
    * @brief Default destructor.
    */
    ~Sample();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object Sample that will be copied.
    */
    Sample(const Sample &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object Sample that will be copied.
    */
    Sample(Sample &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object Sample that will be copied.
    */
    Sample& operator=(const Sample &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object Sample that will be copied.
    */
    Sample& operator=(Sample &&x);

    /*!
    * @brief This function copies the value in member info
    * @param _info New value to be copied in member info
    */
    inline void info(const SampleInfo &_info)
    {
        info_ = _info;
    }

    /*!
    * @brief This function moves the value in member info
    * @param _info New value to be moved in member info
    */
    inline void info(SampleInfo &&_info)
    {
        info_ = std::move(_info);
    }

    /*!
    * @brief This function returns a constant reference to member info
    * @return Constant reference to member info
    */
    inline const SampleInfo& info() const
    {
        return info_;
    }

    /*!
    * @brief This function returns a reference to member info
    * @return Reference to member info
    */
    inline SampleInfo& info()
    {
        return info_;
    }
    /*!
    * @brief This function copies the value in member data
    * @param _data New value to be copied in member data
    */
    inline void data(const SampleData &_data)
    {
        data_ = _data;
    }

    /*!
    * @brief This function moves the value in member data
    * @param _data New value to be moved in member data
    */
    inline void data(SampleData &&_data)
    {
        data_ = std::move(_data);
    }

    /*!
    * @brief This function returns a constant reference to member data
    * @return Constant reference to member data
    */
    inline const SampleData& data() const
    {
        return data_;
    }

    /*!
    * @brief This function returns a reference to member data
    * @return Reference to member data
    */
    inline SampleData& data()
    {
        return data_;
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
    static size_t getCdrSerializedSize(const Sample& data, size_t current_alignment = 0);


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
    SampleInfo info_;
    SampleData data_;
};
typedef std::vector<Sample> SampleSeq;
/*!
* @brief This class represents the structure SampleDelta defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class SampleDelta
{
public:

    /*!
    * @brief Default constructor.
    */
    SampleDelta();

    /*!
    * @brief Default destructor.
    */
    ~SampleDelta();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object SampleDelta that will be copied.
    */
    SampleDelta(const SampleDelta &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object SampleDelta that will be copied.
    */
    SampleDelta(SampleDelta &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object SampleDelta that will be copied.
    */
    SampleDelta& operator=(const SampleDelta &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object SampleDelta that will be copied.
    */
    SampleDelta& operator=(SampleDelta &&x);

    /*!
    * @brief This function copies the value in member info_delta
    * @param _info_delta New value to be copied in member info_delta
    */
    inline void info_delta(const SampleInfoDelta &_info_delta)
    {
        info_delta_ = _info_delta;
    }

    /*!
    * @brief This function moves the value in member info_delta
    * @param _info_delta New value to be moved in member info_delta
    */
    inline void info_delta(SampleInfoDelta &&_info_delta)
    {
        info_delta_ = std::move(_info_delta);
    }

    /*!
    * @brief This function returns a constant reference to member info_delta
    * @return Constant reference to member info_delta
    */
    inline const SampleInfoDelta& info_delta() const
    {
        return info_delta_;
    }

    /*!
    * @brief This function returns a reference to member info_delta
    * @return Reference to member info_delta
    */
    inline SampleInfoDelta& info_delta()
    {
        return info_delta_;
    }
    /*!
    * @brief This function copies the value in member data
    * @param _data New value to be copied in member data
    */
    inline void data(const SampleData &_data)
    {
        data_ = _data;
    }

    /*!
    * @brief This function moves the value in member data
    * @param _data New value to be moved in member data
    */
    inline void data(SampleData &&_data)
    {
        data_ = std::move(_data);
    }

    /*!
    * @brief This function returns a constant reference to member data
    * @return Constant reference to member data
    */
    inline const SampleData& data() const
    {
        return data_;
    }

    /*!
    * @brief This function returns a reference to member data
    * @return Reference to member data
    */
    inline SampleData& data()
    {
        return data_;
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
    static size_t getCdrSerializedSize(const SampleDelta& data, size_t current_alignment = 0);


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
    SampleInfoDelta info_delta_;
    SampleData data_;
};
/*!
* @brief This class represents the structure SamplePacked defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class SamplePacked
{
public:

    /*!
    * @brief Default constructor.
    */
    SamplePacked();

    /*!
    * @brief Default destructor.
    */
    ~SamplePacked();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object SamplePacked that will be copied.
    */
    SamplePacked(const SamplePacked &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object SamplePacked that will be copied.
    */
    SamplePacked(SamplePacked &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object SamplePacked that will be copied.
    */
    SamplePacked& operator=(const SamplePacked &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object SamplePacked that will be copied.
    */
    SamplePacked& operator=(SamplePacked &&x);

    /*!
    * @brief This function copies the value in member info_base
    * @param _info_base New value to be copied in member info_base
    */
    inline void info_base(const SampleInfo &_info_base)
    {
        info_base_ = _info_base;
    }

    /*!
    * @brief This function moves the value in member info_base
    * @param _info_base New value to be moved in member info_base
    */
    inline void info_base(SampleInfo &&_info_base)
    {
        info_base_ = std::move(_info_base);
    }

    /*!
    * @brief This function returns a constant reference to member info_base
    * @return Constant reference to member info_base
    */
    inline const SampleInfo& info_base() const
    {
        return info_base_;
    }

    /*!
    * @brief This function returns a reference to member info_base
    * @return Reference to member info_base
    */
    inline SampleInfo& info_base()
    {
        return info_base_;
    }
    /*!
    * @brief This function copies the value in member sample
    * @param _sample New value to be copied in member sample
    */
    inline void sample(const std::vector<SampleDelta> &_sample)
    {
        sample_ = _sample;
    }

    /*!
    * @brief This function moves the value in member sample
    * @param _sample New value to be moved in member sample
    */
    inline void sample(std::vector<SampleDelta> &&_sample)
    {
        sample_ = std::move(_sample);
    }

    /*!
    * @brief This function returns a constant reference to member sample
    * @return Constant reference to member sample
    */
    inline const std::vector<SampleDelta>& sample() const
    {
        return sample_;
    }

    /*!
    * @brief This function returns a reference to member sample
    * @return Reference to member sample
    */
    inline std::vector<SampleDelta>& sample()
    {
        return sample_;
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
    static size_t getCdrSerializedSize(const SamplePacked& data, size_t current_alignment = 0);


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
    SampleInfo info_base_;
    std::vector<SampleDelta> sample_;
};
typedef std::vector<SamplePacked> SamplePackedSeq;
/*!
* @brief This class represents the union RT_Data defined by the user in the IDL file.
* @ingroup OBJECTVARIANT
*/
class RT_Data
{
public:

    /*!
    * @brief Default constructor.
    */
    RT_Data();

    /*!
    * @brief Default destructor.
    */
    ~RT_Data();

    /*!
    * @brief Copy constructor.
    * @param x Reference to the object RT_Data that will be copied.
    */
    RT_Data(const RT_Data &x);

    /*!
    * @brief Move constructor.
    * @param x Reference to the object RT_Data that will be copied.
    */
    RT_Data(RT_Data &&x);

    /*!
    * @brief Copy assignment.
    * @param x Reference to the object RT_Data that will be copied.
    */
    RT_Data& operator=(const RT_Data &x);

    /*!
    * @brief Move assignment.
    * @param x Reference to the object RT_Data that will be copied.
    */
    RT_Data& operator=(RT_Data &&x);

    /*!
    * @brief This function sets the discriminator value.
    * @param __d New value for the discriminator.
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the new value doesn't correspond to the selected union member.
    */
    void _d(ReadMode __d);

    /*!
    * @brief This function returns the value of the discriminator.
    * @return Value of the discriminator
    */
    ReadMode _d() const;

    /*!
    * @brief This function returns a reference to the discriminator.
    * @return Reference to the discriminator.
    */
    ReadMode& _d();

    /*!
    * @brief This function copies the value in member data
    * @param _data New value to be copied in member data
    */
    void data(const SampleData &_data);

    /*!
    * @brief This function moves the value in member data
    * @param _data New value to be moved in member data
    */
    void data(SampleData &&_data);

    /*!
    * @brief This function returns a constant reference to member data
    * @return Constant reference to member data
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    const SampleData& data() const;

    /*!
    * @brief This function returns a reference to member data
    * @return Reference to member data
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    SampleData& data();
    /*!
    * @brief This function copies the value in member data_seq
    * @param _data_seq New value to be copied in member data_seq
    */
    void data_seq(const SampleDataSeq &_data_seq);

    /*!
    * @brief This function moves the value in member data_seq
    * @param _data_seq New value to be moved in member data_seq
    */
    void data_seq(SampleDataSeq &&_data_seq);

    /*!
    * @brief This function returns a constant reference to member data_seq
    * @return Constant reference to member data_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    const SampleDataSeq& data_seq() const;

    /*!
    * @brief This function returns a reference to member data_seq
    * @return Reference to member data_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    SampleDataSeq& data_seq();
    /*!
    * @brief This function copies the value in member sample
    * @param _sample New value to be copied in member sample
    */
    void sample(const Sample &_sample);

    /*!
    * @brief This function moves the value in member sample
    * @param _sample New value to be moved in member sample
    */
    void sample(Sample &&_sample);

    /*!
    * @brief This function returns a constant reference to member sample
    * @return Constant reference to member sample
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    const Sample& sample() const;

    /*!
    * @brief This function returns a reference to member sample
    * @return Reference to member sample
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    Sample& sample();
    /*!
    * @brief This function copies the value in member sample_seq
    * @param _sample_seq New value to be copied in member sample_seq
    */
    void sample_seq(const SampleSeq &_sample_seq);

    /*!
    * @brief This function moves the value in member sample_seq
    * @param _sample_seq New value to be moved in member sample_seq
    */
    void sample_seq(SampleSeq &&_sample_seq);

    /*!
    * @brief This function returns a constant reference to member sample_seq
    * @return Constant reference to member sample_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    const SampleSeq& sample_seq() const;

    /*!
    * @brief This function returns a reference to member sample_seq
    * @return Reference to member sample_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    SampleSeq& sample_seq();
    /*!
    * @brief This function copies the value in member sample_packed_seq
    * @param _sample_packed_seq New value to be copied in member sample_packed_seq
    */
    void sample_packed_seq(const SamplePackedSeq &_sample_packed_seq);

    /*!
    * @brief This function moves the value in member sample_packed_seq
    * @param _sample_packed_seq New value to be moved in member sample_packed_seq
    */
    void sample_packed_seq(SamplePackedSeq &&_sample_packed_seq);

    /*!
    * @brief This function returns a constant reference to member sample_packed_seq
    * @return Constant reference to member sample_packed_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    const SamplePackedSeq& sample_packed_seq() const;

    /*!
    * @brief This function returns a reference to member sample_packed_seq
    * @return Reference to member sample_packed_seq
    * @exception eprosima::fastcdr::BadParamException This exception is thrown if the requested union member is not the current selection.
    */
    SamplePackedSeq& sample_packed_seq();

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
    static size_t getCdrSerializedSize(const RT_Data& data, size_t current_alignment = 0);


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
    ReadMode discriminator_;

    SampleData data_;
    SampleDataSeq data_seq_;
    Sample sample_;
    SampleSeq sample_seq_;
    SamplePackedSeq sample_packed_seq_;
};

#endif // _ObjectVariant_H_