#ifndef _XRCE_FACTORY_H_
#define _XRCE_FACTORY_H_

#include <stdint.h>

#include "Serializer.h"

class RESOURCE_STATUS_PAYLOAD;
class DATA_PAYLOAD;

class XRCEFactory
{
public:
    XRCEFactory(char* buffer, uint32_t max_size) : serializer_(buffer, max_size) {};
    void header(int32_t client_key, uint8_t session_id, uint8_t stream_id, uint16_t sequence_nr);
    void status(const RESOURCE_STATUS_PAYLOAD& payload);
    void data(const DATA_PAYLOAD& payload);
    size_t get_total_size();
private:

    void submessage_header(uint8_t submessage_id, uint8_t flags, uint16_t submessage_length);
    Serializer serializer_;
};

#endif // !_XRCE_FACTORY_H