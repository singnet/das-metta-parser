#!/bin/bash

#cat tests/data/edges.metta | bin/syntax_check &&\
cat tests/data/type_defs.metta | bin/syntax_check &&\
cat tests/data/gencode_nodes.metta | bin/syntax_check &&\
cat tests/data/gencode_edges.metta | bin/syntax_check
