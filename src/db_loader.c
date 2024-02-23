#include <bson/bson.h>
#include <mongoc/mongoc.h>
#include <limits.h>
#include <float.h>
#include <stdlib.h>
#include <signal.h>
#include <hiredis_cluster/hircluster.h>

#include "actions.h"
#include "limits.h"
#include "handle_list.h"
#include "action_util.h"
#include "expression_hasher.h"

#include <unistd.h>

extern int yylineno;
extern unsigned long INPUT_LINE_COUNT;

// Private stuff

// MongoDB reusable globals
static mongoc_database_t *MONGODB = NULL;
static mongoc_client_t *MONGODB_CLIENT = NULL;
static mongoc_collection_t *MONGODB_TYPES= NULL;
static mongoc_collection_t *MONGODB_NODES= NULL;
static mongoc_collection_t *MONGODB_LINKS_1= NULL;
static mongoc_collection_t *MONGODB_LINKS_2= NULL;
static mongoc_collection_t *MONGODB_LINKS_N= NULL;
static bson_t MONGODB_REPLY = BSON_INITIALIZER;
static bson_error_t MONGODB_ERROR = {0};
static bson_t *MONGODB_REPLACE_OPTIONS = NULL;
static bson_t *MONGODB_INSERT_MANY_OPTIONS = NULL;

// Redis reusable globals
#ifdef DB_LOADER_USE_REDIS_CLUSTER
#define REDIS_APPEND_COMMAND_MACRO redisClusterAppendCommand
#define REDIS_APPEND_COMMAND_ARGV_MACRO redisClusterAppendCommandArgv
#define REDIS_GET_REPLAY_MACRO redisClusterGetReply
#define REDIS_FREE_MACRO redisClusterFree
#define REDIS_CONTEXT_MACRO redisClusterContext
#else
#define REDIS_APPEND_COMMAND_MACRO redisAppendCommand
#define REDIS_APPEND_COMMAND_ARGV_MACRO redisAppendCommandArgv
#define REDIS_GET_REPLAY_MACRO redisGetReply
#define REDIS_FREE_MACRO redisFree
#define REDIS_CONTEXT_MACRO redisContext
#endif
static REDIS_CONTEXT_MACRO *REDIS = NULL;
static char REDIS_KEY[MAX_REDIS_KEY_SIZE];
static unsigned int HASH_SIZE = 0;
static unsigned int MAX_INDEXABLE_ARITY = 4;
static unsigned int PENDING_REDIS_COMMANDS = 0;
static char WILDCARD[] = "*";
static char **KEY_BUFFER = NULL;
static char *VALUE_BUFFER = NULL;
static char RPUSH[] = "RPUSH";
static char SADD[] = "SADD";
static char NAMED_ENTITIES[] = "names";
static char OUTGOING_SET[] = "outgoing_set";
static char INCOMMING_SET[] = "incomming_set";
static char TEMPLATES[] = "templates";
static char PATTERNS[] = "patterns";

// Atom insertion buffers
#define EXPRESSION_BUFFER_SIZE ((unsigned int) 100000)
#define SYMBOL_BUFFER_SIZE ((unsigned int) 300000)
static unsigned int EXPRESSION_BUFFER_CURSOR = 0;
static unsigned int SYMBOL_BUFFER_CURSOR = 0;
struct BufferedExpression {
    bool is_toplevel;
    struct HandleList composite;
    char *hash;
};
struct BufferedSymbol {
    bool is_literal;
    long value_as_int;
    long value_as_float;
    char *hash;
    char *name;
};
struct BufferedExpression EXPRESSION_BUFFER[EXPRESSION_BUFFER_SIZE];
struct BufferedSymbol SYMBOL_BUFFER[SYMBOL_BUFFER_SIZE];

