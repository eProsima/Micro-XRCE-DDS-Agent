// Copyright 2017-present Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef UXR__AGENT__TRANSPORT__UTIL__INTERFACE_HPP_
#define UXR__AGENT__TRANSPORT__UTIL__INTERFACE_HPP_

#include <uxr/agent/types/XRCETypes.hpp>

#include <iostream>

inline
std::ostream & operator<<(std::ostream & os, dds::xrce::TransportAddress const & address)
{
    switch (address._d())
    {
        case dds::xrce::ADDRESS_FORMAT_MEDIUM:
            os << int(address.medium_locator().address().at(0)) << "."
               << int(address.medium_locator().address().at(1)) << "."
               << int(address.medium_locator().address().at(2)) << "."
               << int(address.medium_locator().address().at(3)) << ":"
               << address.medium_locator().port();
            break;
        case dds::xrce::ADDRESS_FORMAT_LARGE:
            os << std::hex << "["
               << int(address.large_locator().address().at(0))
               << int(address.large_locator().address().at(1))
               << ":"
               << int(address.large_locator().address().at(2))
               << int(address.large_locator().address().at(3))
               << ":"
               << int(address.large_locator().address().at(4))
               << int(address.large_locator().address().at(5))
               << ":"
               << int(address.large_locator().address().at(6))
               << int(address.large_locator().address().at(7))
               << ":"
               << int(address.large_locator().address().at(8))
               << int(address.large_locator().address().at(9))
               << ":"
               << int(address.large_locator().address().at(10))
               << int(address.large_locator().address().at(11))
               << ":"
               << int(address.large_locator().address().at(12))
               << int(address.large_locator().address().at(13))
               << ":"
               << int(address.large_locator().address().at(14))
               << int(address.large_locator().address().at(15))
               << "]:" << std::dec
               << address.large_locator().port();
        default:
            break;
    } 
    return os;
}

#endif // UXR__AGENT__TRANSPORT__UTIL__INTERFACE_HPP_