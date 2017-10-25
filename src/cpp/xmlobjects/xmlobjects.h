#ifndef _XML_OBJECTS_H
#define _XML_OBJECTS_H

#include <cstddef>

namespace eprosima {

namespace fastrtps {
class ParticipantAttributes;
class PublisherAttributes;
class SubscriberAttributes;
}
namespace micrortps {
namespace xmlobjects {

bool parse_participant(const char* source, std::size_t source_size, eprosima::fastrtps::ParticipantAttributes& participant);
bool parse_publisher(const char* source, std::size_t source_size, eprosima::fastrtps::PublisherAttributes& publisher);
bool parse_subscriber(const char* source, std::size_t source_size, eprosima::fastrtps::SubscriberAttributes& subscriber);

} // namespace xmlobjects
} // namespace micrortps
} // namespace eprosima

#endif // !_XML_OBJECTS_H