// Reusable types and hashes
static char *SYMBOL = "Symbol";
static char *SYMBOL_HASH = NULL;
static char *SYMBOL_SYMBOL = NULL;
static char *EXPRESSION = "Expression";
static char *EXPRESSION_HASH = NULL;
static char *EXPRESSION_SYMBOL = NULL;
static char *TYPE = "Type";
static char *TYPE_HASH = NULL;
static char *TYPE_SYMBOL = NULL;
static char *METTA_TYPE = "MettaType";
static char *METTA_TYPE_HASH = NULL;
static char *METTA_TYPE_SYMBOL = NULL;
static char *TYPEDEF_MARK = ":";
static char *TYPEDEF_MARK_HASH = NULL;
static char *ARROW = "->";
static char *ARROW_HASH = NULL;
static char *COMPOSITE_TYPE_TYPEDEF_HASH = NULL;
// All global hashes must be listed below to avoid being freed
#define DESTROY_HANDLE(S) if (\
    S != SYMBOL_HASH && \
    S != EXPRESSION_HASH && \
    S != TYPE_HASH && \
    S != METTA_TYPE_HASH && \
    S != TYPEDEF_MARK_HASH && \
    S != ARROW_HASH && \
    S != COMPOSITE_TYPE_TYPEDEF_HASH) free(S);


static void mongodb_destroy() {
    mongoc_database_destroy(MONGODB);
    mongoc_client_destroy(MONGODB_CLIENT);
    mongoc_collection_destroy(MONGODB_TYPES);
    mongoc_collection_destroy(MONGODB_NODES);
    mongoc_collection_destroy(MONGODB_LINKS_1);
    mongoc_collection_destroy(MONGODB_LINKS_2);
    mongoc_collection_destroy(MONGODB_LINKS_N);
    bson_destroy(&MONGODB_REPLY);
    bson_destroy(MONGODB_REPLACE_OPTIONS);
    bson_destroy(MONGODB_INSERT_MANY_OPTIONS);
    mongoc_cleanup();
}

static void mongodb_error(char *message) {
    fprintf(stderr, "MongoDB: %s\n", message);
    mongodb_destroy();
    exit(1);
}

static void mongodb_setup() {

    char *host = getenv("DAS_MONGODB_HOSTNAME");
    char *port = getenv("DAS_MONGODB_PORT");
    char *user = getenv("DAS_MONGODB_USERNAME");
    char *password = getenv("DAS_MONGODB_PASSWORD");

    if (host == NULL || port == NULL || user == NULL || password == NULL) {
        fprintf(stderr, "You need to set MongoDB credentials as environment variables\n");
        exit(1);
    }
    if (strlen(host) + strlen(port) + strlen(user) + strlen(password) > 200) {
        fprintf(stderr, "Invalid MongoDB credentials\n");
        exit(1);
    }
    //char uri[] = "mongodb://dbadmin:dassecret@127.0.0.1:28000/";
    char uri[256];
    sprintf(uri, "mongodb://%s:%s@%s:%s/", user, password, host, port);

    printf("Connecting to MongoDB at %s:%s\n", host, port);
    mongoc_init();
    MONGODB_CLIENT = mongoc_client_new(uri);
    if (!MONGODB_CLIENT) {
        mongodb_error("Failed to create a MongoDB client.");
    }
    MONGODB_TYPES = mongoc_client_get_collection(MONGODB_CLIENT, "das", "atom_types");
    MONGODB_NODES = mongoc_client_get_collection(MONGODB_CLIENT, "das", "nodes");
    MONGODB_LINKS_1 = mongoc_client_get_collection(MONGODB_CLIENT, "das", "links_1");
    MONGODB_LINKS_2 = mongoc_client_get_collection(MONGODB_CLIENT, "das", "links_2");
    MONGODB_LINKS_N = mongoc_client_get_collection(MONGODB_CLIENT, "das", "links_n");

    MONGODB_REPLACE_OPTIONS = bson_new();
    BSON_APPEND_BOOL(MONGODB_REPLACE_OPTIONS, "upsert", true);
    MONGODB_INSERT_MANY_OPTIONS = bson_new();
    BSON_APPEND_BOOL(MONGODB_INSERT_MANY_OPTIONS, "ordered", false);
    BSON_APPEND_BOOL(MONGODB_INSERT_MANY_OPTIONS, "bypassDocumentValidation", true);
}

