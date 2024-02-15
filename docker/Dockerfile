FROM ubuntu:latest
RUN apt-get update -y && apt-get install -y \
    build-essential \
    bison \
    flex \
    libmongoc-dev \
    libhiredis-dev\
    git \
    cmake \
    python3-pip \
    libmbedtls14 \
    libmbedtls-dev
RUN \
    cd /tmp &&\
    git clone https://github.com/sewenew/redis-plus-plus.git &&\
    cd redis-plus-plus &&\
    mkdir build &&\
    cd build &&\
    cmake .. &&\
    make &&\
    make install
RUN pip3 install hyperon-das
ENV C_INCLUDE_PATH=/usr/include/libmongoc-1.0:/usr/include/libbson-1.0
RUN mkdir /opt/das-metta-parser
VOLUME /opt/das-metta-parser
WORKDIR /opt/das-metta-parser
