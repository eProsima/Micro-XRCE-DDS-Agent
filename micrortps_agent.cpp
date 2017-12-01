
// Copyright 2017 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
