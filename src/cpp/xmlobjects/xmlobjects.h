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
} // namespace fastrtps
namespace micrortps {
namespace xmlobjects {

bool parse_participant(const char* source, std::size_t source_size,
                       eprosima::fastrtps::ParticipantAttributes& participant);
bool parse_publisher(const char* source, std::size_t source_size, eprosima::fastrtps::PublisherAttributes& publisher);
bool parse_subscriber(const char* source, std::size_t source_size,
                      eprosima::fastrtps::SubscriberAttributes& subscriber);
bool parse_topic(const char* source, std::size_t source_size, eprosima::fastrtps::TopicAttributes& topic);

} // namespace xmlobjects
} // namespace micrortps
} // namespace eprosima

#endif // !_XML_OBJECTS_H
