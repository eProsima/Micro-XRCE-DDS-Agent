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

#ifndef PARTICIPANT_H_
#define PARTICIPANT_H_

#include <string>

#include <agent/Common.h>
#include <agent/Payloads.h>
#include <agent/types/ShapePubSubTypes.h>
#include <agent/datareader/DataReader.h>

namespace eprosima {

namespace fastrtps {
    class Participant;
}

namespace micrortps {

/**
 * Class Participana, modules DDS participant.
 * @ingroup MICRORTPS_MODULE
 */
class XRCEParticipant : public XRCEObject
{
public:
    XRCEParticipant();
    virtual ~XRCEParticipant();

    bool init();

    fastrtps::Participant* get_participant() { return mp_rtps_participant;};

    XRCEObject* create_publisher() {}
    XRCEObject* create_subscriber() {}
    XRCEObject* create_writer();
    XRCEObject* create_reader(ReaderListener* message_listener);

    XRCEObject* get_writer();
    XRCEObject* get_reader();

private:

    eprosima::fastrtps::Participant* mp_rtps_participant;
    XRCEObject* writer_;
    XRCEObject* reader_;
};

} /* namespace micrortps */
} /* namespace eprosima */

#endif /* PARTICIPANT_H_ */
