#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

class OBJK_CLIENT_Representation;

namespace eprosima{
namespace micrortps{

class ProxyClient
{
public:
    ProxyClient() = default;
    ProxyClient(const OBJK_CLIENT_Representation& client);
    ~ProxyClient() = default;

private:

};
} // eprosima
} // micrortps

#endif //PROXY_CLIENT_H