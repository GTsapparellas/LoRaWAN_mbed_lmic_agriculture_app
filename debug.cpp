/*******************************************************************************
 * Copyright (c) 2014-2015 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *    Semtech Apps Team       - Adapted for MBED
 *******************************************************************************/
#include <stdio.h>
#include "lmic.h"
#include "debug.h"

void debug_init () {
    // print banner
    debug_str("\r\n============== DEBUG STARTED ==============\r\n");
}

void debug_led (u1_t val) {
    debug_val( "LED = ", val );
}

void debug_char (u1_t c) {
    fprintf(stderr, "%c", c );
}

void debug_hex (u1_t b) {
    fprintf(stderr, "%02X", b );
}

void debug_buf (const u1_t* buf, u2_t len) {
    while( len-- ) {
        debug_hex( *buf++ );
        debug_char( ' ' );
    }
    debug_char( '\r' );
    debug_char( '\n' );
}

void debug_uint (u4_t v) {
    for( s1_t n = 24; n >= 0; n -= 8 ) {
        debug_hex( v >> n );
    }
}

void debug_str (const u1_t* str) {
    while( *str ) {
        debug_char( *str++ );
    }
}

void debug_val (const u1_t* label, u4_t val) {
    debug_str( label );
    debug_uint( val );
    debug_char( '\r' );
    debug_char( '\n' );
}

void debug_event (int ev) {
    static const u1_t* evnames[] = {
        [EV_SCAN_TIMEOUT]   = "SCAN_TIMEOUT",
        [EV_BEACON_FOUND]   = "BEACON_FOUND",
        [EV_BEACON_MISSED]  = "BEACON_MISSED",
        [EV_BEACON_TRACKED] = "BEACON_TRACKED",
        [EV_JOINING]        = "JOINING",
        [EV_JOINED]         = "JOINED",
        [EV_RFU1]           = "RFU1",
        [EV_JOIN_FAILED]    = "JOIN_FAILED",
        [EV_REJOIN_FAILED]  = "REJOIN_FAILED",
        [EV_TXCOMPLETE]     = "TXCOMPLETE",
        [EV_LOST_TSYNC]     = "LOST_TSYNC",
        [EV_RESET]          = "RESET",
        [EV_RXCOMPLETE]     = "RXCOMPLETE",
        [EV_LINK_DEAD]      = "LINK_DEAD",
        [EV_LINK_ALIVE]     = "LINK_ALIVE",
    };
    debug_str(evnames[ev]);
    debug_char('\r');
    debug_char('\n');
}
