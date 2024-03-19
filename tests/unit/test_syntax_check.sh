#!/bin/bash

echo "tests/data/special_cases.metta" &&\
./bin/syntax_check tests/data/special_cases.metta &&\
echo "tests/data/animals.metta" &&\
./bin/syntax_check tests/data/animals.metta &&\
echo "tests/data/type_defs.metta" &&\
./bin/syntax_check tests/data/type_defs.metta &&\
echo "tests/data/gencode_edges.metta" &&\
./bin/syntax_check tests/data/gencode_edges.metta
