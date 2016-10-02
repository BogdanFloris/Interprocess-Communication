/*
 * Operating Systems   (2INCO)   Practical Assignment
 * Interprocess Communication
 *
 * Contains definitions which are commonly used by the farmer and the workers
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include "settings.h"


// TODO: put your definitions of the datastructures here
typedef struct
{
    int                     a;
} MQ_REQUEST_MESSAGE;

typedef struct
{
    int                     b;
    int                     c[X_PIXEL];
} MQ_RESPONSE_MESSAGE;


#endif
