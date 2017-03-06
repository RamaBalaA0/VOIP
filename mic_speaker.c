/* Speak to microphone and listen from speaker without sockets*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pulse/simple.h>
#include <pulse/error.h>
#include <pulse/gccmacro.h>

#define BUFSIZE 1024

int main(int argc, char*argv[]) {

    /* The Sample format to use */
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,			//signed 16 bit PCM Encoding
        .rate = 44100,
        .channels = 2
    };

    pa_simple *sin=NULL,*sout = NULL;
    int ret = 1;
    int error;

    /* Create a new playback stream */
    if (!(sout = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }
    /* Create the recording stream */
    if (!(sin = pa_simple_new(NULL, argv[0], PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error))) {
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    for (;;) {
        uint8_t buf[BUFSIZE];
        ssize_t r;


#if 1
        pa_usec_t latency;

        if ((latency = pa_simple_get_latency(sin, &error)) == (pa_usec_t) -1) {
            fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
            goto finish;
        }

        //fprintf(stderr, "%MIC : 0.0f usec\n", (float)latency);

        if ((latency = pa_simple_get_latency(sout, &error)) == (pa_usec_t) -1) {
            fprintf(stderr, __FILE__": pa_simple_get_latency() failed: %s\n", pa_strerror(error));
            goto finish;
        }

        //fprintf(stderr, "Speaker : %0.0f usec\n", (float)latency);
#endif

        /* Record some data blocks from stream ... */
        if (pa_simple_read(sin, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
            goto finish;
        }

        /* ... and play it,writing the data to the stream */
        if (pa_simple_write(sout, buf, sizeof(buf), &error) < 0) {
            fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
            goto finish;
        }
    }

    /* Make sure that every single sample was played*/
    if (pa_simple_drain(sout, &error) < 0) {
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
        goto finish;
    }

    ret = 0;

finish:
	/*Finishes by flushing the stream */
    if (sin)
        pa_simple_free(sin);
    if (sout)
        pa_simple_free(sout);

    return ret;
}
