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
 * /////////////////////////////////////////////////////////////////////////////
 *
 * Used by Giorgos Tsapparellas for Internet of Things (IoT) smart monitoring
 * device for agriculture using LoRaWAN technology.
 * 
 * Date of issued copy: 25 January 2018
 *
 * Modifications: 
 * - No external modifications of the existing "AS IT IS" software.
 * - Added some external comments for meeting good principles of 
 *   source code re-usability.   
 ******************************************************************************/
 
#include <stdio.h>
#include "lmic.h"
#include "debug.h"

/* 
 * debug_init function of type void.
 *
 * Input parameters: None
 *
 */ 
void debug_init () {
    // print banner
    debug_str("\r\n============== DEBUG STARTED ==============\r\n");
}// end of debug_init function.

/* 
 * debug_led function of type void.
 *
 * Input parameters: unsigned char val.
 *
 */ 
void debug_led (u1_t val) {
    debug_val( "LED = ", val );
}// end of debug_led function.

/* 
 * debug_char function of type void.
 *
 * Input parameters: unsigned char c.
 *
 */ 
void debug_char (u1_t c) {
    fprintf(stderr, "%c", c );
}// end of debug_char function.

/* 
 * debug_hex function of type void.
 *
 * Input parameters: unsigned char b.
 *
 */ 
void debug_hex (u1_t b) {
    fprintf(stderr, "%02X", b );
}// end of debug_hex function.

/* 
 * debug_buf function of type void.
 *
 * Input parameters: unsigned char buf
 *                   unsigned short len 
 *
 */ 
void debug_buf (const u1_t* buf, u2_t len) {
    while( len-- ) {
        debug_hex( *buf++ );
        debug_char( ' ' );
    }
    debug_char( '\r' );
    debug_char( '\n' );
}// end of debug_buf function.

/* 
 * debug_uint function of type void.
 *
 * Input parameters: unsigned int v 
 *
 */ 
void debug_uint (u4_t v) {
    for( s1_t n = 24; n >= 0; n -= 8 ) {
        debug_hex( v >> n );
    }
}// end of debug_uint function.

/* 
 * debug_str function of type void.
 *
 * Input parameters: const unsigned char str 
 *
 */ 
void debug_str (const u1_t* str) {
    while( *str ) {
        debug_char( *str++ );
    }
}// end of debug_str function.

/* 
 * debug_val function of type void.
 *
 * Input parameters: const unsigned char label
 *                   unsigned int val 
 *
 */ 
void debug_val (const u1_t* label, u4_t val) {
    debug_str( label );
    debug_uint( val );
    debug_char( '\r' );
    debug_char( '\n' );
}// end of debug_val function.

/* 
 * debug_event function of type void.
 *
 * Input parameters: int ev 
 *
 */ 
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
}// end of debug_event function.