static void redis_setup() {

    char *host = getenv("DAS_REDIS_HOSTNAME");
    char *port = getenv("DAS_REDIS_PORT");

    if (host == NULL || port == NULL) {
        fprintf(stderr, "You need to set Redis access info as environment variables\n");
        exit(1);
    }
    printf("Connecting to Redis at %s:%s\n", host, port);
#ifdef DB_LOADER_USE_REDIS_CLUSTER
    char *redis_address = (char *) malloc((strlen(host) + strlen(port) + 2) * sizeof(char));
    sprintf(redis_address, "%s:%s", host, port);
    REDIS = redisClusterConnect(redis_address, 0);
    free(redis_address);
#else
    REDIS = redisConnect(host, atoi(port));
#endif
    if (REDIS == NULL) {
        printf("Connection error.\n");
        exit(1);
    }
    if (REDIS->err) {
        printf("Redis error: %s\n", REDIS->errstr);
        exit(1);
    }

    HASH_SIZE = strlen(SYMBOL_HASH);
    KEY_BUFFER = (char **) malloc(MAX_INDEXABLE_ARITY * sizeof(char *));
    VALUE_BUFFER = (char *) malloc(((HASH_SIZE * (MAX_INDEXABLE_ARITY + 1)) + 1) * sizeof(char));
}

static struct HandleList build_handle_list(char *element, char *element_type) {
    struct HandleList handle_list;
    handle_list.size = 1;
    handle_list.elements = (char **) malloc(sizeof(char *));
    handle_list.elements[0] = element;
    handle_list.elements_type = (char **) malloc(sizeof(char *));
    handle_list.elements_type[0] = element_type;
    return handle_list;
}

static char *add_function_typedef(struct HandleList composite) {

    // printf("ADD FUNCTION TYPEDEF\n");

    bson_t *selector = bson_new();
    bson_t *doc = bson_new();
    char *hash = expression_hash(ARROW_HASH, composite.elements_type, composite.size);
    BSON_APPEND_UTF8(selector, "_id", hash);
    BSON_APPEND_UTF8(doc, "_id", hash);
    BSON_APPEND_UTF8(doc, "composite_type_hash", COMPOSITE_TYPE_TYPEDEF_HASH);
    char *named_type = string_copy(ARROW);
    // This is expensive but we hope for a relatively low number of function typedefs as compared
    // to regular expressions in a large knowledge base (where this inneficiency would be relevant).
    for (unsigned int i = 0; i < composite.size; i++) {
        named_type = (char *) realloc(named_type, (strlen(named_type) + strlen(composite.elements[i]) + 2) * sizeof(char));
        strcat(named_type, " ");
        strcat(named_type, composite.elements[i]);
    }
    char *namedtype_hash = named_type_hash(named_type);
    BSON_APPEND_UTF8(doc, "named_type", named_type);
    BSON_APPEND_UTF8(doc, "named_type_hash", namedtype_hash);
    if (! mongoc_collection_replace_one(MONGODB_TYPES, selector, doc, MONGODB_REPLACE_OPTIONS, NULL, &MONGODB_ERROR)) {
        mongodb_error((char *) &MONGODB_ERROR.message);
    } else {
        bson_destroy(selector);
        bson_destroy(doc);
        free(named_type);
        free(namedtype_hash);
    }
    return hash;
}

static void destroy_buffered_symbol(struct BufferedSymbol *symbol) {
    DESTROY_HANDLE(symbol->hash);
    free(symbol->name);
    symbol->hash = NULL;
}

static void symbol_copy(struct BufferedSymbol *s1, struct BufferedSymbol *s2) {
    destroy_buffered_symbol(s1);
    s1->hash = string_copy(s2->hash);
    s1->name = string_copy(s2->name);
    s1->is_literal = s2->is_literal;
    s1->value_as_int = s2->value_as_int;
    s1->value_as_float = s2->value_as_float;
}

static int symbol_cmp(const void *s1, const void *s2) {
    return strcmp(
        ((struct BufferedSymbol *) s1)->hash,
        ((struct BufferedSymbol *) s2)->hash);
}

static bson_t *build_symbol_bson_document(char *hash, char *name, bool is_literal, long value_as_int, double value_as_float) {
    bson_t *doc = bson_new();
    BSON_APPEND_UTF8(doc, "_id", hash);
    BSON_APPEND_UTF8(doc, "composite_type_hash", SYMBOL_HASH);
    BSON_APPEND_UTF8(doc, "name", name);
    BSON_APPEND_UTF8(doc, "named_type", SYMBOL);
    BSON_APPEND_BOOL(doc, "is_literal", is_literal);
    if (value_as_int != LONG_MIN) {
        BSON_APPEND_INT64(doc, "value_as_int", value_as_int);
    }
    if (value_as_float != DBL_MIN) {
        BSON_APPEND_DOUBLE(doc, "value_as_float", value_as_float);
    }
    return doc;
}

