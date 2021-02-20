#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
 
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>       /* close */
#include <netdb.h> 
 
#include <sys/epoll.h>
#include <pthread.h>
 
 
#define HTTP_VERSION    "HTTP/1.1"
#define USER_AGENT      "User-Agent: Mozilla/5.0 (Windows NT 5.1; rv:10.0.2) Gecko/20100101 Firefox/10.0.2\r\n"
#define ENCODE_TYPE     "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
#define CONNECTION_TYPE "Connection: close\r\n"
 
 
 
#define BUFFER_SIZE     4096
 
 
char *host_to_ip(const char *hostname) {
 
    struct hostent *host_entry = gethostbyname(hostname); //gethostbyname_r / getaddrinfo
    if (host_entry) {
        return inet_ntoa(*(struct in_addr*)*host_entry->h_addr_list);
    } else {
        return NULL;
    }
}
 
int http_create_socket( char *ip) {
 
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 
    struct sockaddr_in sin = {0};
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(80);
    sin.sin_family = AF_INET;
 
    if (-1 == connect(sockfd, (struct sockaddr*)&sin, sizeof(struct sockaddr_in))) {
        return -1;
    }
 
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
 
    return sockfd;
 
}
 
 
char *http_send_request(int sockfd, const char *hostname, const char *resource) {
 
    char buffer[BUFFER_SIZE] = {0};
     
    int len = sprintf(buffer, 
"GET %s %s\r\n\
Host: %s\r\n\
%s\r\n\
\r\n",
         resource, HTTP_VERSION,
         hostname,
         CONNECTION_TYPE
         );
    printf("request buffer:%s\n", buffer);
    send(sockfd, buffer, strlen(buffer), 0);
 
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
 
    fd_set fdread;
    FD_ZERO(&fdread);
    FD_SET(sockfd, &fdread);
 
    char *result = malloc(sizeof(int));
    result[0] = '\0';
 
    while (1) {
 
        int selection = select(sockfd+1, &fdread, NULL, NULL, &tv);
        if (!selection || !(FD_ISSET(sockfd, &fdread))) {
            break;
        } else {
            len = recv(sockfd, buffer, BUFFER_SIZE, 0);
            if (len == 0) break;
 
            result = realloc(result, (strlen(result) + len + 1) * sizeof(char));
            strncat(result, buffer, len);
        }
    }
 
    return result;
 
}
 
 
int http_client_commit(const char *hostname, const char *resource) {
    char *ip = host_to_ip(hostname);
 
    int sockfd = http_create_socket(ip);
 
    char *content =  http_send_request(sockfd, hostname, resource);
    if (content == NULL) {
        printf("have no data\n");
    }
 
    puts(content);
    close(sockfd);
    free(content);

    return 0;
}


struct http_request {
    char *hostname;
    char *resource;
};
 

struct http_request reqs[] = {
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=beijing&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=changsha&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shenzhen&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=shanghai&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=tianjin&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuhan&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hefei&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=hangzhou&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=nanjing&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=jinan&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=taiyuan&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=wuxi&language=zh-Hans&unit=c" },
    {"api.seniverse.com", "/v3/weather/now.json?key=0pyd8z7jouficcil&location=suzhou&language=zh-Hans&unit=c" },
};
 

struct async_context {
    pthread_t threadid;
    int epfd;
};

#define HOSTNAME_LENGTH     1024
#define ASYNC_CLIENT_NUM    1024

struct ep_arg {
    int sockfd;
    char hostname[HOSTNAME_LENGTH];
    async_result_cb cb;
};

typedef void (*async_result_cb)(const char *hostname, const char *result);

void *http_async_client_callback(void *arg) {

    struct async_context *ctx = (struct async_context*)arg;
    int epfd = ctx->epfd;

    while (1) {

        struct epoll_event events[]
        int nready = epoll_wait(epfd, events, ASYNC_CLIENT_NUM, -1);

        int i = 0;
        for (i = 0;i < nready;i ++) {
            
            struct ep_arg *data = (struct ep_arg*)events[i].data.ptr;
            int sockfd = data->sockfd;

            recv();

            epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);

            close(sockfd);
            free(data);

        }

    }

}


struct async_context* http_async_client_init(void) {

    struct async_context *ctx = (struct async_context*)malloc(sizeof(struct async_context));
    if (!ctx) {
        return NULL;
    }

    ctx->epfd = epoll_create(1);

    int ret = pthread_create(&ctx->threadid, NULL, http_async_client_callback, ctx); //创建的线程
    if (ret) {
        perror("pthread_create");
        close(ctx->epfd);
        free(ctx);
        return NULL;
    }
    usleep(1);

    return ctx;
    
}

struct http_async_client_uninit(struct async_context *ctx) {

    pthread_cancel(ctx->threadid);
    //pthread_join(ctx->threadid, &ctx);

    close(ctx->epfd);

    free(ctx);

}

//
int http_async_client_commit(struct async_context *ctx, const char *hostname, const char *resource, async_result_cb cb) {

    char buffer[BUFFER_SIZE] = {0};
     
    int len = sprintf(buffer, 
"GET %s %s\r\n\
Host: %s\r\n\
%s\r\n\
\r\n",
         resource, HTTP_VERSION,
         hostname,
         CONNECTION_TYPE
         );
    printf("request buffer:%s\n", buffer);


    char *ip = host_to_ip(hostname);
 
    int sockfd = http_create_socket(ip);
    send(sockfd, buffer, strlen(buffer), 0);


    struct ep_arg *eparg = (struct ep_arg*)malloc(sizeof(struct ep_arg));
    if (!eparg) {
        return NULL;
    }
    struct epoll_event ev;
    ev.data.ptr = eparg;
    ev.events = EPOLLIN;

    epoll_ctl(ctx->epfd, sockfd, EPOLL_CTL_ADD, &ev);

 


    return result;

}

 
int main(int argc, char *argv[]) {

     
    int count = sizeof(reqs) / sizeof(reqs[0]);
    int i = 0;
    for (i = 0;i < count;i ++) {
        http_client_commit(reqs[i].hostname, reqs[i].resource);
    }

 
    return 0;
}