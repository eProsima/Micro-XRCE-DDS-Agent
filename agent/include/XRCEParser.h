#ifndef _XRCE_PARSER_H_
#define _XRCE_PARSER_H_

#include "Serializer.h"

#include <functional>

class CREATE_PAYLOAD;
class DELETE_PAYLOAD;
class WRITE_DATA_PAYLOAD;
class READ_DATA_PAYLOAD;

class XRCEListener {
public:
    XRCEListener() {};
    virtual ~XRCEListener() {};

    virtual void on_create(const CREATE_PAYLOAD& create_payload) = 0;
    virtual void on_delete(const DELETE_PAYLOAD& create_payload) = 0;
    virtual void on_write(const WRITE_DATA_PAYLOAD&  write_payload) = 0;
    virtual void on_read(const READ_DATA_PAYLOAD&   read_payload) = 0;
};

class XRCEParser
{
public:
    XRCEParser(char* buffer, size_t size, XRCEListener* listener)
        : deserializer_(buffer, size),
        listener_(listener)
    {

    }
    bool parse();
private:

    bool process_create();
    bool process_write_data();
    bool process_read_data();

    XRCEListener* listener_ = nullptr;
    Serializer deserializer_;
};

#endif // !_XRCE_PARSER_H_
