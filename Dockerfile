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
RUN apt install -y openjdk-8-jdk
ENV JAVA_HOME "/usr/lib/jvm/java-8-openjdk-amd64/"

# Gradle
RUN apt-get install -y gradle
RUN apt-get clean

# Clone and install Micro XRCE-DDS Agent
RUN git clone https://github.com/eProsima/Micro-XRCE-DDS-Agent.git agent && \
    mkdir agent/build && cd agent/build && \
    cmake -DTHIRDPARTY=ON -DVERBOSE=ON .. && \
    make install

RUN ldconfig
