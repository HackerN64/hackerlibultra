
/*====================================================================
 * os_cache.h
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

        $RCSfile: os_cache.h,v $
        $Revision: 1.1 $
        $Date: 1998/10/09 08:01:04 $
 *---------------------------------------------------------------------*/

#ifndef _OS_CACHE_H_
#define _OS_CACHE_H_

#ifdef _LANGUAGE_C_PLUS_PLUS
extern "C" {
#endif

#include <PR/ultratypes.h>

#if defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS)

/**************************************************************************
 *
 * Type definitions
 *
 */

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

#define OS_DCACHE_ROUNDUP_ADDR(x) (void*)(((((u32)(x) + 0xf) / 0x10) * 0x10))
#define OS_DCACHE_ROUNDUP_SIZE(x) (u32)(((((u32)(x) + 0xf) / 0x10) * 0x10))

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

/* Cache operations and macros */

extern void osInvalDCache(void*, s32);
extern void osInvalICache(void*, s32);
extern void osWritebackDCache(void*, s32);
extern void osWritebackDCacheAll(void);

#endif /* defined(_LANGUAGE_C) || defined(_LANGUAGE_C_PLUS_PLUS) */

#ifdef _LANGUAGE_C_PLUS_PLUS
}
#endif

#endif /* !_OS_CACHE_H_ */
