#!/bin/bash

CONTAINER_NAME="das-metta-parser-bash"

docker run \
    --net="host" \
    --name=$CONTAINER_NAME \
    --volume /tmp:/tmp \
    --volume .:/opt/das-metta-parser \
    -it das-metta-parser-builder \
    bash

sleep 1
docker rm $CONTAINER_NAME >& /dev/null
