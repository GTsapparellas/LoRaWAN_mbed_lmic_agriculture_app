/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello, world!", that
 * will be processed by The Things Network server.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
*  0.1% in g2).
 *
 * Change DEVADDR to a unique address!
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h, default is:
 *   #define CFG_sx1272_radio 1
 * for SX1272 and RFM92, but change to:
 *   #define CFG_sx1276_radio 1
 * for SX1276 and RFM95.
 *
 *******************************************************************************/
 
 // ADD IFDEF, AND events, comments and states
 // ADD to git hub as well

#include <mbed.h>
#include <lmic.h>
#include <hal.h>
#include <SPI.h>
#include <DHT.h>

#if 1
#define SINGLE_CHANNEL_GATEWAY //force it to use 902.3 MHz only
#define TRANSMIT_INTERVAL 1800//300 //in seconds, TOO OFTEN MAY GET YOUR TRAFFIC IGNORED
//#include "test_node_secrets_PROTOTYPE.h"
//static const u1_t APPEUI[8]  = MY_APPEUI;
//static const u1_t DEVEUI[8]  = MY_DEVEUI;
//static const u1_t DEVKEY[16] = MY_NWKSKEY;
//static const u1_t ARTKEY[16] = MY_APPSKEY;
//static const u4_t DEVADDR = MY_DEVADDR;
//#else
//#define TRANSMIT_INTERVAL 120
//#warning YOU SHOULD SET CREATE AN APP AND A NODE IDENTITY (use a personal node)
//#warning SEE http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart
// LoRaWAN Application identifier (AppEUI)
// Not used in this example
uint8_t LMIC_FRAME_LENGTH = 8;

static const u1_t APPEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0xA4, 0x5B };

// LoRaWAN DevEUI, unique device ID (LSBF)
// Not used in this example
static const u1_t DEVEUI[8]  = { 0x00, 0x5A, 0xE2, 0x2A, 0x73, 0x7B, 0x7A, 0x77 };

// LoRaWAN NwkSKey, network session key
// Use this key for The Things Network
static const u1_t DEVKEY[16] = { 0xDF, 0x9B, 0xB1, 0x30, 0xE8, 0x33, 0x42, 0x76, 0x33, 0x0C, 0x88, 0xBB, 0x30, 0xE2, 0xC2, 0xE9 };

// LoRaWAN AppSKey, application session key
// Use this key to get your data decrypted by The Things Network
static const u1_t ARTKEY[16] = { 0xE0, 0x52, 0x18, 0x15, 0x0B, 0xE1, 0xEF, 0x1F, 0xAF, 0x8C, 0x8A, 0x31, 0x09, 0xB9, 0xAB, 0x9C };

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
//#warning FIXME You MUST SET A CUSTOM ADDRESS
static const u4_t DEVADDR = 0x26011B39 ; // <-- Change this address for every node!

#endif

/*On board's LEDs Declaration */
DigitalOut RED_LED(PTB22);                  // PTB22 = Red pin -- Indicates an error occur
DigitalOut GREEN_LED(PTE26);                // PTE26 = Green pin -- Indicates transmission is in progress
DigitalOut BLUE_LED(PTB21);                 // PTB21 = Blue pin -- Indicates MCU is sleeping

DHT sensorTempHum(D6, DHT11);
AnalogIn sensorLight(A1);
AnalogIn sensorSoilMoisture(A3);

void getTemperatureHumidity(float& temperature, float& humidity) {

    uint8_t err = ERROR_NONE;
    humidity = 0.0f; 
    temperature = 0.0f;
    
    err = sensorTempHum.readData();
    
    if (err == ERROR_NONE) {
        
        temperature = sensorTempHum.ReadTemperature(CELCIUS);
        humidity = sensorTempHum.ReadHumidity();
        //printf("Temperature:   %4.2f Celsius \r\n", temperature);
        //printf("Humidity:      %4.2f Percentage Relative Humidity \r\n", humidity);
    }
    else{
        printf("Error: %d\r\n", err);
        //RED_LED = !RED_LED;    
    }
}

void getLightOutput(float& lightOutputVoltage) {
    lightOutputVoltage = 0.0f;
    uint16_t lightOutputAnalogue = 0;
    //float resistance = 0.0f;
    
    lightOutputAnalogue = sensorLight.read_u16();
    lightOutputVoltage = (float) lightOutputAnalogue*(5.0/65536.0);
    //resistance = (float)(65536-lightOutputAnalogue)*10/lightOutputAnalogue;
    //printf("Light Output:  %2.2f Volts -- ", lightOutputVoltage);
    //printf("Resistance: %2.2f Kilooohm \r\n", resistance);
}

void getSoilMoisture(float& soilMoistureVoltage) {
    soilMoistureVoltage = 0.0f;
    uint16_t soilMoistureAnalogue = 0;
    
    soilMoistureAnalogue = sensorSoilMoisture.read_u16();
    soilMoistureVoltage = (float) soilMoistureAnalogue*(5.0/65536.0);
    //printf("Soil Moisture: %2.2f Volts -- ", soilMoistureVoltage);
    //printf("Analogue Value: %d \r\n", soilMoistureAnalogue);
}

