/*******************************************************************************
 * Copyright (c) 2014 IBM Corporation.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors:
 *    IBM Zurich Research Lab - initial API, implementation and documentation
 *    Semtech Apps Team       - Modified to support the MBED sx1276 driver
 *                              library.
 *                              Possibility to use original or Semtech's MBED
 *                              radio driver. The selection is done by setting
 *                              USE_SMTC_RADIO_DRIVER preprocessing directive
 *                              in lmic.h
 * /////////////////////////////////////////////////////////////////////////////
 *
 * Used by Giorgos Tsapparellas for Internet of Things (IoT) smart monitoring
 * device for agriculture using LoRaWAN technology.
 *
 * Date of issued copy: 25 January 2018
 *
 * Modifications: 
 * - Modified to meet SX1272MB2xAS LoRa shield's pin allocations.
 *
 * Notice that, connectivity for SX1272MB2xAS LoRa shield is allocated as:
 * SX1272MB2xAS   MBED Pin
 * SCK             D13
 * MOSI            D11
 * MISO            D12
 * NSS             D10
 * DIO0            D2
 * DIO1            D3
 * DIO2            D4
 * DIO3            D5
 * NRESET          A0
 *
 * - Added some external comments for meeting good principles of 
 *   source code re-usability. 
 *******************************************************************************/
 
#include "mbed.h"
#include "lmic.h"
#include "mbed_debug.h"

#if !USE_SMTC_RADIO_DRIVER

extern void radio_irq_handler( u1_t dio );

static DigitalOut nss( D10 ); // nss
static SPI spi( D11, D12, D13 ); // ( mosi, miso, sclk )
 
static DigitalInOut rst( A0 ); // rst (reset)
static DigitalOut rxtx( A4 ); // rx and tx
 
static InterruptIn dio0( D2 ); // dio0
static InterruptIn dio1( D3 ); // dio1
static InterruptIn dio2( D4 ); // dio2 

/* 
 * dio0Irq function of type void.
 *
 * Input parameters: None
 *
 */ 
static void dio0Irq( void ) {
    radio_irq_handler( 0 );
}// end of dio0Irq function.

/* 
 * dio1Irq function of type void.
 *
 * Input parameters: None
 *
 */ 
static void dio1Irq( void ) {
    radio_irq_handler( 1 );
}// end of dio1Irq funtion.

/* 
 * dio2Irq function of type void.
 *
 * Input parameters: None
 *
 */ 
static void dio2Irq( void ) {
    radio_irq_handler( 2 );
}// end of dio2Irq function.

#endif

static u1_t irqlevel = 0;
static u4_t ticks = 0;

static Timer timer;
static Ticker ticker;

/* 
 * reset_timer function of type void.
 *
 * Input parameters: None
 *
 */ 
static void reset_timer( void ) {
    ticks += timer.read_us( ) >> 6;
    timer.reset( );
}// end of reset_timer function.

/* 
 * hal_init function of type void.
 *
 * Input parameters: None
 *
 */ 
void hal_init( void ) {
     __disable_irq( );
     irqlevel = 0;

#if !USE_SMTC_RADIO_DRIVER
    // Configure input lines.
    dio0.mode( PullDown );
    dio0.rise( dio0Irq );
    dio0.enable_irq( );
    dio1.mode( PullDown );   
    dio1.rise( dio1Irq );
    dio1.enable_irq( );
    dio2.mode( PullDown );
    dio2.rise( dio2Irq );
    dio2.enable_irq( );
    // Configure reset line.
    rst.input( );
    // Configure spi.
    spi.frequency( 8000000 );
    spi.format( 8, 0 );
    nss = 1;
#endif
    // Configure timer.
    timer.start( );
    ticker.attach_us( reset_timer, 10000000 ); // reset timer every 10sec
     __enable_irq( );
}// end of hal_init function.

#if !USE_SMTC_RADIO_DRIVER

/* 
 * hal_pin_rxtx function of type void.
 *
 * Input parameters: unsigned char val
 *
 */ 
void hal_pin_rxtx( u1_t val ) {
    rxtx = !val;
}// end of hal_pin_rxtx function.

/* 
 * hal_pin_nss function of type void.
 *
 * Input parameters: unsigned char val
 *
 */ 
void hal_pin_nss( u1_t val ) {
    nss = val;
}// end of hal_pin_nss function.

/* 
 * hal_pin_rst function of type void.
 *
 * Input parameters: unsigned char val
 *
 */ 
void hal_pin_rst( u1_t val ) {
    if( val == 0 || val == 1 )
    { // drive pin
        rst.output( );
        rst = val;
    } 
    else
    { // keep pin floating
        rst.input( );
    }
}//end of hal_pin_rst function.

/* 
 * hal_spi function of type unsigned char.
 *
 * Input parameters: unsigned char out
 *
 * Return: spi out value
 */ 
u1_t hal_spi( u1_t out ) {
    return spi.write( out );
}// end of hal_spi function.

#endif

/* 
 * hal_disableIRQs function of type void.
 *
 * Input parameters: None
 *
 */ 
void hal_disableIRQs( void ) {
    __disable_irq( );
    irqlevel++;
}// end of hal_disableIRQs function.

/* 
 * hal_enableIRQs function of type void.
 *
 * Input parameters: None
 *
 */ 
void hal_enableIRQs( void ) {
    if( --irqlevel == 0 )
    {
        __enable_irq( );
    }
}// end of hal_enableIRQs function.

/* 
 * hal_sleep function of type void.
 *
 * Input parameters: None
 *
 */ 
void hal_sleep( void ) {
    // NOP
}// end of hal_sleep function.

/* 
 * hal_ticks function of type unsigned int.
 *
 * Input parameters: None
 * Return: t value 
 *
 */ 
u4_t hal_ticks( void ) {
    hal_disableIRQs( );
    int t = ticks + ( timer.read_us( ) >> 6 );
    hal_enableIRQs( );
    return t;
}//end of hal_ticks function.

/* 
 * deltaticks function of type unsigned short.
 *
 * Input parameters: unsigned int time
 * Return: d time
 */ 
static u2_t deltaticks( u4_t time ) {
    u4_t t = hal_ticks( );
    s4_t d = time - t;
    if( d <= 0 ) {
        return 0;    // in the past
    }
    if( ( d >> 16 ) != 0 ) {
        return 0xFFFF; // far ahead
    }
    return ( u2_t )d;
}// end of deltaticks function.

/* 
 * hal_waitUntil function of type void.
 *
 * Input parameters: unsigned int time
 *
 */ 
void hal_waitUntil( u4_t time ) {
    while( deltaticks( time ) != 0 ); // busy wait until timestamp is reached
}// end of hal_waitUntil function.

/* 
 * hal_checkTimer function of type unsigned char.
 *
 * Input parameters: unsigned int time
 * Return: deltaticks time
 *
 */ 
u1_t hal_checkTimer( u4_t time ) {
    return ( deltaticks( time ) < 2 );
}// end of hal_checkTimer function.

/* 
 * hal_failed function of type void.
 *
 * Input parameters: None
 *
 */ 
void hal_failed( void ) {
    while( 1 );
}// end of hal_failed function.
