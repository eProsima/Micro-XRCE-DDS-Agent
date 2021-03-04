# Micro XRCE-DDS Agent

[![Releases](https://img.shields.io/github/release/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/releases)
[![License](https://img.shields.io/github/license/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/stargazers)

[![Docker Build Status](https://img.shields.io/docker/cloud/build/eprosima/micro-xrce-dds-agent)](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*The Micro XRCE-DDS Agent* acts as a server to bridge the DDS dataspace network with *Micro XRCE-DDS Client* applications.

The *Micro XRCE-DDS Agents* receive messages containing request operations from the *Clients* to publish and subscribe to topics in the DDS global dataspace. Remote procedure calls, as defined by the DDS-RPC standard, are also supported, allowing to communicate according to a request/reply paradigm.
The *Agents* then process these XRCE requests and send back a response with the operation status result and with the requested data, in the case of subscribe/reply operations.

*Agents* keep track of the *Clients* by means of a dedicated *ProxyClient* entity that acts on behalf of the latter.
This is made possible by the creation of *DDS Entities* on the *Agent* as a result of *Clients*' operations, such as *Participants*, *Topics*, *Publishers*, and *Subscribers*, which can interact with the DDS Global dataspace.

The communication between a *Micro XRCE-DDS Client* and a *Micro XRCE-DDS Agent* is achieved by means of several kinds of built-in transports: **UDPv4**, **UDPv6**, **TCPv4**, **TCPv6** and **Serial** communication. In addition, there is the possibility for the user to generate its own **Custom** transport.
You can use an *Agent* with these transports by means of the standalone executable generated when building the project, which comes with a built-in CLI tool to select one of the transports listed above.
This built-in *Agent* can also be installed and launched using the provided [Snap package](https://snapcraft.io/micro-xrce-dds-agent) or the provided [Docker image](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/).

![Architecture](docs/agent_architecture.png)

## Documentation

You can access *Micro XRCE-DDS* documentation online, which is hosted on Read the Docs.

* [Start Page](http://micro-xrce-dds.readthedocs.io)
* [Installation manual](http://micro-xrce-dds.readthedocs.io/en/latest/installation.html)
* [User manual](http://micro-xrce-dds.readthedocs.io/en/latest/introduction.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