//////////////////////////////////////////////////
// APPLICATION CALLBACKS
//////////////////////////////////////////////////

// provide application router ID (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
    //memcpy(buf, APPEUI, 8);
}

// provide device ID (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
    //memcpy(buf, DEVEUI, 8);
}

// provide device key (16 bytes)
void os_getDevKey (u1_t* buf) {
    //memcpy(buf, DEVKEY, 16);
}

//uint8_t mydata[] = "Hello, from NYC!";
static osjob_t sendjob;

void onEvent (ev_t ev) {
    //debug_event(ev);

    switch(ev) {
      // scheduled data sent (optionally data received)
      // note: this includes the receive window!
      case EV_TXCOMPLETE:
          // use this event to keep track of actual transmissions
          //Serial.print("Event EV_TXCOMPLETE, time: ");
          //Serial.println(millis() / 1000);
          if(LMIC.dataLen) { // data received in rx slot after tx
              //debug_buf(LMIC.frame+LMIC.dataBeg, LMIC.dataLen);
              //Serial.println("Data Received!");
              printf("Data Received!\n");
          }
          break;
       default:
          break;
    }
}

unsigned int xmit_count = 1;
void do_send(osjob_t* j){
        //printf("txCnhl: %u , Channel Ready? ", LMIC.txChnl);
    if (LMIC.opmode & (1 << 7)) {
      //Serial.println("OP_TXRXPEND, not sending");
        printf("NO, waiting...\n\n");
    } else {
      //Serial.println("ok");
      //printf("YES, sensor readings...\n\n");
      // Prepare upstream data transmission at the next possible time.
      //   LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
      //char buf[128];
      float temperature;
      float humidity;
      float lightOutput;
      float soilMoisture;
      getTemperatureHumidity(temperature, humidity);
      getLightOutput(lightOutput);
      getSoilMoisture(soilMoisture); 

      int16_t temp = temperature*100;
      int16_t hum = humidity*100;
      int16_t light = lightOutput*100;
      int16_t soil = soilMoisture*100;
      
      //printf("      ----->Preparing packet...\n");
      
      LMIC.frame[0] = temp >> 8;
      LMIC.frame[1] = temp & 0xFF;
      LMIC.frame[2] = hum >> 8;
      LMIC.frame[3] = hum & 0xFF; 
      LMIC.frame[4] = light >> 8;
      LMIC.frame[5] = light & 0xFF;
      LMIC.frame[6] = soil >> 8;
      LMIC.frame[7] = soil & 0xFF;
      
      //printf("      ----->Packet READY\n\n");
     // sprintf(buf, "%4.2f%4.2f\n", temperature, humidity/*, lightOutput, soilMoisture*/);
      //sprintf(buf, "NYC TTN test packet %u\n", xmit_count++);/
      LMIC_setTxData2(1, /*(unsigned char *) buf*/LMIC.frame, 8/*strlen(buf)-1*/, 0/*0*/);
      //printf("      ----->Sending packet %u of byte size %u\n\n", xmit_count++, LMIC_FRAME_LENGTH);
    }
    // Schedule a timed job to run at the given timestamp (absolute system time)
    os_setTimedCallback(j, os_getTime()+sec2osticks(TRANSMIT_INTERVAL), do_send);

}

void setup() {
//  Serial.begin(9600);
  //Serial.println("Starting");
  //printf("IoT smart monitoring device for agriculture using LoRaWAN technology\n\n");
  //SPI.begin();
  //delay(3000); //Give teensy USB some time
  //while (millis() < 1000);
  //Serial.println("setting up");
  //printf("setting up\n");
  //fprintf(stderr, "stderr test\n");
  // LMIC init
  os_init();
  //printf("MBED_OS_INIT\n\n");
  //printf("os_init\n");
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  LMIC_setSession (0x1, DEVADDR, (uint8_t*)DEVKEY, (uint8_t*)ARTKEY);
  // Disable data rate adaptation
  LMIC_setAdrMode(0);
  // Disable link check validation
  LMIC_setLinkCheckMode(0);
  // Disable beacon tracking
  LMIC_disableTracking ();
  // Stop listening for downstream data (periodical reception)
  LMIC_stopPingable();
  //LMIC.txpow = 27;
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14);
  //printf("LMiC_LoRa_MAC_INIT\n\n");
  //
//  Serial.flush();
    fflush(stdout);
  #ifdef SINGLE_CHANNEL_GATEWAY
  //Serial.println("Disabling all channels but 0 (902.3 MHz) for single-channel gateway compatibility");
  //printf("      ----->Disabling all channels but 0 (868.0 MHz) for single-channel gateway compatibility\n\n\n");
  for (int i=1; i<16; i++)
   LMIC_disableChannel(i);
  #endif
  //printf("//////////Entering into TIME-TRIGGERED packet sending through LoRaWAN//////////\n");
  //printf("---------------------Packets to be sent every ~30 seconds----------------------\n\n");
}

void loop() {

do_send(&sendjob);

while(1) {
  os_runloop_once();
  wait_ms(20);
  }
  }

int main(int argc, char **argv) {
    setup();
    while(1) loop();
    }

 