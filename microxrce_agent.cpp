// Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <uxr/agent/utils/CLI.hpp>

int main(int argc, char** argv)
{
    std::cout << "Enter 'q' for exit" << std::endl;

    /* CLI application. */
    CLI::App app("eProsima Micro XRCE-DDS Agent");
    app.require_subcommand(0, 1);
    app.get_formatter()->column_width(42);

    /* CLI subcommands. */
    eprosima::uxr::cli::UDPSubcommand udp_subcommand(app);
    eprosima::uxr::cli::TCPSubcommand tcp_subcommand(app);
    eprosima::uxr::cli::SerialSubcommand serial_subcommand(app);
    eprosima::uxr::cli::PseudoSerialSubcommand pseudo_serial_subcommand(app);

    /* CLI parse. */
    CLI11_PARSE(app, argc, argv);

    /* Waiting until exit. */
    std::cin.clear();
    char exit_flag = 0;
    while ('q' != exit_flag)
    {
        std::cin >> exit_flag;
    }

    return 0;
}
