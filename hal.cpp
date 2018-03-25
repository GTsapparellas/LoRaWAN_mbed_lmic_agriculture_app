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
 *******************************************************************************/
#include "mbed.h"
#include "lmic.h"
#include "mbed_debug.h"

#if !USE_SMTC_RADIO_DRIVER

extern void radio_irq_handler( u1_t dio );

static DigitalOut nss( D10 );
static SPI spi( D11, D12, D13 ); // ( mosi, miso, sclk )
 
static DigitalInOut rst( A0 );
static DigitalOut rxtx( A4 );
 
static InterruptIn dio0( D2 );
static InterruptIn dio1( D3 );
static InterruptIn dio2( D4 ); 

static void dio0Irq( void ) {
    radio_irq_handler( 0 );
}

static void dio1Irq( void ) {
    radio_irq_handler( 1 );
}

static void dio2Irq( void ) {
    radio_irq_handler( 2 );
}

#endif

static u1_t irqlevel = 0;
static u4_t ticks = 0;

static Timer timer;
static Ticker ticker;

static void reset_timer( void ) {
    ticks += timer.read_us( ) >> 6;
    timer.reset( );
}

void hal_init( void ) {
     __disable_irq( );
     irqlevel = 0;

#if !USE_SMTC_RADIO_DRIVER
    // configure input lines
    dio0.mode( PullDown );
    dio0.rise( dio0Irq );
    dio0.enable_irq( );
    dio1.mode( PullDown );   
    dio1.rise( dio1Irq );
    dio1.enable_irq( );
    dio2.mode( PullDown );
    dio2.rise( dio2Irq );
    dio2.enable_irq( );
    // configure reset line
    rst.input( );
    // configure spi
    spi.frequency( 8000000 );
    spi.format( 8, 0 );
    nss = 1;
#endif
    // configure timer
    timer.start( );
    ticker.attach_us( reset_timer, 10000000 ); // reset timer every 10sec
     __enable_irq( );
}

#if !USE_SMTC_RADIO_DRIVER

void hal_pin_rxtx( u1_t val ) {
    rxtx = !val;
}

void hal_pin_nss( u1_t val ) {
    nss = val;
}

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
}

u1_t hal_spi( u1_t out ) {
    return spi.write( out );
}

#endif

void hal_disableIRQs( void ) {
    __disable_irq( );
    irqlevel++;
}

void hal_enableIRQs( void ) {
    if( --irqlevel == 0 )
    {
        __enable_irq( );
    }
}

void hal_sleep( void ) {
    // NOP
}

u4_t hal_ticks( void ) {
    hal_disableIRQs( );
    int t = ticks + ( timer.read_us( ) >> 6 );
    hal_enableIRQs( );
    return t;
}

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
}

void hal_waitUntil( u4_t time ) {
    while( deltaticks( time ) != 0 ); // busy wait until timestamp is reached
}

u1_t hal_checkTimer( u4_t time ) {
    return ( deltaticks( time ) < 2 );
}

void hal_failed( void ) {
    while( 1 );
}
