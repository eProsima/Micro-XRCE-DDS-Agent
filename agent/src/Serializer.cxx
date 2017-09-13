#include "Serializer.h"

#include "MessageHeader.h"
#include "SubMessageHeader.h"
#include "Payloads.h"

Serializer::Serializer(char* const buffer_, size_t buffer_size) :
	fastbuffer_(buffer_, buffer_size),
	serializer_(fastbuffer_, eprosima::fastcdr::Cdr::DEFAULT_ENDIAN, eprosima::fastcdr::Cdr::DDS_CDR)	
{
}

template<class T>
bool Serializer::serialize(const T& data)
{
	try
	{
		data.serialize(serializer_); // Serialize the object:
	}
	catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
	{
		return false;
	}
	return true;
}

template bool Serializer::serialize(const MessageHeader& data);
template bool Serializer::serialize(const SubmessageHeader& data);
template bool Serializer::serialize(const CREATE_PAYLOAD& data);
template bool Serializer::serialize(const RESOURCE_STATUS_PAYLOAD& data);
template bool Serializer::serialize(const DATA_PAYLOAD& data);
template bool Serializer::serialize(const READ_DATA_PAYLOAD& data);
template bool Serializer::serialize(const WRITE_DATA_PAYLOAD& data);

template<class T> bool Serializer::deserialize(T& data)
{
	try
	{
		data.deserialize(serializer_); //Deserialize the object:
	}
	catch (eprosima::fastcdr::exception::NotEnoughMemoryException& /*exception*/)
	{
		return false;
	}
	return true;
}

template bool Serializer::deserialize(MessageHeader& data);
template bool Serializer::deserialize(SubmessageHeader& data);
template bool Serializer::deserialize(CREATE_PAYLOAD& data);
template bool Serializer::deserialize(RESOURCE_STATUS_PAYLOAD& data);
template bool Serializer::deserialize(DATA_PAYLOAD& data);
template bool Serializer::deserialize(READ_DATA_PAYLOAD& data);
template bool Serializer::deserialize(WRITE_DATA_PAYLOAD& data);


size_t Serializer::get_serialized_size()
{
	return serializer_.getSerializedDataLength();
}

bool Serializer::bufferEnd()
{
	return fastbuffer_.getBufferSize() == serializer_.getSerializedDataLength();
}

void Serializer::align()
{

}