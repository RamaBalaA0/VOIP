/* 
 * A demo program for an interval timer.
 *
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <signal.h>
#include <sys/time.h>

/* 
 * I'll make this global so that the alarm handler
 * function can change the interval if necessary
 */
struct itimerval it;

  struct timeval start, end;


/* Put stuff here to perform periodic task.
 * Note that you might have to change the values
 * in the itimerval struct to compensate for
 * variations in "real" time-of-day time due
 * to the inexactness of Linux system time
 * versus wall clock time
 */
void sigalrm_handler(int sig)
{
    static unsigned count = 0;
  gettimeofday(&start, NULL);
    printf("Alarm number %5u at %ld\n", ++count,(start.tv_sec * 1000000 + start.tv_usec));

}

int main()
{
    char buffer[BUFSIZ];


    it.it_value.tv_sec     = 0;       /* start in 1 second      */
    it.it_value.tv_usec    = 10000;
    it.it_interval.tv_sec  = 0;       /* repeat every 5 seconds */
    it.it_interval.tv_usec = 20000;

    signal(SIGALRM, sigalrm_handler); /* Install the handler    */


    /* Here's your main program loop. The alarm handler
     * function does its thing regardless of this
     */
gettimeofday(&start, NULL);
    printf("Started timer at %ld\n\n",(start.tv_sec * 1000000 + start.tv_usec)   );
/* turn on interval timer,ITIMER_REAL    decrements in real time, and delivers SIGALRM upon expi‐
                      ration. */
    setitimer(ITIMER_REAL, &it, NULL);


    printf("  To exit the program, enter a blank line.\n\n");
    while (fgets(buffer, sizeof(buffer), stdin) && (strlen(buffer) > 1)) {
        printf("You entered: %s\n", buffer);
    }

    printf("Bye\n");
    return 0;
}
