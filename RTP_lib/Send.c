/*
VOIP Implementation using RTP 
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include "Config.h"
#include "RTP.h"
#include "Types.h"
#include "Proto.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<signal.h>
#include <arpa/inet.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/timerfd.h>
#include <time.h>
#include "G711.c"
#include <stdlib.h>
#include <stdint.h> 
#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#define BUFSIZE 1024
short buf[BUFSIZE];
unsigned char buf2[BUFSIZE];

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)
long unsigned no=0;
volatile sig_atomic_t keep_going = 1;

/* Signal Handler Function */
void handler_for_sigint(int signumber)//handler to handle SIGINT ctrl+C
{
	char ans[2];
	if (signumber == SIGINT)
	{
		printf("received SIGINT\n");
		printf("Program received a CTRL-C\n");
		printf("Terminate Y/N : "); 
		scanf("%s", ans);
		if (strcmp(ans,"Y") == 0)//terminate if Y
		{
			keep_going = 0;//set a variable and perform cleanup in main
			exit(0); 
		}
		else
		{
		printf("Continung ..\n");
		}
	}
}


/*Main Function */
int main(int argc, char *argv[])
{
	/*RTP packet related field declarations */
	char buffer[MAX_PAYLOAD_LEN];
	context	cid;
	u_int32		period;
	u_int32		t_inc;
	u_int16		size_read;
	u_int16		last_size_read;

	/* Pulse audio releated specifications */
	static const pa_sample_spec ss = {
	.format = PA_SAMPLE_S16LE,
	.rate = 8000,
	.channels = 2
	};
	pa_simple *s1 = NULL;
	pa_simple *s2=NULL;

	conx_context_t 	 *coucou = NULL;
	remote_address_t *s	 = NULL;
	int ret = 1;
	int error;
	int fd;
	int milisec = 100; // length of time to sleep, in miliseconds
	struct timespec req = {0};
	req.tv_sec = 0;
	req.tv_nsec = milisec * 1000000L;
	/*RTP Creation */
	period = Get_Period_us(PAYLOAD_TYPE);
	
	Init_Socket();
	RTP_Create(&cid);
	RTP_Add_Send_Addr(cid,argv[1], UDP_PORT, 6);
	Set_Extension_Profile(cid, 27);
	Add_Extension(cid, 123456);
	Add_Extension(cid, 654321);
	Add_CRSC(cid, 12569);

	/*Pulse audio audio recording */
	if (!(s1 = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
	fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
	goto finish;
	}
	printf("Creating Recording Stream\n");

	
	if (signal(SIGINT, handler_for_sigint) == SIG_ERR)//register signal handler
		printf("\ncan't catch SIGINT\n");
	
	/* Record some data ... */
	while(1)
	{
		if (pa_simple_read(s1, buf, sizeof(buf), &error) < 0) 
		{
			fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
			goto finish;
		}
	
		/*G711 Encoding */
		for(int i=0;i<BUFSIZE;i++)
		{
			size_read = BUFSIZE;
			buf2[i] = linear2ulaw(buf[i]);  //encode using u law
		}
		
		RTP_Send(cid, t_inc, 0, PAYLOAD_TYPE, buf2, size_read);
		nanosleep(&req, (struct timespec *)NULL);
		
	}

	RTP_Destroy(cid);
	Close_Socket();
	return (0);

	finish:
		if (s1)
		pa_simple_free(s1);

	return ret;
}

