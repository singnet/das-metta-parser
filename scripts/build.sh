#!/bin/bash

mkdir -p bin
docker run \
    --volume .:/opt/das-metta-parser \
    --workdir /opt/das-metta-parser/src \
    das-metta-parser-builder \
    make syntax_check db_loader
