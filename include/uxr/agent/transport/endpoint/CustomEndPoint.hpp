// Copyright 2021 - present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR_AGENT_TRANSPORT_ENDPOINT_CUSTOM_ENDPOINT_HPP_
#define UXR_AGENT_TRANSPORT_ENDPOINT_CUSTOM_ENDPOINT_HPP_

#include <map>
#include <memory>
#include <sstream>

namespace eprosima {
namespace uxr {

/**
 * @brief Class to allow user to easily create an endpoint for their Custom Agent
 *        implementation, if applicable.
 *        A certain set of values are permitted, including unsigned integers and
 *        strings, which usually are more than enough to characterize an endpoint.
 */
class CustomEndPoint
{
private:
    /**
     * @brief Enum class which holds all the available type kinds for an endpoint member.
     */
    enum class MemberKind
    {
        UINT8,
        UINT16,
        UINT32,
        UINT64,
#ifdef __SIZEOF_UINT128__
        UINT128,
#endif // __SIZEOF_UINT128__
        STRING
    };

    /**
     * @brief Struct defining a member in terms of its kind and an opaque pointer,
     *        containing the member data itself.
     */
    typedef struct Member
    {
        MemberKind kind;
        std::shared_ptr<void> data;
    } Member;

    /**
     * @brief Exception to be launched when trying to insert two elements
     *        with the same key on the CustomEndPoint map.
     */
    class SameKeyException : public std::exception
    {
    public:
        SameKeyException(
                const char* file,
                int line,
                const char* func,
                const std::string& key)
        {
            std::stringstream what;
            what << file << ":" << line << ":" << func
                 << ": Cannot add two members with the key '"
                 << key << "'.";
            message_ = what.str();
        }

        const char* what() const noexcept
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    class NoExistingMemberException : public std::exception
    {
    public:
        NoExistingMemberException(
                const char* file,
                int line,
                const char* func,
                const std::string& key)
        {
            std::stringstream what;
            what << file << ":" << line << ":" << func
                 << ": Member '"
                 << key << "' does not exist.";
            message_ = what.str();
        }

        const char* what() const noexcept
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };

    class EmptyMemberException : public std::exception
    {
    public:
        EmptyMemberException(
                const std::string& member_name)
        {
            std::stringstream what;
            what << ": Member '"
                 << member_name
                 << "' is empty.";
            message_ = what.str();
        }

        const char* what() const noexcept
        {
            return message_.c_str();
        }

    private:
        std::string message_;
    };
public:
    /**
     * @brief Default constructor.
     */
    CustomEndPoint() = default;

    /**
     * @brief Default destructor.
     */
    ~CustomEndPoint() = default;

    /**
     * @brief Adds a member to the member list.
     *        The member must not already exist in the EndPoint.
     * @param name The member name,
     * @param kind The member kind.
     * @throw SameKeyException if member already exists.
     * @return true if the insert was successful, or false otherwise.
     */
    bool add_member(
            const std::string& name,
            const MemberKind& kind)
    {
        if (members_.end() != members_.find(name))
        {
            throw SameKeyException(__FILE__, __LINE__, __FUNCTION__, name);
        }

        Member member;
        member.kind = kind;
        member.data.reset();

        auto res = members_.emplace(name, std::move(member));
        return res.second;
    }

    /**
     * @brief Allows to add a member without specifying the MemberKind.
     *        This template must be specialized for every supported type.
     * @param name The new member name.
     * @returns true if the insert was successful, false otherwise.
     */
    template <typename T>
    bool add_member(
            const std::string& name);

    /**
     * @brief Helper method to reset all the contained data within members.
     */
    void reset()
    {
        for (auto& member : members_)
        {
            member.second.data.reset();
        }
    }

    /**
     * @brief Allows setting a value for a given member.
     * @param name Key value to be searched in the members map.
     * @param value A const reference to the value to be set.
     * @throw NoExistingMemberException if trying to set a value not registered in the map.
     */
    template <typename T>
    void set_member_value(
            const std::string& name,
            const T& value)
    {
        if (members_.end() == members_.find(name))
        {
            throw NoExistingMemberException(__FILE__, __LINE__, __FUNCTION__, name);
        }
        else
        {
            members_.at(name).data.reset(new T(value));
        }
    }

    /**
     * @brief Allows setting a value for a given member.
     * @param name Key value to be searched in the members map.
     * @param value A movable reference to the value to be set.
     * @throw NoExistingMemberException if trying to set a value not registered in the map.
     */
    template <typename T>
    void set_member_value(
            const std::string& name,
            T&& value)
    {
        if (members_.end() == members_.find(name))
        {
            throw NoExistingMemberException(__FILE__, __LINE__, __FUNCTION__, name);
        }
        else
        {
            members_.at(name).data.reset(new T(std::move(value)));
        }
    }

    /**
     * 
     */
    void check_non_empty_members()
    {
        for (const auto& member : members_)
        {
            if (nullptr == member.second.data.get())
            {
                throw EmptyMemberException(member.first);
            }
        }
    }

