all: syntax_check db_loader db_loader_redis_cluster

metta.tab.c metta.tab.h: metta.y actions.h
	bison -t -v -d metta.y

lex.yy.c: metta.l metta.tab.h
	flex metta.l

ACTION_OBJS = syntax_check.o db_loader.o db_loader_redis_cluster.o
UTIL_OBJS = expression_hasher.o action_util.o
HEADERS = actions.h handle_list.h
LDFLAGS = -lmbedcrypto -lmongoc-1.0 -lbson-1.0 -lhiredis_cluster -lhiredis
CFLAGS = -Wall
BIN_DIR = ../bin

expression_hasher.o: expression_hasher.c expression_hasher.h
	gcc $(CFLAGS) -c expression_hasher.c

action_util.o: action_util.c action_util.h
	gcc $(CFLAGS) -c action_util.c

syntax_check.o: syntax_check.c actions.h $(HEADERS)
	gcc $(CFLAGS) -c syntax_check.c
    
db_loader.o: db_loader.c actions.h $(HEADERS)
	gcc $(CFLAGS) -c db_loader.c
    
db_loader_redis_cluster.o: db_loader.c actions.h $(HEADERS)
	gcc $(CFLAGS) -D DB_LOADER_USE_REDIS_CLUSTER -c db_loader.c -o db_loader_redis_cluster.o

syntax_check: lex.yy.c metta.tab.c metta.tab.h syntax_check.o $(UTIL_OBJS)
	gcc -o $(BIN_DIR)/syntax_check metta.tab.c lex.yy.c syntax_check.o $(UTIL_OBJS) $(LDFLAGS)

db_loader: lex.yy.c metta.tab.c metta.tab.h db_loader.o $(UTIL_OBJS)
	gcc -o $(BIN_DIR)/db_loader metta.tab.c lex.yy.c db_loader.o $(UTIL_OBJS) $(LDFLAGS)

db_loader_redis_cluster: lex.yy.c metta.tab.c metta.tab.h db_loader_redis_cluster.o $(UTIL_OBJS)
	gcc -o $(BIN_DIR)/db_loader_redis_cluster metta.tab.c lex.yy.c db_loader_redis_cluster.o $(UTIL_OBJS) $(LDFLAGS)

test_mongo.o: test_mongo.c
	gcc $(CFLAGS) -c test_mongo.c

test_mongo: test_mongo.o
	gcc -o test_mongo test_mongo.o $(LDFLAGS)

clean:
	$(RM) metta.tab.c metta.tab.o lex.yy.c lex.yy.o metta.tab.h metta.output $(ACTION_OBJS) $(UTIL_OBJS)
