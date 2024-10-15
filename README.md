# Micro XRCE-DDS Agent

[![Releases](https://img.shields.io/github/release/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/releases)
[![License](https://img.shields.io/github/license/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/stargazers)
[![Read the Docs](https://img.shields.io/readthedocs/micro-xrce-dds?style=flat)](https://micro-xrce-dds.docs.eprosima.com/en/latest/)
[![Twitter Follow](https://img.shields.io/twitter/follow/eprosima?style=social)](https://twitter.com/EProsima)

[![Docker Build Status](https://img.shields.io/docker/cloud/build/eprosima/micro-xrce-dds-agent)](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*eProsima Micro XRCE-DDS* is a library implementing the [DDS-XRCE protocol](https://www.omg.org/spec/DDS-XRCE/About-DDS-XRCE/) as defined and maintained by the OMG, whose aim is to allow resource constrained devices such as microcontrollers to communicate with the [DDS](https://www.omg.org/spec/DDS/About-DDS/) world as any other DDS actor would do.
It follows a client/server paradigm and is composed by two libraries, the *Micro XRCE-DDS Client* and the *Micro XRCE-DDS Agent*. The *Micro XRCE-DDS Clients* are lightweight entities meant to be compiled on e**X**tremely **R**esource **C**onstrained **E**nvironments, while the *Micro XRCE-DDS Agent* is a broker which bridges the *Clients* with the DDS world.

<p align="center"> <img src="https://github.com/eProsima/Micro-XRCE-DDS-Agent/blob/master/docs/General.png?raw=true" alt="General architecture" width="70%"/> </p>

The *Micro XRCE-DDS Agent* receives messages containing request operations from the *Clients* to publish and subscribe to topics in the DDS global dataspace. Remote procedure calls, as defined by the [DDS-RPC standard](https://www.omg.org/spec/DDS-RPC/About-DDS-RPC/), are also supported, allowing to communicate according to a request/reply paradigm.
The *Agent* then processes these requests and sends back a response with the operation status result and with the requested data, in the case of subscribe/reply operations.

*Agents* keep track of the *Clients* by means of a dedicated `ProxyClient` entity that acts on behalf of the latter.
This is made possible by the creation of *DDS Entities* on the *Agent* as a result of *Clients*' operations, such as *Participants*, *Topics*, *Publishers*, and *Subscribers*, which can interact with the DDS global dataspace.

<p align="center"> <img src="https://github.com/eProsima/Micro-XRCE-DDS-Agent/blob/master/docs/Agent.png?raw=true" alt="Agent architecture" width="80%"/> </p>

The communication between a *Micro XRCE-DDS Client* and a *Micro XRCE-DDS Agent* is achieved by means of several kinds of built-in transports: **UDPv4**, **UDPv6**, **TCPv4**, **TCPv6** and **Serial** communication. In addition, there is the possibility for the user to generate its own **Custom** transport.
An *Agent* using any of these built-in transports can be launched by means of the standalone executable generated when building the project, which comes with a built-in CLI tool to select among the transports listed above.

This built-in *Agent* can also be installed and launched using the provided [Snap package](https://snapcraft.io/micro-xrce-dds-agent) or the provided [Docker image](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/).

## Commercial support

Looking for commercial support? Write us to info@eprosima.com

Find more about us at [eProsima’s webpage](https://eprosima.com/).

## Documentation

You can access the *eProsima Micro XRCE-DDS* user documentation online, which is hosted on Read the Docs.

* [Start Page](http://micro-xrce-dds.readthedocs.io)
* [Installation manual](http://micro-xrce-dds.readthedocs.io/en/latest/installation.html)
* [User manual](http://micro-xrce-dds.readthedocs.io/en/latest/introduction.html)
