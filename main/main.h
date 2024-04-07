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

#define STACK_DEPTH 2048
#define BASE_PRIO 10

/* String Constants Declarations*/
extern const char rtrnNewLine[3]; // FIXME (MAGIC NUMBER)
extern const char heapFail[28]; // FIXME (MAGIC NUMBER)
extern const char mtxFail[25]; // FIXME (MAGIC NUMBER)

#endif /* MAIN_H_*/