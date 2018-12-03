#########################################################################################
# Micro XRCE-DDS Agent Docker
#########################################################################################

FROM ubuntu
WORKDIR /root

# Essentials
RUN apt-get update
RUN apt-get install -y \
            software-properties-common \
            build-essential \
            cmake \
            git

# Java
RUN echo oracle-java8-installer shared/accepted-oracle-license-v1-1 select true | debconf-set-selections 
RUN add-apt-repository ppa:webupd8team/java 
RUN apt-get update 
RUN apt-get install -y oracle-java8-installer
RUN rm -rf /var/cache/oracle-jdk8-installer

# Gradle
RUN apt-get install -y gradle
RUN apt-get clean

# Clone and install Micro XRCE-DDS Agent
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS-Agent.git agent && \
    mkdir agent/build && cd agent/build && \
    cmake -DTHIRDPARTY=ON -DVERBOSE=ON .. && \
    make install

RUN ldconfig