static void flush_redis_commands() {
    for (unsigned int i = 0; i < PENDING_REDIS_COMMANDS; i++) {
        if (REDIS_GET_REPLAY_MACRO(REDIS, NULL) != REDIS_OK) {
            printf("REDIS ERROR\n");
        }
    }
    PENDING_REDIS_COMMANDS = 0;
}

static void flush_symbol_buffer() {

    qsort(SYMBOL_BUFFER,
          SYMBOL_BUFFER_CURSOR,
          sizeof(struct BufferedSymbol),
          symbol_cmp);

    unsigned int cursor1 = 1;
    unsigned int cursor2 = 1;
    while (cursor2 < SYMBOL_BUFFER_CURSOR) {
        if (! strcmp(SYMBOL_BUFFER[cursor1 - 1].hash, SYMBOL_BUFFER[cursor2].hash)) {
            cursor2++;
        } else {
            if (cursor1 != cursor2) {
                symbol_copy(&(SYMBOL_BUFFER[cursor1]), &(SYMBOL_BUFFER[cursor2]));
            }
            cursor1++;
            cursor2++;
        }
    }
    unsigned int new_size = cursor1;

    while (cursor1 < SYMBOL_BUFFER_CURSOR) {
        destroy_buffered_symbol(&(SYMBOL_BUFFER[cursor1++]));
    }
    SYMBOL_BUFFER_CURSOR = 0;

    bson_t **bulk_insertion_buffer = (bson_t **) malloc(new_size * sizeof(bson_t *));
    for (unsigned int i = 0; i < new_size; i++) {
        REDIS_APPEND_COMMAND_MACRO(REDIS, "SET %s:%s %s" , NAMED_ENTITIES, SYMBOL_BUFFER[i].hash, SYMBOL_BUFFER[i].name);
        PENDING_REDIS_COMMANDS++;

        bulk_insertion_buffer[i] = build_symbol_bson_document(
                SYMBOL_BUFFER[i].hash,
                SYMBOL_BUFFER[i].name,
                SYMBOL_BUFFER[i].is_literal,
                SYMBOL_BUFFER[i].value_as_int,
                SYMBOL_BUFFER[i].value_as_float);
    }
    mongoc_collection_insert_many(
            MONGODB_NODES,
            (const bson_t **) bulk_insertion_buffer,
            new_size,
            MONGODB_INSERT_MANY_OPTIONS,
            NULL,
            &MONGODB_ERROR);
    flush_redis_commands();

    for (unsigned int i = 0; i < new_size; i++) {
        bson_destroy(bulk_insertion_buffer[i]);
    }
    free(bulk_insertion_buffer);
#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(yylineno, INPUT_LINE_COUNT, 1, 1, false);
#endif
}

static char *add_symbol(char *name, bool is_literal, long value_as_int, double value_as_float) {
    char *hash = terminal_hash(SYMBOL, name);
    SYMBOL_BUFFER[SYMBOL_BUFFER_CURSOR].is_literal = is_literal;
    SYMBOL_BUFFER[SYMBOL_BUFFER_CURSOR].value_as_int = value_as_int;
    SYMBOL_BUFFER[SYMBOL_BUFFER_CURSOR].value_as_float = value_as_float;
    SYMBOL_BUFFER[SYMBOL_BUFFER_CURSOR].hash = string_copy(hash);
    SYMBOL_BUFFER[SYMBOL_BUFFER_CURSOR].name = string_copy(name);
    SYMBOL_BUFFER_CURSOR++;
    if (SYMBOL_BUFFER_CURSOR == SYMBOL_BUFFER_SIZE) {
        flush_symbol_buffer();
    }
    return hash;
}

static void add_link_arity_2(
        char *link_type_hash, 
        char *source_hash, 
        char *source_type_hash, 
        char *target_hash, 
        char *target_type_hash) {

    //printf("ADD LINK2 %s <%s (%s) %s (%s)>\n", link_type_hash, source_hash, source_type_hash, target_hash, target_type_hash);
    struct HandleList composite;
    composite.size = 2;
    composite.expression_type_hash = link_type_hash;
    // These are not memory leaks. These arrays are destroyed when EXPRESSION_BUFFER is proccessed
    composite.elements = (char **) malloc(2 * (sizeof(char *)));
    composite.elements_type = (char **) malloc(2 * (sizeof(char *)));
    composite.elements[0] = source_hash;
    composite.elements[1] = target_hash;
    composite.elements_type[0] = source_type_hash;
    composite.elements_type[1] = target_type_hash;
    char *hash = expression_hash(link_type_hash, composite.elements, 2);
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].is_toplevel = false;
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].composite = composite;
    // hash is destroyed when EXPRESSION_BUFFER is proccessed
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].hash = hash;
    EXPRESSION_BUFFER_CURSOR++;
}

