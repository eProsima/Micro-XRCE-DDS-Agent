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

#ifndef PUBLISHER_H_
#define PUBLISHER_H_

#include <agent/Common.h>

namespace eprosima {
namespace micrortps {

class Publisher : public XRCEObject
{
public:
    Publisher();
    virtual ~Publisher() = default;

   
    void add_writer(XRCEObject*);
    XRCEObject* get_writer();

private:
    XRCEObject* writer_;
    
};

} /* namespace micrortps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
