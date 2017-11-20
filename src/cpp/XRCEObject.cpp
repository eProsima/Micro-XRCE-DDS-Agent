#include <agent/XRCEObject.hpp>

using eprosima::micrortps::XRCEObject;
using eprosima::micrortps::ObjectId;

XRCEObject::~XRCEObject()
{ }

ObjectId XRCEObject::getID() const
{
    return id_;
}