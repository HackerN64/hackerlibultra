
/*====================================================================
 * os_time.h
 *
 * Copyright 1995, Silicon Graphics, Inc.
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics,
 * Inc.; the contents of this file may not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in part,
 * without the prior written permission of Silicon Graphics, Inc.
 *
 * RESTRICTED RIGHTS LEGEND:
 * Use, duplication or disclosure by the Government is subject to
 * restrictions as set forth in subdivision (c)(1)(ii) of the Rights
 * in Technical Data and Computer Software clause at DFARS
 * 252.227-7013, and/or in similar or successor clauses in the FAR,
 * DOD or NASA FAR Supplement. Unpublished - rights reserved under the
 * Copyright Laws of the United States.
 *====================================================================*/

/*---------------------------------------------------------------------*
        Copyright (C) 1998 Nintendo. (Originated by SGI)

        $RCSfile: os_time.h,v $
        $Revision: 1.1 $
        $Date: 1998/10/09 08:01:19 $
 *---------------------------------------------------------------------*/

#ifndef _OS_TIME_H_
#define _OS_TIME_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>
#include "os_message.h"

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Type definitions
 *
 */

/*
 * Structure for time value
 */
typedef u64 OSTime;

/*
 * Structure for interval timer
 */
typedef struct OSTimer_s {
    struct OSTimer_s* next; /* point to next timer in list */
    struct OSTimer_s* prev; /* point to previous timer in list */
    OSTime interval;        /* duration set by user */
    OSTime value;           /* time remaining before */
                            /* timer fires           */
    OSMesgQueue* mq;        /* Message Queue */
    OSMesg msg;             /* Message to send */
} OSTimer;

#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

/**************************************************************************
 *
 * Global definitions
 *
 */

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Macro definitions
 *
 */

/**************************************************************************
 *
 * Extern variables
 *
 */

/**************************************************************************
 *
 * Function prototypes
 *
 */

/* Timer interface */

extern OSTime osGetTime(void);
extern void osSetTime(OSTime);
extern int osSetTimer(OSTimer*, OSTime, OSTime, OSMesgQueue*, OSMesg);
extern int osStopTimer(OSTimer*);

#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_TIME_H_ */
