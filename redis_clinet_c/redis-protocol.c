#include <hiredis/hiredis.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// gcc -o redis-protocol redis-protocol.c -I /usr/local/include/hiredis -L /usr/local/lib -lhiredis 

void flushdb(redisContext *context)
{
    redisReply *reply = (redisReply *)redisCommand(context, "FLUSHDB");
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        /*SET str Hello World*/
        printf("FLUSHDB OK\n");
    }
    printf("reply->str: %s\n", reply->str);

    freeReplyObject(reply);
}

void testSimpleString(redisContext *context)
{
    // Set Key Value
    const char *key = "str";
    const char *val = "Hello World";
    /*SET key value */
    redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", key, val);
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        /*SET str Hello World*/
        printf("SET %s %s\n", key, val);
    }
    printf("reply->str: %s\n", reply->str);

    freeReplyObject(reply);
}

void testErrors(redisContext *context)
{
    flushdb(context);
    // Set Key Value
    printf("SET\n");
    const char *key = "teacher";
    const char *val = "darren";
    /*SET key value */
    redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", key, val);
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        /*SET str Hello World*/
        printf("SET %s %s\n", key, val);
    }
    printf("reply->str: %s\n", reply->str);

    freeReplyObject(reply);

     // INCR Key 
    printf("\nINCR\n");
    /*INCR key value */
    reply = (redisReply *)redisCommand(context, "INCR %s", key);
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_ERROR)
    {
        /*INCR key*/
        printf("INCR %s failed:%s\n", key, reply->str);
    } else if (reply->type == REDIS_REPLY_INTEGER) {
        printf("INCR %s = %lld\n", key, reply->integer);     
    }

    freeReplyObject(reply);
}

void testIntegers(redisContext *context)
{
    flushdb(context);
    // Set Key Value
    printf("SET\n");
    const char *key = "count";
    const char *val = "10";
    /*SET key value */
    redisReply *reply = (redisReply *)redisCommand(context, "SET %s %s", key, val);
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_STATUS)
    {
        /*SET str Hello World*/
        printf("SET %s %s\n", key, val);
    }
    printf("reply->str: %s\n", reply->str);

    freeReplyObject(reply);

     // INCR Key 
    printf("\nINCR\n");
    /*INCR key value */
    reply = (redisReply *)redisCommand(context, "INCR %s", key);
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_ERROR)
    {
        /*INCR key*/
        printf("INCR %s failed:%s\n", key, reply->str);
    } else if (reply->type == REDIS_REPLY_INTEGER) {
        printf("INCR %s = %lld\n", key, reply->integer);     
    }

    freeReplyObject(reply);
}

void testBulkStrings(redisContext *context)
{
    flushdb(context);
    printf("MSET\n");
    /*MSET key value [key value ...]*/
    redisReply *reply = (redisReply *)redisCommand(context, "MSET king redis darren avmedia");
    if (reply->type == REDIS_REPLY_STATUS) {
        printf("MSET king redis darren avmedia\n");
    }
    freeReplyObject(reply);

    printf("\nMGET\n");   
    /*MGET key [key ...]*/
    reply = (redisReply *)redisCommand(context, "MGET king darren");
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_ARRAY) {
        printf("MGET king darren\n");
        redisReply **pReply = reply->element;
        int i = 0;
        size_t len = reply->elements;
        for (; i < len; ++i) {
            printf("%s \n", pReply[i]->str);  // 如果没有数据时为空
        }
        printf("\n");
    }
    freeReplyObject(reply);
}

void testArrays(redisContext *context)
{
    flushdb(context);
    printf("LPUSH\n");
    /*LPUSH key value [value ...]*/
    redisReply *reply = (redisReply *)redisCommand(context, "LPUSH list darren qiuxiang king milo");
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_INTEGER) {
        if(1 == reply->integer) {
            printf("LPUSH ok\n");     
        } else {
            printf("LPUSH failed\n");  
        } 
    } else {
        printf("LPUSH failed\n");
    }
    freeReplyObject(reply);

    printf("\nLRANGE\n");   
    /*LRANGE key start stop*/
    reply = (redisReply *)redisCommand(context, "LRANGE list 0 2");
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_ARRAY) {
        printf("LRANGE list 0 2\n");
        redisReply **pReply = reply->element;
        int i = 0;
        size_t len = reply->elements;
        for (; i < len; ++i) {
            printf("%s \n", pReply[i]->str);  
        }
        printf("\n");
    }
    freeReplyObject(reply);

    /*LRANGE key start stop*/
    reply = (redisReply *)redisCommand(context, "LRANGE list 0 -1");
    printf("reply->type: %d\n", reply->type);
    if (reply->type == REDIS_REPLY_ARRAY) {
        printf("LRANGE list 0 -1\n");
        redisReply **pReply = reply->element;
        int i = 0;
        size_t len = reply->elements;
        //hello world good
        for (; i < len; ++i) {
            printf("%s \n", pReply[i]->str);  
        }
        printf("\n");
    }
    freeReplyObject(reply);
}


int main()
{
    redisReply *reply = NULL;
    // 连接Redis服务
    redisContext *context = redisConnect("127.0.0.1", 6379);
    if (context == NULL || context->err)
    {
        if (context)
        {
            printf("%s\n", context->errstr);
        }
        else
        {
            printf("redisConnect error\n");
        }
        exit(EXIT_FAILURE);
    }
    printf("-----------------connect success--------------------\n");

    // reply = (redisReply *)redisCommand(context, "auth 0voice");
    // printf("type : %d\n", reply->type);
    // if (reply->type == REDIS_REPLY_STATUS)
    // {
    //     /*SET str Hello World*/
    //     printf("auth ok\n");
    // }
    // freeReplyObject(reply);

    testSimpleString(context);
    // testErrors(context);
    // testIntegers(context);
    // testBulkStrings(context);
    // testArrays(context);


    redisFree(context);


    return EXIT_SUCCESS;
}
