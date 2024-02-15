#!/usr/bin/python3

import sys
from hyperon_das_atomdb.utils.expression_hasher import ExpressionHasher

if len(sys.argv) < 2:
    exit(1)
elif len(sys.argv) == 2:
    handle = ExpressionHasher.named_type_hash(sys.argv[1])
elif len(sys.argv) == 3:
    handle = ExpressionHasher.terminal_hash(sys.argv[1], sys.argv[2])
else:
    handle = ExpressionHasher.expression_hash(sys.argv[1], sys.argv[2:])
print(handle)
