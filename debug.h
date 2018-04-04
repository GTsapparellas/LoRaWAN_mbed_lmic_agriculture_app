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
 *******************************************************************************/
#ifndef _debug_hpp_
#define _debug_hpp_

/* 
 * debug_init function of type void.
 *
 * Intializes debug library.
 *
 * Input parameters: None
 */
void debug_init (void);

/* 
 * debug_led function of type void.
 *
 * Sets LED state.
 *
 * Input parameters: unsigned char val.
 *
 */ 
void debug_led (u1_t val);

/* 
 * debug_char function of type void.
 *
 * Writes character to UART
 *
 * Input parameters: unsigned char c.
 *
 */ 
void debug_char (u1_t c);

/* 
 * debug_hex function of type void.
 *
 * Writes byte as two hex digits to UART.
 *
 * Input parameters: unsigned char b.
 *
 */ 
void debug_hex (u1_t b);

/* 
 * debug_buf function of type void.
 *
 * Writes buffer as hex dump to UART.
 *
 * Input parameters: unsigned char buf
 *                   unsigned short len 
 *
 */ 
void debug_buf (const u1_t* buf, u2_t len);

/* 
 * debug_uint function of type void.
 *
 * Writes 32-bit integer as eight hex digits to UART.
 *
 * Input parameters: unsigned int v 
 *
 */ 
void debug_uint (u4_t v);

/* 
 * debug_str function of type void.
 *
 * Writes nul-terminated string to UART.
 *
 * Input parameters: const unsigned char str 
 *
 */ 
void debug_str (const u1_t* str);

/* 
 * debug_val function of type void.
 *
 * Writes label and 32-bit value as hex to UART.
 *
 * Input parameters: const unsigned char label
 *                   unsigned int val 
 *
 */ 
void debug_val (const u1_t* label, u4_t val);

/* 
 * debug_event function of type void.
 *
 * Writes LMiC event name to UART.
 *
 * Input parameters: int ev 
 *
 */ 
void debug_event (int ev);

#endif // _debug_hpp_
