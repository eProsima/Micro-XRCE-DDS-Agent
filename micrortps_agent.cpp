#include <agent/Root.h>

int main(int argc, char** argv)
{
    eprosima::micrortps::Agent* micrortps_agent = eprosima::micrortps::root();
    micrortps_agent->init();

    micrortps_agent->run();
    return 0;
}