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

#ifndef _UXR_AGENT_UTILS_SEQNUM_HPP_
#define _UXR_AGENT_UTILS_SEQNUM_HPP_

#include <cstdint>
#include <utility>

namespace eprosima {
namespace uxr {

class SeqNum
{
public:
    SeqNum() : seq_num_(0) {}
    SeqNum(uint16_t seq_num) : seq_num_(seq_num) {}
    SeqNum(int seq_num) : seq_num_(uint16_t(seq_num)) {}

    /*
     * Operators.
     */
    inline operator uint16_t() const { return seq_num_; }

    friend bool operator<(const SeqNum& lhs, const SeqNum& rhs)
    {
        return (lhs.seq_num_ != rhs.seq_num_) &&
               (((lhs.seq_num_ < rhs.seq_num_) && ((rhs.seq_num_ - lhs.seq_num_) < (ADD_RANGE[1] + 1))) ||
                ((lhs.seq_num_ > rhs.seq_num_) && ((lhs.seq_num_ - rhs.seq_num_) > (ADD_RANGE[1] + 1))));
    }

    friend bool operator>(const SeqNum& lhs, const SeqNum& rhs) { return rhs < lhs; }
    friend bool operator<=(const SeqNum& lhs, const SeqNum& rhs) { return !(lhs > rhs); }
    friend bool operator>=(const SeqNum& lhs, const SeqNum& rhs) { return !(lhs < rhs); }
    friend bool operator==(const SeqNum& lhs, const SeqNum& rhs) { return lhs.seq_num_ == rhs.seq_num_; }
    friend bool operator==(const SeqNum& lhs, const int& rhs) { return lhs.seq_num_ == uint16_t(rhs); }
    friend bool operator==(const int& lhs, const SeqNum& rhs) { return uint16_t(lhs) == rhs.seq_num_; }
    friend bool operator==(const SeqNum& lhs, const uint16_t& rhs) { return lhs.seq_num_ == rhs; }
    friend bool operator==(const uint16_t& lhs, const SeqNum& rhs) { return lhs == rhs.seq_num_; }
    friend bool operator!=(const SeqNum& lhs, const SeqNum& rhs) { return lhs.seq_num_ != rhs.seq_num_; }

    SeqNum& operator+=(const int& rhs)
    {
        if ((rhs >= ADD_RANGE[0]) && (rhs <= (ADD_RANGE[1])))
        {
            seq_num_ = (seq_num_ + uint16_t(rhs)) % MAX;
        }
        return *this;
    }

    SeqNum& operator+=(const SeqNum& rhs)
    {
        if ((rhs >= SeqNum(ADD_RANGE[0])) && (rhs <= SeqNum(ADD_RANGE[1])))
        {
            seq_num_ = (seq_num_ + rhs.seq_num_) % MAX;
        }
        return *this;
    }

    friend SeqNum operator+(SeqNum lhs, const SeqNum& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend SeqNum operator+(SeqNum lhs, const int& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend SeqNum operator+(const int& lhs, SeqNum rhs)
    {
        rhs += lhs;
        return rhs;
    }

    SeqNum& operator++()
    {
        seq_num_ = (seq_num_ == UINT16_MAX) ? 0 : seq_num_ + 1;
        return *this;
    }

    SeqNum& operator-=(const int& rhs)
    {
        seq_num_ = (uint16_t(rhs) > seq_num_)
                ? uint16_t(seq_num_ - uint16_t(rhs) + MAX)
                : uint16_t(seq_num_ - uint16_t(rhs));
        return *this;
    }

    SeqNum& operator-=(const SeqNum& rhs)
    {
        seq_num_ = (rhs.seq_num_ > seq_num_)
                ? uint16_t(seq_num_ - rhs.seq_num_ + MAX)
                : uint16_t(seq_num_ - rhs.seq_num_);
        return *this;
    }

    friend SeqNum operator-(SeqNum lhs, const SeqNum& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend SeqNum operator-(SeqNum lhs, const int& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    friend SeqNum operator-(const int& lhs, SeqNum rhs)
    {
        rhs -= lhs;
        return rhs;
    }

private:
    uint16_t seq_num_;
    static constexpr uint32_t MAX{UINT16_MAX + 1};
    static constexpr uint16_t ADD_RANGE[2]{0, INT16_MAX};
};

} // namespace uxr
} // namespace eprosima

#endif //_UXR_AGENT_UTILS_SEQNUM_HPP_
