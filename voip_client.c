/*
** client.c -- a stream socket client demo. Command line arguments are Ipaddress and port number.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

#include <arpa/inet.h>
#include <fcntl.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#define BUFSIZE 100
#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes,size;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
   /* The sample type to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };
    pa_simple *sout = NULL;
    int ret = 1;
    int error;

    /* Create the recording stream */
    if (!(sout = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    if (argc != 3) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; 		//getaddrinfo gets socket address
    hints.ai_socktype = SOCK_STREAM;		//TCP or UDP

    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {  //arg is node,PORT is service returns a addrinfo structure-Reentrant function
									
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); //rv is error number
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connected to %s\n\n ", s);
    
    freeaddrinfo(servinfo); // all done with this structure
    printf("Hey client Speak : ");
    
 for (;;) {
        uint8_t buf[BUFSIZE];

/*latency varying from 30msec-400msec*/
#if 0
        pa_usec_t latency;

        if ((latency = pa_simple_get_latency(sout, &error)) == (pa_usec_t) -1) {
            fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
            goto finish;
        }

        fprintf(stderr, "%0.0f usec    \r", (float)latency);
#endif

        /* Record some data blocks from stream ... */
        if (pa_simple_read(sout, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            goto finish;
        }
	
	if (send(sockfd,buf,sizeof(buf), 0) == -1)
                perror("send");
	//printf("sent a block");
	usleep(200);
    }
    ret=0;
finish:
    /*Finish by flushing the stream*/
    if (sout)
        pa_simple_free(sout);
    close(sockfd);

    return 0;
}
























