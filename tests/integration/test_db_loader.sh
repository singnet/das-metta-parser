#!/bin/bash

echo "tests/data/special_cases.metta" &&\
./bin/db_loader tests/data/special_cases.metta &&\
echo "tests/data/animals.metta" &&\
./bin/db_loader tests/data/animals.metta &&\
echo "tests/data/type_defs.metta" &&\
./bin/db_loader tests/data/type_defs.metta &&\
echo "tests/data/gencode_edges.metta" &&\
./bin/db_loader tests/data/gencode_edges.metta
