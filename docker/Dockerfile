FROM ubuntu:latest
RUN apt-get update -y
RUN apt-get install -y \
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
COPY assets/hiredis.tgz /tmp
RUN cd /tmp &&\
    tar xzf hiredis.tgz &&\
    cd hiredis &&\
    make &&\
    make install
RUN pip3 install hyperon-das
ENV C_INCLUDE_PATH=/usr/include/libmongoc-1.0:/usr/include/libbson-1.0:/usr/include/hiredis/
RUN mkdir /opt/das-metta-parser
VOLUME /opt/das-metta-parser
WORKDIR /opt/das-metta-parser
