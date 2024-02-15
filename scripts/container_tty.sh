#!/bin/bash

docker run --net="host" --volume /tmp:/tmp --volume .:/opt/das-metta-parser -it das-metta-parser-builder bash
