FROM ubuntu:latest
RUN apt-get update -y
RUN apt-get install -y \
    build-essential \
    bison \
    flex \
    libmongoc-dev \
    git \
    cmake \
    python3-pip \
    libmbedtls14 \
    libmbedtls-dev \
    libevent-dev \
    libssl-dev \
    pkg-config \
    cmake-data
COPY assets/hiredis-cluster.tgz /tmp
RUN cd /tmp &&\
    tar xzf hiredis-cluster.tgz &&\
    cd hiredis-cluster &&\
    mkdir build &&\
    cd build &&\
    cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_SSL=ON ..&&\
    make &&\
    make install
RUN pip3 install hyperon-das
ENV C_INCLUDE_PATH=/usr/include/libmongoc-1.0:/usr/include/libbson-1.0
RUN mkdir /opt/das-metta-parser
VOLUME /opt/das-metta-parser
WORKDIR /opt/das-metta-parser
