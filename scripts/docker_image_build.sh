#!/bin/bash

docker buildx build -t das-metta-parser-builder --load -f docker/Dockerfile .