static char *add_typedef(char *child_type, bool child_is_hash, char *parent_type, bool parent_is_hash) {

    //printf("ADD TYPEDEF %s (%s) %s (%s)\n", child_type, (child_is_hash ? "hash" : "string"), parent_type, (parent_is_hash ? "hash" : "string"));

    bson_t *selector = bson_new();
    bson_t *doc = bson_new();
    char *child_hash = child_is_hash ? child_type : add_symbol(child_type, false, LONG_MIN, DBL_MIN);
    char *parent_hash = parent_is_hash ? parent_type : add_symbol(parent_type, false, LONG_MIN, DBL_MIN);
    char *pair[2] = {child_hash, parent_hash};
    char *hash = expression_hash(TYPEDEF_MARK_HASH, pair, 2);
    BSON_APPEND_UTF8(selector, "_id", hash);
    BSON_APPEND_UTF8(doc, "_id", hash);
    BSON_APPEND_UTF8(doc, "composite_type_hash", COMPOSITE_TYPE_TYPEDEF_HASH);
    BSON_APPEND_UTF8(doc, "named_type", child_type);
    BSON_APPEND_UTF8(doc, "named_type_hash", child_hash);
    add_link_arity_2(METTA_TYPE_HASH, child_hash, SYMBOL_HASH, parent_hash, SYMBOL_HASH);
    if (! mongoc_collection_replace_one(MONGODB_TYPES, selector, doc, MONGODB_REPLACE_OPTIONS, NULL, &MONGODB_ERROR)) {
        mongodb_error((char *) &MONGODB_ERROR.message);
    } else {
        bson_destroy(selector);
        bson_destroy(doc);
        if (! child_is_hash) {
            free(child_hash);
        }
        if (! parent_is_hash) {
            free(parent_hash);
        }
    }
    return hash;
}

static void add_redis_pattern(char **composite_key, unsigned int arity, char *value) {
    char *key = composite_hash(composite_key, arity);
    REDIS_APPEND_COMMAND_MACRO(REDIS, "%s %s:%s %s", SADD, PATTERNS, key, value);
    PENDING_REDIS_COMMANDS++;
    free(key);
}

static void add_redis_indexes(char *hash, struct HandleList *composite, char *composite_type_hash) {

    // Incomming and outgoing sets
    sprintf(REDIS_KEY, "%s:%s", OUTGOING_SET, hash);
    char **argv = (char **) malloc((composite->size + 2) * sizeof(char *));
    argv[0] = RPUSH;
    argv[1] = REDIS_KEY;
    for (unsigned int i = 0; i < composite->size; i++) {
        argv[i + 2] = composite->elements[i];
        REDIS_APPEND_COMMAND_MACRO(REDIS, "%s %s:%s %s", SADD, INCOMMING_SET, composite->elements[i], hash);
        PENDING_REDIS_COMMANDS++;
    }
    REDIS_APPEND_COMMAND_ARGV_MACRO(REDIS, composite->size + 2, (const char **) argv, NULL);
    free(argv);

    // hash + targets used in temnplates and patterns
    unsigned int cursor = 0;
    for (unsigned int j = 0; j < HASH_SIZE; j++) {
        VALUE_BUFFER[cursor++] = hash[j];
    }
    for (unsigned int i = 0; i < composite->size; i++) {
        for (unsigned int j = 0; j < HASH_SIZE; j++) {
            VALUE_BUFFER[cursor++] = composite->elements[i][j];
        }
    }
    VALUE_BUFFER[cursor] = '\0';

    // Templates
    REDIS_APPEND_COMMAND_MACRO(REDIS, "%s %s:%s %s", SADD, TEMPLATES, composite_type_hash, VALUE_BUFFER);
    PENDING_REDIS_COMMANDS++;

    // Patterns
    unsigned int arity = composite->size;
    if (arity > MAX_INDEXABLE_ARITY) {
        return;
    }
    switch (arity) {
        case 4:
            for (unsigned int c1 = 0; c1 < arity; c1++) {
                for (unsigned int c2 = 0; c2 < arity; c2++) {
                    for (unsigned int c3 = 0; c3 < arity; c3++) {
                        for (unsigned int i = 0; i < arity; i++) {
                            if (i == c1 || i == c2 || i == c3) {
                                KEY_BUFFER[i] = WILDCARD;
                            } else {
                                KEY_BUFFER[i] = composite->elements[1];
                            }
                        }
                        add_redis_pattern(KEY_BUFFER, arity, VALUE_BUFFER);
                    }
                }
            }
            break;
        case 3:
            for (unsigned int c1 = 0; c1 < arity; c1++) {
                for (unsigned int c2 = 0; c2 < arity; c2++) {
                    for (unsigned int i = 0; i < arity; i++) {
                        if (i == c1 || i == c2) {
                            KEY_BUFFER[i] = WILDCARD;
                        } else {
                            KEY_BUFFER[i] = composite->elements[1];
                        }
                    }
                    add_redis_pattern(KEY_BUFFER, arity, VALUE_BUFFER);
                }
            }
            break;
        case 2:
            for (unsigned int c1 = 0; c1 < arity; c1++) {
                for (unsigned int i = 0; i < arity; i++) {
                    if (i == c1) {
                        KEY_BUFFER[i] = WILDCARD;
                    } else {
                        KEY_BUFFER[i] = composite->elements[1];
                    }
                }
               add_redis_pattern(KEY_BUFFER, arity, VALUE_BUFFER);
            }
        default:
    }
}

