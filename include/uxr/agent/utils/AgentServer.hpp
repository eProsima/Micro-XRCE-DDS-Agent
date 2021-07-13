
#ifndef UXR_AGENT_SERVER_ARGUMENTPARSER_HPP_
#define UXR_AGENT_SERVER_ARGUMENTPARSER_HPP_

#include <sstream>
#include <fstream>
#include <csignal>
#include <type_traits>
#include <unordered_map>
#include <uxr/agent/transport/Server.hpp>

#ifdef _WIN32
#include <uxr/agent/transport/udp/UDPv4AgentWindows.hpp>
#include <uxr/agent/transport/udp/UDPv6AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv4AgentWindows.hpp>
#include <uxr/agent/transport/tcp/TCPv6AgentWindows.hpp>
#else
#include <uxr/agent/transport/udp/UDPv4AgentLinux.hpp>
#include <uxr/agent/transport/udp/UDPv6AgentLinux.hpp>
#include <uxr/agent/transport/tcp/TCPv4AgentLinux.hpp>
#include <uxr/agent/transport/tcp/TCPv6AgentLinux.hpp>
#include <uxr/agent/transport/serial/TermiosAgentLinux.hpp>
#include <uxr/agent/transport/serial/PseudoTerminalAgentLinux.hpp>
#include <uxr/agent/transport/serial/MultiTermiosAgentLinux.hpp>
#include <uxr/agent/transport/serial/baud_rate_table_linux.h>

#include <termios.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif // _WIN32

namespace eprosima {
namespace uxr {
namespace agent {

enum class TransportKind
{
    INVALID,
    UDP4,
    UDP6,
    TCP4,
    TCP6,
#ifndef _WIN32
    SERIAL,
    MULTISERIAL,
    PSEUDOTERMINAL,
#endif // _WIN32
    HELP
};

template<typename AgentType>
class AgentServer
{
public:

    AgentServer() 
    {
    }

    ~AgentServer() = default;

    void configure_agent_ipv4(uint16_t port, Middleware::Kind middleware)
    {
        agent_server_.reset(new AgentType(port, middleware));
    }

#ifndef _WIN32
    void configure_agent_serial(std::string dev, const std::string baudrate, Middleware::Kind middleware)
    {
        struct termios attr = init_termios(baudrate.c_str());
        
        agent_server_.reset(new TermiosAgent(dev.c_str(), O_RDWR | O_NOCTTY, attr, 0, middleware));
    }

    void configure_agent_multiserial(std::vector<std::string> devs, const std::string baudrate, Middleware::Kind middleware)
    {
        struct termios attr = init_termios(baudrate.c_str());

        agent_server_.reset(new MultiTermiosAgent(devs, O_RDWR | O_NOCTTY, attr, 0, middleware));
    }

    void configure_agent_pseudoterminal(const std::string baudrate, Middleware::Kind middleware)
    {
        agent_server_.reset(new PseudoTerminalAgent(O_RDWR | O_NOCTTY, baudrate.c_str(), 0, middleware));
    }

    bool enable_discovery(uint16_t discovery_port)
    {
        if (agent_server_->has_discovery())
        {
            return agent_server_->enable_discovery(discovery_port);
        }
        else
        {
            UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("Discovery protocol error"),
                "Not supported on selected transport",
                "");

            return false;
        }
    }

    bool enable_p2p(uint16_t p2p_port)
    {
        if (agent_server_->has_p2p())
        {
            return agent_server_->enable_p2p(p2p_port);
        }
        else
        {
            UXR_AGENT_LOG_WARN(
                UXR_DECORATE_YELLOW("P2P protocol error"),
                "Not supported on selected transport",
                "");

            return false;
        }
    }

    bool load_config_file(const std::string& file_path)
    {
        return agent_server_->load_config_file(file_path);
    }

    void set_verbose_level(uint8_t verbose_level)
    {
        agent_server_->set_verbose_level(verbose_level);
    }

    bool start_agent()
    {
        return agent_server_->start();
    }

    bool stop_agent()
    {
        return agent_server_->stop();
    }

    termios init_termios(const char * baudrate_str)
    {
        struct termios attr = {};

        /* Setting CONTROL OPTIONS. */
        attr.c_cflag |= unsigned(CREAD);    // Enable read.
        attr.c_cflag |= unsigned(CLOCAL);   // Set local mode.
        attr.c_cflag &= unsigned(~PARENB);  // Disable parity.
        attr.c_cflag &= unsigned(~CSTOPB);  // Set one stop bit.
        attr.c_cflag &= unsigned(~CSIZE);   // Mask the character size bits.
        attr.c_cflag |= unsigned(CS8);      // Set 8 data bits.
        attr.c_cflag &= unsigned(~CRTSCTS); // Disable hardware flow control.

        /* Setting LOCAL OPTIONS. */
        attr.c_lflag &= unsigned(~ICANON);  // Set non-canonical input.
        attr.c_lflag &= unsigned(~ECHO);    // Disable echoing of input characters.
        attr.c_lflag &= unsigned(~ECHOE);   // Disable echoing the erase character.
        attr.c_lflag &= unsigned(~ISIG);    // Disable SIGINTR, SIGSUSP, SIGDSUSP and SIGQUIT signals.

        /* Setting INPUT OPTIONS. */
        attr.c_iflag &= unsigned(~IXON);    // Disable output software flow control.
        attr.c_iflag &= unsigned(~IXOFF);   // Disable input software flow control.
        attr.c_iflag &= unsigned(~INPCK);   // Disable parity check.
        attr.c_iflag &= unsigned(~ISTRIP);  // Disable strip parity bits.
        attr.c_iflag &= unsigned(~IGNBRK);  // No ignore break condition.
        attr.c_iflag &= unsigned(~IGNCR);   // No ignore carrier return.
        attr.c_iflag &= unsigned(~INLCR);   // No map NL to CR.
        attr.c_iflag &= unsigned(~ICRNL);   // No map CR to NL.

        /* Setting OUTPUT OPTIONS. */
        attr.c_oflag &= unsigned(~OPOST);   // Set raw output.

        /* Setting OUTPUT CHARACTERS. */
        attr.c_cc[VMIN] = 1;
        attr.c_cc[VTIME] = 1;

        /* Setting baudrate. */
        speed_t baudrate = getBaudRate(baudrate_str);
        attr.c_ispeed = baudrate;
        attr.c_ospeed = baudrate;

        return attr;
    }
#endif

private:
    std::unique_ptr<AgentType> agent_server_;
};

} // namespace agent
} // namespace uxr
} // namespace eprosima

#endif // UXR_AGENT_SERVER_ARGUMENTPARSER_HPP_