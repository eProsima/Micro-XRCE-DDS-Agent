#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

#include <agent/ObjectVariant.h>

#include <map>

namespace eprosima{
namespace micrortps{

using InternalObjectId = std::array<uint8_t, 4>;

class XRCEObject
{
public:
    virtual ~XRCEObject() = default;
};
class TestObjectP : public XRCEObject
{

};
class TestObjectS : public XRCEObject
{

};

class ProxyClient
{
public:
    ProxyClient() = default;
    ProxyClient(const OBJK_CLIENT_Representation& client);
    ~ProxyClient();

    Status create(const CreationMode& creation_mode, const ObjectId& object_id, const ObjectVariant& representation);
    Status update(const ObjectId& object_id, const ObjectVariant& representation);
    Info get_info(const ObjectId& object_id);
    Status delete_object(const ObjectId& object_id);
private:

    OBJK_CLIENT_Representation representation_;

    std::map<InternalObjectId, XRCEObject*> _objects;

    bool create(const InternalObjectId& internal_id, const ObjectVariant& representation);

};
} // eprosima
} // micrortps

#endif //PROXY_CLIENT_H