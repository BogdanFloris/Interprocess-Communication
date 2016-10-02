/*
 * Bogdan Floris
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>          // for perror()
#include <unistd.h>         // for getpid()
#include <mqueue.h>         // for mq-stuff
#include <time.h>           // for time()
#include <complex.h>

#include "settings.h"
#include "common.h"

static void rsleep (int t);

static double
complex_dist (complex a)
{
    // distance of vector 'a'
    // (in fact the square of the distance is computed...)
    double re, im;

    re = __real__ a;
    im = __imag__ a;
    return (re * re + im * im);
}

static int
mandelbrot_point (double x, double y)
{
    int     k;
    complex z;
	complex c;

	z = x + y * I;     // create a complex number 'z' from 'x' and 'y'
	c = z;

	for (k = 0; k < MAX_ITER && complex_dist(z) < INFINITY; k++)
	{
	    z = z * z + c;
    }

    //                                    2
    // k >= MAX_ITER or | z | >= INFINITY

    return (k);
}


int main (int argc, char * argv[])
{
    // TODO:
    // (see message_queue_test() in interprocess_basic.c)
    //  * open the two message queues (whose names are provided in the arguments)
    //  * repeatingly:
    //      - read from a message queue the new job to do
    //      - wait a random amount of time (e.g. rsleep(10000);)
    //      - do that job (use mandelbrot_point() if you like)
    //      - write the results to a message queue
    //    until there are no more jobs to do
    //  * close the message queues

    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;


    //open the two message queues (whose names are provided in the arguments)
    mq_fd_request = mq_open (argv[1], O_RDONLY);
    mq_fd_response = mq_open (argv[2], O_WRONLY);

    // while jobs are still there !!!!!!!
    while(true)
    {
      // read the message queue and store it in the request message
      mq_receive (mq_fd_request, (char *) &req, sizeof (req), NULL);
      if(req.a < 0)
      {
        break;
      }
      int line = req.a;

      //printf("Request%d\n", line );
      //wait a random amount of time (e.g. rsleep(10000);)
      rsleep(10000);

      rsp.b = line;
      int j;
      //do that job (use mandelbrot_point() if you like)
      for(j = 0; j < X_PIXEL; j++)
      {
        double mx = (X_LOWERLEFT + j * STEP);
        double my = (Y_LOWERLEFT + rsp.b * STEP);
        rsp.c[j] = mandelbrot_point(mx, my);
        //printf("%d\n", rsp.c[j]);
      }

      // send the response
      mq_send (mq_fd_response, (char *) &rsp, sizeof (rsp), 0);
    }


    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    return (0);
}

/*
 * rsleep(int t)
 *
 * The calling thread will be suspended for a random amount of time
 * between 0 and t microseconds
 * At the first call, the random generator is seeded with the current time
 */
static void rsleep (int t)
{
    static bool first_call = true;

    if (first_call == true)
    {
        srandom (time (NULL) % getpid ());
        first_call = false;
    }
    usleep (random() % t);
}
