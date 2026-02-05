#!/bin/bash

DIR=`dirname $2`
FILE=`basename $2`
CONTAINER_NAME="das-metta-parser-run"

docker run \
    --net="host" \
    --name=$CONTAINER_NAME \
    --env DAS_REDIS_HOSTNAME="139.84.157.32" \
    --env DAS_REDIS_PORT=40020 \
    --env DAS_MONGODB_HOSTNAME="0.0.0.0" \
    --env DAS_MONGODB_PORT=40021 \
    --env DAS_MONGODB_USERNAME="admin" \
    --env DAS_MONGODB_PASSWORD="admin" \
    --volume $DIR:/opt/data \
    --volume .:/opt/das-metta-parser \
    -it das-metta-parser-builder \
    bin/$1 /opt/data/$FILE

sleep 1
docker rm $CONTAINER_NAME >& /dev/null