static bson_t *build_expression_bson_document(char *hash, bool is_toplevel, struct HandleList *composite) {
    bson_t *doc = bson_new();
    char **composite_type = (char **) malloc((composite->size + 1) * sizeof(char *));
    composite_type[0] = composite->expression_type_hash;
    for (unsigned int i = 0; i < composite->size; i++) {
        composite_type[i + 1] = composite->elements_type[i];
    }
    BSON_APPEND_UTF8(doc, "_id", hash);
    char *composite_type_hash = composite_hash(composite_type, composite->size + 1);
    BSON_APPEND_UTF8(doc, "composite_type_hash", composite_type_hash);
    free(composite_type_hash);
    BSON_APPEND_BOOL(doc, "is_toplevel", is_toplevel);
    bson_t *composite_type_doc = bson_new();
    char count[8];
    for (unsigned int i = 0; i < (composite->size + 1); i++) {
        sprintf(count, "%d", i);
        BSON_APPEND_UTF8(composite_type_doc, count, composite_type[i]);
    }
    BSON_APPEND_ARRAY(doc, "composite_type", composite_type_doc);
    free(composite_type);
    bson_destroy(composite_type_doc);
    BSON_APPEND_UTF8(doc, "named_type", EXPRESSION);
    BSON_APPEND_UTF8(doc, "named_type_hash", EXPRESSION_HASH);
    char key_tag[8];
    for (unsigned int i = 0; i < composite->size; i++) {
        sprintf(key_tag, "key_%d", i);
        BSON_APPEND_UTF8(doc, key_tag, composite->elements[i]);
    }
    add_redis_indexes(hash, composite, composite_type_hash);
    return doc;
}

static void destroy_handle_list(struct HandleList *handle_list) {
    for (unsigned int i = 0; i < handle_list->size; i++) {
        DESTROY_HANDLE(handle_list->elements[i]);
        DESTROY_HANDLE(handle_list->elements_type[i]);
    }
    free(handle_list->elements);
    free(handle_list->elements_type);
}

static void destroy_buffered_expression(struct BufferedExpression *expression) {
    DESTROY_HANDLE(expression->hash);
    destroy_handle_list(&(expression->composite));
    expression->hash = NULL;
}

static int expression_cmp(const void *e1, const void *e2) {
    return strcmp(
        ((struct BufferedExpression *) e1)->hash,
        ((struct BufferedExpression *) e2)->hash);
}

static void expression_copy(struct BufferedExpression *e1, struct BufferedExpression *e2) {
    destroy_buffered_expression(e1);
    e1->hash = string_copy(e2->hash);
    e1->is_toplevel = e2->is_toplevel;
    e1->composite.size = e2->composite.size;
    e1->composite.elements = (char **) malloc(e1->composite.size * sizeof(char *));
    e1->composite.elements_type = (char **) malloc(e1->composite.size * sizeof(char *));
    for (unsigned int i = 0; i < e1->composite.size; i++) {
        e1->composite.elements[i] = string_copy(e2->composite.elements[i]);
        e1->composite.elements_type[i] = string_copy(e2->composite.elements_type[i]);
    }
}

