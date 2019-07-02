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

#ifndef _WIN32
static bool terminate_flag = false;

static void sigterm_handler(int signo)
{
    if (signo == SIGTERM || signo == SIGINT)
    {
        terminate_flag = true;
    }
}
#endif

int main(int argc, char** argv)
{

#ifndef _WIN32
    if (signal(SIGTERM, &sigterm_handler) == SIG_ERR
       || signal(SIGINT, &sigterm_handler) == SIG_ERR)
    {
        return -1;
    }
#endif

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
    for (int i = 1; i < argc; ++i)
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
#ifdef _WIN32
    std::cin.clear();
    char exit_flag = 0;
    while ('q' != exit_flag)
    {
        std::cin >> exit_flag;
    }
#else
    /* Wait for SIGTERM/SIGINT instead, as reading from stdin may be redirected to /dev/null. */
    while (!terminate_flag)
    {
        sleep(15);
    }
#endif

    return 0;
}
