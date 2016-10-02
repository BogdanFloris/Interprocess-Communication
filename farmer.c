/*
 * Bogdan Floris
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>         // for execlp
#include <mqueue.h>         // for mq
#include <time.h>

#include "settings.h"
#include "output.h"
#include "common.h"

#define STUDENT_NAME        "Bogdan"

static char                 mq_name1[80];
static char                 mq_name2[80];


static void create_children(pid_t childrenPIDs[])
{
  int i;
  /* Start children. */
  for (i = 0; i < NROF_WORKERS; i++)
  {
    pid_t processID = fork(); //fork
    if (processID < 0)
    {
      // if fork is unsuccesful
      perror("fork");
      abort();
    }
    else if (processID == 0)
    {
      // launch workers using execlp() with the two messege queue arguments
      execlp("./worker", "worker", mq_name1, mq_name2, NULL);
      perror("execlp() failed");
    }
    //save children pids in order to see when they are finished
    childrenPIDs[i] = processID;
    //printf("%d\n", childrenPIDs[i]);
  }
}
//this function waits for all the children to finish
static void kill_children(pid_t childrenPIDs[])
{
  int i;
  for (i = 0; i < NROF_WORKERS; i++)
  {
    waitpid(childrenPIDs[i],NULL,0);
  }
}
//draw function
static void draw_result(MQ_RESPONSE_MESSAGE rsp)
{
  int i;
  for(i = 0; i < X_PIXEL; i++)
  {
    output_draw_pixel(i , rsp.b , rsp.c[i]);
  }
}

int main (int argc, char * argv[])
{
    if (argc != 1)
    {
        fprintf (stderr, "%s: invalid arguments\n", argv[0]);
    }
    output_init();
    // TODO:
    //  * create the message queues (see message_queue_test() in interprocess_basic.c)
    mqd_t               mq_fd_request;
    mqd_t               mq_fd_response;
    MQ_REQUEST_MESSAGE  req;
    MQ_RESPONSE_MESSAGE rsp;
    struct mq_attr      attr;

    sprintf (mq_name1, "/mq_request_%s_%d", STUDENT_NAME, getpid());
    sprintf (mq_name2, "/mq_response_%s_%d", STUDENT_NAME, getpid());

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_REQUEST_MESSAGE);
    mq_fd_request = mq_open (mq_name1, O_WRONLY | O_CREAT | O_EXCL, 0600, &attr);

    attr.mq_maxmsg  = MQ_MAX_MESSAGES;
    attr.mq_msgsize = sizeof (MQ_RESPONSE_MESSAGE);
    mq_fd_response = mq_open (mq_name2, O_RDONLY | O_CREAT | O_EXCL, 0600, &attr);
    //  * create the child processes (see process_test() and message_queue_test())
    pid_t childrenPIDs[NROF_WORKERS];//child pids array
    create_children(childrenPIDs);//creating children
    //  * do the farming (use output_draw_pixel() for the coloring)
    int sent_msg = 0; // a counter to keep the sent messeges
    int received_msg = 0; // a counter to keep received messeges

    while(sent_msg < Y_PIXEL || received_msg < Y_PIXEL)
    {
      //get attributes for request queue
      mq_getattr(mq_fd_request, &attr);
      //while queue is not full && there are still jobs to be done
      while (attr.mq_curmsgs < attr.mq_maxmsg && sent_msg < Y_PIXEL)
      {
        //send messeges
        req.a = sent_msg;
        mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
        sent_msg++;
        //printf("%d %d\n", sent_msg, req.a);
        mq_getattr(mq_fd_request, &attr);

      }
      //get attributes for response queue
      mq_getattr(mq_fd_response, &attr);
      //while there are still jobs computed by the worker to be drawn
      while(attr.mq_curmsgs > 0)
      {
        // receive done job from worker
        mq_receive (mq_fd_response, (char *) &rsp, sizeof (rsp), NULL);
        received_msg++;
        //draw
        draw_result(rsp);
        //add another messege
        if (sent_msg < Y_PIXEL)
        {
            req.a = sent_msg;
            mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
            sent_msg++;
        }
        mq_getattr(mq_fd_response, &attr);
      }
    }
    //close children
    int i;
    for (i = 0; i < NROF_WORKERS; i++)
    {
      req.a = -1;//send -1 for the workers to break and exit
      mq_send (mq_fd_request, (char *) &req, sizeof (req), 0);
    }

    kill_children(childrenPIDs);

    //close the buffers and unlink

    mq_close (mq_fd_response);
    mq_close (mq_fd_request);
    mq_unlink (mq_name1);
    mq_unlink (mq_name2);

    // Important notice: make sure that the names of the message queues contain your
    // student name and the process id (to ensure uniqueness during testing)

    output_end ();

    return (0);
}
