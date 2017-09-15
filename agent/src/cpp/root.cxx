#include "agent/root.h"

using namespace eprosima::micrortps;

XRCEAgent::XRCEAgent() {}

XRCEAgent::~XRCEAgent() {}



XRCEAgent& root()
{
    static XRCEAgent xrce_agent;
    return xrce_agent;
}