
#include <agent/Root.h>

int main(int argc, char** argv)
{
    if(argc > 2)
    {
        eprosima::micrortps::Agent* micrortps_agent = eprosima::micrortps::root();
        if(strcmp(argv[1], "serial") == 0)
        {
            // "/dev/ttyACM0"
            micrortps_agent->init(argv[2]);
        }
        else if(strcmp(argv[1], "udp") == 0 && argc == 5)
        {
            micrortps_agent->init(argv[2], atoi(argv[3]), atoi(argv[4]));
        }
        micrortps_agent->run();
    }
    else
    {
        std::cout << "Help: program [serial | udp recv_port send_port]" << std::endl;
    }
    return 0;
}
