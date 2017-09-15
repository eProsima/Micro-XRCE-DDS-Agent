#ifndef ROOT_H
#define ROOT_H

namespace eprosima{
namespace micrortps{

class XRCEAgent;
XRCEAgent& root();

class XRCEAgent
{
public:
    XRCEAgent();
    ~XRCEAgent();
};
} // eprosima
} // micrortps


#endif //_ROOT_H