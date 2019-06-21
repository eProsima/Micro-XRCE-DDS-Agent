# Micro XRCE-DDS Agent

[![Releases](https://img.shields.io/github/release/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/releases)
[![License](https://img.shields.io/github/license/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/blob/master/LICENSE)
[![Issues](https://img.shields.io/github/issues/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Micro-XRCE-DDS-Agent.svg)](https://github.com/eProsima/Micro-XRCE-DDS-Agent/stargazers)

[![Docker Automated build](https://img.shields.io/docker/automated/eprosima/micro-xrce-dds-agent.svg?logo=docker)](https://hub.docker.com/r/meprosima/micro-xrce-dds-agent/)
[![Docker Build Status](https://img.shields.io/docker/cloud/build/eprosima/micro-xrce-dds-agent.svg?logo=docker)](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/)
[![Compare Images](https://images.microbadger.com/badges/image/eprosima/micro-xrce-dds-agent.svg)](https://microbadger.com/eprosima/micro-xrce-dds-agent/)

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*Micro XRCE-DDS Agent* acts as a server between DDS Network and *Micro XRCE-DDS Clients*.
*Micro XRCE-DDS Agents* receive messages containing Operations from Clients.
Agents keep track of the Clients and the *Micro XRCE-DDS Entities* they create.
The Agent uses the Entities to interact with DDS Global Data Space on behalf of the Client.

The communication between a *Micro XRCE-DDS Client* and a *Micro XRCE-DDS Agent* supports two kind transports: UDP or SerialPort.
While running *Micro XRCE-DDS Agent* will attend any received request from your *Micro XRCE-DDS Clients*. *Micro XRCE-DDS Agent* answers back with the result of a request each time a request is attended.

![Architecture](docs/architecture_agent.png)

## Documentation

You can access *Micro XRCE-DDS* documentation online, which is hosted on Read the Docs.

* [Start Page](http://micro-xrce-dds.readthedocs.io)
* [Installation manual](http://micro-xrce-dds.readthedocs.io/en/latest/installation.html)
* [User manual](http://micro-xrce-dds.readthedocs.io/en/latest/introduction.html)

## Dockerfile

There is a *Micro XRCE-DDS Agent* Dockerfile available on [Docker Hub](https://hub.docker.com/r/eprosima/micro-xrce-dds-agent/).

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
