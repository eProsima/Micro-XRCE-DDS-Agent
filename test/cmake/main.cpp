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

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPServerWindows.hpp>
#include <uxr/agent/transport/tcp/TCPServerWindows.hpp>
#else
#include <uxr/agent/transport/serial/SerialServerLinux.hpp>
#include <uxr/agent/transport/udp/UDPServerLinux.hpp>
#include <uxr/agent/transport/tcp/TCPServerLinux.hpp>
#endif //_WIN32

int main(int argc, char** argv)
{

    eprosima::uxr::UDPServer* udp_server = new eprosima::uxr::UDPServer(2018);
    eprosima::uxr::TCPServer* tcp_server = new eprosima::uxr::TCPServer(2018);
#ifndef _WIN32
    eprosima::uxr::SerialServer* serial_server = new eprosima::uxr::SerialServer(1, 0);
#endif //_WIN32
    return 0;
}

