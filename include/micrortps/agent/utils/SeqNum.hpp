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

#ifndef _MICRORTPS_AGENT_UTILS_SEQNUM_HPP_
#define _MICRORTPS_AGENT_UTILS_SEQNUM_HPP_

#include <cstdint>

namespace eprosima {
namespace micrortps {

class SeqNum
{
public:
    SeqNum() : seq_num_(0) {}
    SeqNum(uint16_t seq_num) : seq_num_(seq_num) {}
    SeqNum(int seq_num) : seq_num_(static_cast<uint16_t>(seq_num)) {}

    /*
     * Operators.
     */
    inline operator uint16_t() const { return seq_num_; }
    operator int() const { return seq_num_; }

    inline SeqNum& operator+=(const int& rhs)
    {
        seq_num_ = (seq_num_ + static_cast<uint16_t>(rhs)) % seq_num_limits_;
        return *this;
    }

    inline SeqNum& operator+=(const SeqNum& rhs)
    {
        seq_num_ = (seq_num_ + rhs.seq_num_) % seq_num_limits_;
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

    friend bool operator<(const SeqNum& lhs, const SeqNum& rhs)
    {
        return (lhs.seq_num_ != rhs.seq_num_) &&
               (((lhs.seq_num_ < rhs.seq_num_) && ((rhs.seq_num_ - lhs.seq_num_) < (seq_num_add_range_))) ||
                ((lhs.seq_num_ > rhs.seq_num_) && ((lhs.seq_num_ - rhs.seq_num_) > (seq_num_add_range_))));
    }

    friend bool operator>(const SeqNum& lhs, const SeqNum& rhs) { return rhs < lhs; }
    friend bool operator<=(const SeqNum& lhs, const SeqNum& rhs) { return !(lhs > rhs); }
    friend bool operator>=(const SeqNum& lhs, const SeqNum& rhs) { return !(lhs < rhs); }
    friend bool operator==(const SeqNum& lhs, const SeqNum& rhs) { return lhs.seq_num_ == rhs.seq_num_; }
    friend bool operator!=(const SeqNum& lhs, const SeqNum& rhs) { return lhs.seq_num_ != rhs.seq_num_; }

private:
    uint16_t seq_num_;
    static const int32_t seq_num_limits_ = (1 << 16);
    static const int32_t seq_num_add_range_ = (1 << 15);
};

} // namespace micrortps
} // namespace eprosima

#endif //_MICRORTPS_AGENT_UTILS_SEQNUM_HPP_
