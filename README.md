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