static void flush_expression_buffer() {

    qsort(EXPRESSION_BUFFER,
          EXPRESSION_BUFFER_CURSOR,
          sizeof(struct BufferedExpression),
          expression_cmp);

    unsigned int cursor1 = 1;
    unsigned int cursor2 = 1;
    while (cursor2 < EXPRESSION_BUFFER_CURSOR) {
        if (! strcmp(EXPRESSION_BUFFER[cursor1 - 1].hash, EXPRESSION_BUFFER[cursor2].hash)) {
            cursor2++;
        } else {
            if (cursor1 != cursor2) {
                expression_copy(&(EXPRESSION_BUFFER[cursor1]), &(EXPRESSION_BUFFER[cursor2]));
            }
            cursor1++;
            cursor2++;
        }
    }
    unsigned int new_size = cursor1;

    while (cursor1 < EXPRESSION_BUFFER_CURSOR) {
        destroy_buffered_expression(&(EXPRESSION_BUFFER[cursor1++]));
    }
    EXPRESSION_BUFFER_CURSOR = 0;

    bson_t **bulk_insertion_buffer = (bson_t **) malloc(new_size * sizeof(bson_t *));
    for (unsigned int i = 0; i < new_size; i++) {
        bulk_insertion_buffer[i] = build_expression_bson_document(
                EXPRESSION_BUFFER[i].hash,
                EXPRESSION_BUFFER[i].is_toplevel,
                &(EXPRESSION_BUFFER[i].composite));
    }

    mongoc_collection_insert_many(
            // XXX TODO Fix mongodb collections
            MONGODB_LINKS_N,
            (const bson_t **) bulk_insertion_buffer,
            new_size,
            MONGODB_INSERT_MANY_OPTIONS,
            NULL,
            &MONGODB_ERROR);
    flush_redis_commands();

    for (unsigned int i = 0; i < new_size; i++) {
        bson_destroy(bulk_insertion_buffer[i]);
    }
    free(bulk_insertion_buffer);
#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(yylineno, INPUT_LINE_COUNT, 1, 1, false);
#endif
}

static char *add_expression(bool is_toplevel, struct HandleList composite) {
    char *hash = expression_hash(EXPRESSION_HASH, composite.elements, composite.size);
    composite.expression_type_hash = EXPRESSION_HASH;
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].is_toplevel = is_toplevel;
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].composite = composite;
    EXPRESSION_BUFFER[EXPRESSION_BUFFER_CURSOR].hash = string_copy(hash);
    EXPRESSION_BUFFER_CURSOR++;
    if (EXPRESSION_BUFFER_CURSOR == EXPRESSION_BUFFER_SIZE) {
        flush_expression_buffer();
    }
    return hash;
}

static void insert_commom_atoms() {
    SYMBOL_SYMBOL = add_symbol(SYMBOL, false, LONG_MIN, DBL_MIN);
    TYPE_SYMBOL = add_symbol(TYPE, false, LONG_MIN, DBL_MIN);
    EXPRESSION_SYMBOL = add_symbol(EXPRESSION, false, LONG_MIN, DBL_MIN);
    METTA_TYPE_SYMBOL = add_symbol(METTA_TYPE, false, LONG_MIN, DBL_MIN);
    add_typedef(SYMBOL_SYMBOL, true, TYPE_SYMBOL, true);
    add_typedef(EXPRESSION_SYMBOL, true, TYPE_SYMBOL, true);
    add_typedef(METTA_TYPE_SYMBOL, true, TYPE_SYMBOL, true);
}

// =====================================================================
// Public "action" API

void initialize_actions() {
    SYMBOL_HASH = named_type_hash(SYMBOL);
    EXPRESSION_HASH = named_type_hash(EXPRESSION);
    TYPEDEF_MARK_HASH = named_type_hash(TYPEDEF_MARK);
    ARROW_HASH = named_type_hash(ARROW);
    TYPE_HASH = named_type_hash(TYPE);
    METTA_TYPE_HASH = named_type_hash(METTA_TYPE);
    char *composite_type_typedef[3] = {TYPEDEF_MARK_HASH, TYPE_HASH, TYPE_HASH};
    COMPOSITE_TYPE_TYPEDEF_HASH = composite_hash((char **) composite_type_typedef, 3);
    redis_setup();
    mongodb_setup();
    insert_commom_atoms();
#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(yylineno, INPUT_LINE_COUNT, 1, 1, false);
#endif
}