    /**
     * @brief Helper static method that allows to compare two members of the same type.
     * @param first First member to be compared.
     * @param second Second member to be compared against the first.
     * @return 0 if the members are equal, -1 if first < second, 1 if second > first.
     */
    template <typename T>
    static int8_t less_than_members(
            const Member& first,
            const Member& second)
    {
        T first_data = *static_cast<T *>(first.data.get());
        T second_data = *static_cast<T *>(second.data.get());

        if (first_data == second_data)
        {
            return 0;
        }
        else if (first_data < second_data)
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }

    /**
     * @brief Operator < overload.
     * @param other The CustomEndPoint to be checked against this one.
     * @return True if this < other, false otherwise.
     */
    bool operator <(
            const CustomEndPoint& other) const
    {
        for (const auto& member : members_)
        {
            const std::string& member_name = member.first;

            auto other_member = other.members_.at(member_name);
            switch (member.second.kind)
            {
                case MemberKind::UINT8:
                {
                    int8_t res = less_than_members<uint8_t>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
                case MemberKind::UINT16:
                {
                    int8_t res = less_than_members<uint16_t>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
                case MemberKind::UINT32:
                {
                    int8_t res = less_than_members<uint32_t>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
                case MemberKind::UINT64:
                {
                    int8_t res = less_than_members<uint64_t>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
#ifdef __SIZEOF_UINT128__
                case MemberKind::UINT128:
                {
                    int8_t res = less_than_members<uint128_t>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
#endif // __SIZEOF_UINT128__
                case MemberKind::STRING:
                {
                    int8_t res = less_than_members<std::string>(member.second, other_member);
                    if (0 == res)
                    {
                        break;
                    }
                    return (res < 0) ? true : false;
                }
            }
        }
        return false;
    }

    /**
     * @brief Operator << overload for ostream operations.
     * @param os The ostream object to which the output is sent.
     * @param endpoint The CustomEndPoint to be sent to the ostream.
     * @return Reference to the modified ostream.
     */
    friend std::ostream& operator <<(
            std::ostream& os,
            const CustomEndPoint& endpoint)
    {
        for (const auto& member : endpoint.members_)
        {
            os << member.first << ": ";

            if (nullptr == member.second.data.get())
            {
                os << "<null>";
            }
            else
            {
                switch (member.second.kind)
                {
                    case MemberKind::UINT8:
                    {
                        os << *static_cast<uint8_t *>(member.second.data.get());
                        break;
                    }
                    case MemberKind::UINT16:
                    {
                        os << *static_cast<uint16_t *>(member.second.data.get());
                        break;
                    }
                    case MemberKind::UINT32:
                    {
                        os << *static_cast<uint32_t *>(member.second.data.get());
                        break;
                    }
                    case MemberKind::UINT64:
                    {
                        os << *static_cast<uint64_t *>(member.second.data.get());
                        break;
                    }
#ifdef __SIZEOF_UINT128__
                    case MemberKind::UINT128:
                    {
                        os << *static_cast<uint128_t *>(member.second.data.get());
                        break;
                    }
#endif // __SIZEOF_UINT128__
                    case MemberKind::STRING:
                    {
                        os << "'"
                        << *static_cast<std::string *>(member.second.data.get())
                        << "'";
                        break;
                    }
                }
            }

            if (&(*endpoint.members_.rbegin()) != &member)
            {
                os << ", ";
            }
        }

        return os;
    }

    /**
     * @brief Get a member's value, given its key.
     * @param key The member's key.
     * @throw NoExistingMemberException if the member is not found.
     * @return Const reference to the requested value.
     */
    template <typename T>
    const T& get_member(
            const char* key) const
    {
        if (members_.end() == members_.find(key))
        {
            throw NoExistingMemberException(__FILE__, __LINE__, __FUNCTION__, key);
        }
        else
        {
            return *static_cast<T *>(members_.at(key).data.get());
        }
    }

    /**
     * @brief Get a member's value, given its key.
     * @param key The member's key.
     * @throw NoExistingMemberException if the member is not found.
     * @return Const reference to the requested value.
     */
    template <typename T>
    const T& get_member(
            const std::string& key) const
    {
        return this->get_member<T>(key.c_str());
    }

private:
    std::map<std::string, Member> members_;
};

/**
 * @brief CustomEndPoint::add_member template method specializations,
 *        for each of the available MemberKind types.
 */
template <>
inline bool CustomEndPoint::add_member<uint8_t>(
        const std::string& name)
{
    return add_member(name, MemberKind::UINT8);
}

template <>
inline bool CustomEndPoint::add_member<uint16_t>(
        const std::string& name)
{
    return add_member(name, MemberKind::UINT16);
}

template <>
inline bool CustomEndPoint::add_member<uint32_t>(
        const std::string& name)
{
    return add_member(name, MemberKind::UINT32);
}

template <>
inline bool CustomEndPoint::add_member<uint64_t>(
        const std::string& name)
{
    return add_member(name, MemberKind::UINT64);
}

#ifdef __SIZEOF_UINT128__
template <>
inline bool CustomEndPoint::add_member<uint128_t>(
        const std::string& name)
{
    return add_member(name, MemberKind::UINT128);
}
#endif // __SIZEOF_UINT128__

template <>
inline bool CustomEndPoint::add_member<std::string>(
        const std::string& name)
{
    return add_member(name, MemberKind::STRING);
}

} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_TRANSPORT_ENDPOINT_IPV4_ENDPOINT_HPP_
