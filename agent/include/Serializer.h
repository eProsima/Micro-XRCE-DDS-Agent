#ifndef _SERIALIZER_H
#define _SERIALIZER_H



#include <stdint.h>

#include "fastcdr/Cdr.h"
#include "fastcdr/FastCdr.h"
#include "fastcdr/exceptions/Exception.h"

class Serializer
{
public:
    Serializer(char* const buffer_, size_t buffer_size );
    
    template<class T> bool serialize(const T& data);

    template<class T> bool deserialize(T& data);

    size_t get_serialized_size();
    bool bufferEnd();
private:

    void align();

    eprosima::fastcdr::FastBuffer fastbuffer_; // Object that manages the raw buffer.
    eprosima::fastcdr::Cdr serializer_; // Object that serializes the data.

};
#endif // !_SERIALIZER_H