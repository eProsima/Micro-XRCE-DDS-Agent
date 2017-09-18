#ifndef ROOT_H
#define ROOT_H

#include <agent/ObjectVariant.h>
#include <agent/client/ProxyClient.h>

#include <map>

namespace eprosima{
namespace micrortps{

class Agent;
Agent& root();

class Agent
{
public:
    Agent();
    ~Agent() = default;

    Status create_client(const ClientKey& client_key,  const ObjectVariant& client_representation);
    Status delete_client(const ClientKey& client_key);
private:
    std::map<ClientKey, ProxyClient> clients_;
};

} // eprosima
} // micrortps

#endif //_ROOT_H