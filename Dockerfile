#########################################################################################
# Micro XRCE-DDS Agent Docker
#########################################################################################

# Build stage
FROM ubuntu AS build
WORKDIR /root

# Essentials
RUN apt-get update
RUN apt-get install -y \
            software-properties-common \
            build-essential \
            cmake \
            git

RUN apt-get clean

# Prepare Micro XRCE-DDS Agent workspace
RUN mkdir -p /agent/build
COPY ./ /agent/

# Build Micro XRCE-DDS Agent and install
RUN cd /agent/build && \
    cmake -DTHIRDPARTY=ON -DVERBOSE=ON .. && \
    make install

# Prepare Micro XRCE-DDS Agent artifacts
RUN cd /usr/local/lib/ && \
    tar -czvf libmicroxrcedds_agent.tar.gz libmicroxrcedds_agent.so.* && \
    tar -czvf libfastcdr.tar.gz libfastcdr.so.* && \
    tar -czvf libfastrtps.tar.gz libfastrtps.so.*

# Final user image
FROM ubuntu
WORKDIR /root

# Copy Micro XRCE-DDS Agent build artifacts
COPY --from=build /usr/local/lib/libmicroxrcedds_agent.tar.gz  /usr/local/lib/
RUN tar -xzvf /usr/local/lib/libmicroxrcedds_agent.tar.gz -C /usr/local/lib/ && \
    rm /usr/local/lib/libmicroxrcedds_agent.tar.gz
COPY --from=build /usr/local/include/uxr /usr/local/include/uxr
COPY --from=build /usr/local/share/microxrcedds_agent/ /usr/local/share/microxrcedds_agent/
COPY --from=build /usr/local/bin/MicroXRCEAgent /usr/local/bin/MicroXRCEAgent

COPY --from=build /usr/local/lib/libfastcdr.tar.gz /usr/local/lib/
RUN tar -xzvf /usr/local/lib/libfastcdr.tar.gz -C /usr/local/lib/ && \
    rm /usr/local/lib/libfastcdr.tar.gz
COPY --from=build /usr/local/include/fastcdr /usr/local/include/fastcdr
COPY --from=build /usr/local/share/fastcdr/ /usr/local/share/fastcdr/

COPY --from=build /usr/local/lib/libfastrtps.tar.gz /usr/local/lib/
RUN tar -xzvf /usr/local/lib/libfastrtps.tar.gz -C /usr/local/lib/ && \
    rm /usr/local/lib/libfastrtps.tar.gz
COPY --from=build /usr/local/include/fastrtps /usr/local/include/fastrtps
COPY --from=build /usr/local/share/fastrtps/ /usr/local/share/fastrtps/

RUN ldconfig
