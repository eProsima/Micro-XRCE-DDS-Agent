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

#ifndef SUBSCRIBER_H_
#define SUBSCRIBER_H_

#include <agent/XRCEObject.hpp>

namespace eprosima {
namespace micrortps {

class Subscriber : public XRCEObject
{
public:
    Subscriber(const dds::xrce::ObjectId& id, XRCEObject& participant);
    virtual ~Subscriber() = default;

    XRCEObject& get_participant();

private:
    XRCEObject& participant_;
};

} /* namespace micrortps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
