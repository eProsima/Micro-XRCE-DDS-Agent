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

/**
 * @file DataReader.h
 */


#ifndef DATAREADER_H_
#define DATAREADER_H_


#include <fastrtps/participant/Participant.h>
#include <fastrtps/subscriber/Subscriber.h>

#include <agent/types/ShapePubSubTypes.h>
#include <agent/Payloads.h>
#include <agent/Common.h>

namespace eprosima {
namespace micrortps {

class ReaderListener
{
public:
    ReaderListener() = default;
    virtual ~ReaderListener() = default;

    virtual void on_read_data(const ObjectId& object_id, const RequestId& req_id, const octet* data, const size_t length) = 0;
};

/**
 * Class DataReader, contains the public API that allows the user to control the reception of messages.
 * @ingroup MICRORTPS_MODULE
 */
class DataReader: public XRCEObject
{
public:
    DataReader() = default;
    DataReader(const ReaderListener* read_list);
    virtual ~DataReader();

    bool init();
    int read(READ_DATA_PAYLOAD &read_data);
    int cancel_read();
    int read_sample();

    bool readNextData(void* data);
    bool takeNextData(void* data);
private:

    const ReaderListener* mp_reader_listener;
    fastrtps::Participant* mp_rtps_participant;
    fastrtps::Subscriber* mp_rtps_subscriber;
    std::string m_rtps_subscriber_prof;
    ShapeTypePubSubType m_shape_type;
};



} /* namespace micrortps */
} /* namespace eprosima */

#endif /* DATAREADER_H_ */
