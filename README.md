# Micro RTPS Agent

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

*Micro RTPS Agent* acts as a server between DDS Network and *Micro RTPS Clients*. *Micro RTPS Agents* receive messages containing Operations from Clients. Agents keep track of the Clients and the *Micro RTPS Entities* they create. The Agent uses the Entities to interact with DDS Global Data Space on behalf of the Client.

The communication between a *Micro RTPS Client* and a *Micro RTPS Agent* supports two kind transports: UDP or SerialPort. While running *Micro RTPS Agent* will attend any received request from your *Micro RTPS Clients*. *Micro RTPS Agent* answers back with the result of a request each time a request is attended.

![Architecture](docs/architecture_agent.png)

Installation from Sources
=========================

Clone the project from Github: ::

    $ git clone --recursive https://github.com/eProsima/micro-RTPS-agent.git
    $ mkdir micro-RTPS-agent/build && cd micro-RTPS-agent/build

On Linux, execute: ::

    $ cmake -DTHIRDPARTY=ON ..
    $ make
    $ sudo make install

Now you have micrortps_agent installed in your system. Before running it, you need to add /usr/local/lib to the dynamic loader-linker directories. ::

    sudo ldconfig /usr/local/lib/

Run an Agent
============

To run the *Micro RTPS Agent*, you need to launch it executing one of the following options:

For running it on UDP.  ::

    $ ./micrortps_agent udp <local_ip> <out_UDP_port> <in_UDP_port>

For running it on SerialPort.  ::

    $ ./micrortps_agent serial <serial_Device>

## Documentation

You can access Micro-RTPS documentation online, which is hosted on [Read the Docs](http://micro-rtps.readthedocs.io).

* [Start Page](http://micro-rtps.readthedocs.io)
* [Installation manual](http://micro-rtps.readthedocs.io/en/latest/dependencies.html)
* [User manual](http://micro-rtps.readthedocs.io/en/latest/introduction.html)

## Getting Help

If you need support you can reach us by mail at `support@eProsima.com` or by phone at `+34 91 804 34 48`.
