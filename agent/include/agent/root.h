#ifndef ROOT_H
#define ROOT_H

#include <agent/ObjectVariant.h>
#include <agent/client/ProxyClient.h>

#include <vector>

namespace eprosima{
namespace micrortps{

class Agent;
Agent& root();

class Agent
{
public:
    Agent();
    ~Agent() = default;

    ResultStatus create_client(const OBJK_CLIENT_Representation& client_representation);
    void delete_client();
private:
    std::vector<ProxyClient> clients_;
};

} // eprosima
} // micrortps

#endif //_ROOT_H