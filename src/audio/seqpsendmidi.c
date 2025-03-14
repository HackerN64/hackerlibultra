/*====================================================================
 * seqpsendmidi.c
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

#include <libaudio.h>

void alSeqpSendMidi(ALSeqPlayer* seqp, s32 ticks, u8 status, u8 byte1, u8 byte2) {
    ALEvent evt;
    ALMicroTime deltaTime;

    evt.type = AL_SEQP_MIDI_EVT;
    evt.msg.midi.ticks = 0;
    evt.msg.midi.status = status;
    evt.msg.midi.byte1 = byte1;
    evt.msg.midi.byte2 = byte2;
    evt.msg.midi.duration = 0;

    deltaTime = ticks * seqp->uspt;

    alEvtqPostEvent(&seqp->evtq, &evt, deltaTime);
}
