# das-metta-parser
A Flex/Bison parser for MeTTa knowledge base files.

## How to build

```
$ ./scripts/docker_image_build.sh
$ ./scripts/build.sh
```

On success, parser binaries will be placed on `bin/`. However, these binaries use a bunch of libraries so they are supposed to run inside docker containers.

## How to run

There are 3 executables in `./bin`. They are supposed to be executed from inside a container (to use them without docker you should take a look at `docker/Dockerfile` and install all the dependencies in your environment).

* `syntax_check` - Perform a syntax check in the file. No action is taken (i.e. no atom is created anywhere). This is useful to check the syntax of huge files before actually trying to load them.
* `db_loader` - Load a MeTTa file into a MongoDB/Redis environment. This environment is supposed to be set up properly before runing the loader. See details in https://github.com/singnet/das-toolbox
* `db_loader_redis_cluster` - The same as the previous but load to a Redis Cluster environment instead of a stand-alone single-server Redis.

NB you are expected to pass a single file to `run.sh`. No directories, no wildcards.

```
./scripts/run.sh syntax_check /path/to/file.metta
./scripts/run.sh db_loader /path/to/file.metta
./scripts/run.sh db_loader_redis_cluster /path/to/file.metta
```

## Mapping MeTTa expressions to Atomspace nodes and links

MeTTa expressions are mapped to nodes and links using only two types of atoms:

* Symbols (nodes)
* Expressions (links)

E.g. an expression like:

```
(A B (C D))
```

Is mapped to 4 nodes and 2 links like this:

```
Expression
    Symbol 'A'
    Symbol 'B'
    Expression
        Symbol 'C'
        Symbol 'D'
```

Literals are mapped likewise. For instance, this expression:

```
(A "text text text")
```

is mapped like this:

```
Expression
    Symbol 'A'
    Symbol '"text text text"'
```

NB the double quotes `"` are part of the Symbol name. In addition to this,
literals are marked as such with a `is_literal` flag in the Python dict which
represents the atom. Numeric literals are similar:

```
(A 0.5)
```

is mapped like this:

```
Expression
    Symbol A
    Symbol '0.5'
```

In this case, the name of the Symbol node is the string '0.5', but in adition
to the `is_literal` flag, an extra field `value_as_float` is added to the
Python dict which represents the atom. It's the same for integer literals,
which gain an extra `value_as_int` field.

Type definitions are treated in a similar fashion:

```
(: A Concept)
```

is mapped to:

```
Expression
    Symbol ':'
    Symbol 'A'
    Symbol 'Concept'
```

However, type definitions have a side-effect which is the creation of an extra link of type MettaType:

```
MettaType
    Symbol 'A'
    Symbol 'Concept'
```

NB Type definitions may have expressions in the place of symbols. For instance:

```
(: A (B C))
```

In this case, in addition to the usual Expression and Symbol atoms, a MettaType link will be created like this:

```
MettaType
    Symbol 'A'
    Expression
        Symbol 'B'
        Symbol 'C'
```
