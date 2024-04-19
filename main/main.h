/* Texas A&M University
** Electronic Systems Engineering Technology
** ESET-420 Engineering Technology Capstone II
** Author: Warren Watts
** File: main.h
** ----------
** ......
*/

#ifndef MAIN_H_
#define MAIN_H_

/* Standard Library Headers */
#include <stdio.h>

/* Driver Headers */
#include "sdkconfig.h"

/* Defines */

#define DEF_PEND pdMS_TO_TICKS(10)
#define LOCK_DELAY pdMS_TO_TICKS(10000)
#define NO_WAIT 0

#define STACK_DEPTH 2048
#define BASE_PRIO 10

/* Enum for Global Constant String Sizes */
typedef enum 
{
    NEWLINE_LEN = 3,
    MALLOC_LEN = 14,
    MTX_LEN = 25,
    HEAP_LEN = 28,

} glblStrLengths;

/* String Constants Declarations*/
extern const char rtrnNewLine[NEWLINE_LEN];
extern const char heapFail[HEAP_LEN];
extern const char mtxFail[MTX_LEN];

#endif /* MAIN_H_*/