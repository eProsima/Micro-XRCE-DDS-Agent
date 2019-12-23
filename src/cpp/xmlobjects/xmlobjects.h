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

#ifndef _XML_OBJECTS_H
#define _XML_OBJECTS_H

#include <cstddef>
#include <string>

namespace eprosima {

namespace fastrtps {
class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;
class TopicAttributes;
class RequesterAttributes;
class ReplierAttributes;
} // namespace fastrtps
namespace uxr {
namespace xmlobjects {

bool parse_participant(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::ParticipantAttributes& participant);

bool parse_publisher(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::PublisherAttributes& publisher);

bool parse_subscriber(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::SubscriberAttributes& subscriber);

bool parse_topic(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::TopicAttributes& topic);

bool parse_requester(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::RequesterAttributes& requester);

bool parse_replier(
    const char* source,
    std::size_t source_size,
    eprosima::fastrtps::ReplierAttributes& replier);

} // namespace xmlobjects
} // namespace uxr
} // namespace eprosima

#endif // !_XML_OBJECTS_H
