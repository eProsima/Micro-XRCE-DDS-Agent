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

    /* CLI application. */
    CLI::App app("eProsima Micro XRCE-DDS Agent");
    app.require_subcommand(1, 1);
    app.get_formatter()->column_width(42);

    /* CLI subcommands. */
    eprosima::uxr::cli::UDPSubcommand udp_subcommand(app);
    eprosima::uxr::cli::TCPSubcommand tcp_subcommand(app);
#ifndef _WIN32
    eprosima::uxr::cli::SerialSubcommand serial_subcommand(app);
    eprosima::uxr::cli::PseudoSerialSubcommand pseudo_serial_subcommand(app);
#endif
    eprosima::uxr::cli::ExitSubcommand exit_subcommand(app);

    /* CLI parse. */
    std::string cli_input{};
    for (int i = 0; i < argc; ++i)
    {
        cli_input.append(argv[i]);
        cli_input.append(" ");
    }

    while (true)
    {
        try
        {
            app.parse(cli_input);
            break;
        }
        catch (const CLI::ParseError& e)
        {
            app.exit(e);
            std::cin.clear();
            std::cout << std::endl;
            std::cout << "Enter command: ";
            std::getline(std::cin, cli_input);
        }
    }

    /* Waiting until exit. */
    std::cin.clear();
    char exit_flag = 0;
    while ('q' != exit_flag)
    {
        std::cin >> exit_flag;
    }

    return 0;
}