void finalize_actions() {
    if (EXPRESSION_BUFFER_CURSOR > 0) {
        flush_expression_buffer();
    }
    REDIS_FREE_MACRO(REDIS);
    mongodb_destroy();

#ifndef SUPPRESS_PROGRESS_BAR
    print_progress_bar(INPUT_LINE_COUNT, INPUT_LINE_COUNT, 1, 1, true);
#endif
}

void start() {
}

void toplevel_list_base(char *handle) {
    free(handle);
}

void toplevel_list_recursion(char *handle) {
    free(handle);
}

char *typedef_base(char *handle) {
    return handle;
}

char *typedef_inherited(char *handle) {
    return handle;
}

char *symbol_typedef_symbol_type(char *symbol) {
    return add_typedef(symbol, false, TYPE_SYMBOL, true);
}

char *symbol_typedef_symbol_symbol(char *symbol, char *parent_type) {
    return add_typedef(symbol, false, parent_type, false);
}

char *symbol_typedef_literal_type(char *literal) {
    return add_typedef(literal, true, TYPE_SYMBOL, true);
}

char *symbol_typedef_literal_symbol(char *literal, char *parent_type) {
    return add_typedef(literal, true, parent_type, false);
}

char *inherited_typedef(char *symbol, char *parent_type) {
    return symbol_typedef_symbol_symbol(symbol, parent_type);
}

char *base_typedef_function(char *symbol, char *function_handle) {
    char *answer = add_typedef(symbol, false, function_handle, true);
    free(function_handle);
    return answer;
}

char *function_typedef(struct HandleList composite) {
    return add_function_typedef(composite);
}

struct HandleList type_desc_type() {
    return build_handle_list(TYPE_SYMBOL, SYMBOL_HASH);
}

struct HandleList type_desc_symbol(char *symbol) {
    return build_handle_list(add_symbol(symbol, false, LONG_MIN, DBL_MIN), SYMBOL_HASH);
}

struct HandleList type_desc_function(char *handle) {
    // TODO review this: there should be a way to determine function type instead of copying handle
    return build_handle_list(handle, string_copy(handle));
}

char *toplevel_expression(struct HandleList composite) {
    return add_expression(true, composite);
}

struct HandleList expression_list_base(struct HandleList base) {
    return base;
}

struct HandleList expression_list_recursion(struct HandleList list, struct HandleList new_element) {

    struct HandleList new_list;

    new_list.size = list.size + 1;
    new_list.elements = (char **) realloc(list.elements, new_list.size * sizeof(char *));
    new_list.elements_type = (char **) realloc(list.elements_type, new_list.size * sizeof(char *));
    new_list.elements[list.size] = new_element.elements[0];
    new_list.elements_type[list.size] = new_element.elements_type[0];

    free(new_element.elements);
    free(new_element.elements_type);

    return new_list;
}

struct HandleList type_desc_list_base(struct HandleList base) {
    return base;
}

struct HandleList type_desc_list_recursion(struct HandleList list, struct HandleList new_element) {
    return expression_list_recursion(list, new_element);
}

struct HandleList expression_symbol(char *symbol) {
    return build_handle_list(add_symbol(symbol, false, LONG_MIN, DBL_MIN), SYMBOL_HASH);
}

struct HandleList expression_literal(char *literal) {
    return build_handle_list(literal, SYMBOL_HASH);
}

struct HandleList expression_composite(struct HandleList composite) {
    return build_handle_list(add_expression(false, composite), EXPRESSION_HASH);
}

char *literal_string(char *literal) {
    return add_symbol(literal, true, LONG_MIN, DBL_MIN);
}

char *literal_int(long literal) {
    char name[64];
    sprintf(name, "%ld", literal);
    return add_symbol(name, true, literal, DBL_MIN);
}

char *literal_float(double literal) {
    char name[64];
    sprintf(name, "%g", literal);
    return add_symbol(name, true, LONG_MIN, literal);
}
