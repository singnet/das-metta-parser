#!/bin/bash

echo "tests/data/special_cases.metta" &&\
./scripts/run.sh syntax_check ./tests/data/special_cases.metta &&\
echo "tests/data/animals.metta" &&\
./scripts/run.sh syntax_check ./tests/data/animals.metta &&\
echo "tests/data/type_defs.metta" &&\
./scripts/run.sh syntax_check ./tests/data/type_defs.metta &&\
echo "tests/data/gencode_nodes.metta" &&\
./scripts/run.sh syntax_check ./tests/data/gencode_nodes.metta &&\
echo "tests/data/gencode_edges.metta" &&\
./scripts/run.sh syntax_check ./tests/data/gencode_edges.metta &&\
echo "tests/data/hocomoco_nodes.metta" &&\
./scripts/run.sh syntax_check ./tests/data/hocomoco_nodes.metta
