/*******************************************************************************
 * Internet of Things (IoT) smart monitoring
 * device for agriculture using LoRaWAN technology.
 * 
 * LoRa Gateway:           Single-channel Dragino LG01-P LoRa Gateway 
 *
 * Measurement parameters: Temperature (Celcius)
 *                         Humidity (Relative Humidity %)
 *                         Light Intenisty (Volts)
 *                         Soil Moisture (Volts)
 *
 * Evaluation board:       FRDM-K64F ARM mbed board
 *
 * LoRa shield:            Semtech SX1272MB2xAS
 * 
 * IoT Cloud Server:       The Things Network (Europe EU-868.1 frequency band)     
 * 
 * API Platform:           All Things Talk Maker
 *
 * - Time-triggered program periodically sends playload data (including 
 * temperature, humidity, light intensity and soil moisture sensor parameters)
 * by using FRDM-K64F ARM mbed board and Semtech SX1272MB2xAS as the LoRa Node.
 *
 * - DHT library and Digital Input pin used for the successful measurement 
 * of temperature and humidity sensor parameters.
 *
 * - Analog Input pins used for the successful employement of soil moisture and
 * light intensity sensor parameters.
 *
 * - Semtech's SX1272Lib used for the successful configuration and set up of 
 * of SX1272MB2xAS LoRa shield.  
 * 
 * - IBM's LMiC library used for the successful implementation of LoRa modulation.
 *
 * - LoRa Node transmitting playload data directly to the single-channel Dragino
 * LG01-P LoRa Gateway which is connected to The Things Network Cloud Server.
 * 
 * - ABP (Activation By Personalization) selected as the activation method over 
 * The Things Network Cloud Server.
 *
 * - The Things Network Cloud Server makes playload data available online.
 *
 * - Through required integration, The Things Network Cloud Server passes 
 * playload data to All Things Talk Maker API which visualizes the data in 
 * a meaningful way for end-user's reference.  
 *
 * - Fully reset device through RESET BUTTON pressed.       
 * 
 * @Author: Giorgos Tsapparellas
 * 
 * Code available at: 1) https://os.mbed.com/users/GTsapparellas/code/LoRaWAN_mbed_lmic_agriculture_app/
 *                    2) https://github.com/GTsapparellas/LoRaWAN_mbed_lmic_agriculture_app
 *
 * SEE readME.txt file for instructions of how to compile and run the program.
 *
 *******************************************************************************/

#include <mbed.h>
#include <lmic.h>
#include <hal.h>
#include <SPI.h>
#include <DHT.h>
#include <debug.h>

///////////////////////////////////////////////////
// DEFINITION DECLARATIONS                      //
/////////////////////////////////////////////////

#define MAX_EU_CHANNELS 16     // Frequency channels automatically initialized for EU reqion. 
#define SINGLE_CHANNEL_GATEWAY // Force it to use 868.1 MHz frequency band only due to Dragino LG01-P LoRa Gateway hardware limitation.   
#define TRANSMIT_INTERVAL 300 // Transmit interval in seconds, too often may get traffic ignored.
#define DEBUG_LEVEL 0          // Set debug level to 1 for outputting messages to the UART Terminal (e.g. Tera Term).
#define ACTIVATION_METHOD 0    // Set activation method to 0 for ABP (Activation By Personalization)
                               // Set activation method to 1 for OTAA (Over The Air Activation)

///////////////////////////////////////////////////
// GLOBAL VARIABLES DECLARATIONS                //
/////////////////////////////////////////////////

// Transmit interval in seconds.
uint16_t transmit_interval = TRANSMIT_INTERVAL;

// Static osjob_t sendjob variable used by loop function
static osjob_t sendjob;

// Unsigned integer packet counter used by transmit function.
unsigned int packetCounter = 1;

// Disable data rate adaption.
// Set to 1 in order to enable data rate adaption.
static const bit_t disableAdrMode = 0; 

// Disable link check validadtion.
// Set to 1 in order to enable link check validation.
static const bit_t disableLinkCheck = 0; 

// Set LoRa Node's transmission power to 14 dBm.
static const s1_t txPower = 14;

// Set LoRa Node's network id to 0x1.
static const u4_t NETID = 0x1;

/* LMiC frame initializations. */

// Playload frame length of size 8. 
static const u1_t LMIC_FRAME_LENGTH = 8;

// Set listening port to 1.
static const u1_t LMIC_PORT = 1;

// Disable confirmation of transmitted LMiC data.
// Set to 1 in order to enable confirmation of transmitted LMiC data.
static const u1_t LMIC_CONFIRMED = 0;

#if ACTIVATION_METHOD == 1 // if OTAA (Over The Air Activation) is applied.

// LoRaWAN Application identifier (AppEUI) associated with The Things Network Cloud Server.
static const u1_t APPEUI[8]  = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0xA4, 0x54 };

// LoRaWAN unique device ID (DevEUI) associated with The Things Network Cloud Server.
static const u1_t DEVEUI[8]  = { 0x00, 0x1D, 0x45, 0x32, 0xEC, 0xA8, 0x01, 0x59 };

#endif

// Acquired activation method
#if ACTIVATION_METHOD == 0 // if ABP (Activation By Personalization) is applied.

// LoRaWAN network session key (NwkSKey) associated with The Things Network Cloud Server.
static const u1_t NWKSKEY[16] = { 0xDF, 0x9B, 0xB1, 0x30, 0xE8, 0x33, 0x42, 0x76, 0x33, 0x0C, 0x88, 0xBB, 0x30, 0xE2, 0xC2, 0xE9 };

// LoRaWAN application session key (AppSKey) associated with The Things Network Cloud Server.    
static const u1_t APPSKEY[16] = { 0xE0, 0x52, 0x18, 0x15, 0x0B, 0xE1, 0xEF, 0x1F, 0xAF, 0x8C, 0x8A, 0x31, 0x09, 0xB9, 0xAB, 0x9C };

// LoRaWAN end-device address (DevAddr) associated with The Things Network Cloud Server.
static const u4_t DEVADDR = 0x26011B39 ;

#endif

/* Sensor declarations. */

// Digital Input pin of temperature and humidity sensor set to D6.
DHT sensorTempHum(D6, DHT11);

// Analog Input pin of light intensity sensor set to A1.
AnalogIn sensorLight(A1);

// Analog Input pin of soil moisture sensor set to A3.
AnalogIn sensorSoilMoisture(A3);

///////////////////////////////////////////////////
// LMiC APPLICATION CALLBACKS                   //
/////////////////////////////////////////////////

/* 
 * os_getArtEui callback of type void.
 *
 * Copies application ID (8 bytes) of The Things Network Cloud Server into
 * memory if OTAA(Over The Air Activation) is applied.
 *
 * Input parameters: unsigned char buf of APPEUI. 
 *
 */ 
void os_getArtEui (u1_t* buf) {
    #if ACTIVATION_METHOD == 1 // if OTAA (Over The Air Activation) is applied.
        memcpy(buf, APPEUI, 8);
    #endif
}// end of os_getArtEui callback.

/* 
 * os_getDevEui callback of type void.
 *
 * Copies device ID (8 bytes) of The Things Network Cloud Server into
 * memory if OTAA(Over The Air Activation) is applied.
 *
 * Input parameters: unsigned char buf of DEVEUI. 
 *
 */ 
void os_getDevEui (u1_t* buf) {
    #if ACTIVATION_METHOD == 1 // if OTAA (Over The Air Activation) is applied.
        memcpy(buf, DEVEUI, 8);
    #endif
}// end of os_getDevEui callback.

/* 
 * os_getDevKey callback of type void.
 *
 * Copies device network session ID (16 bytes) of The Things Network Cloud Server
 * into memory.
 *
 * Input parameters: unsigned char buf of NWKSKEY. 
 *
 */ 
void os_getDevKey (u1_t* buf) {
    memcpy(buf, NWKSKEY, 16);
}// end of os_getDevKey callback.

/* 
 * onEvent callback of type void.
 *
 * Outputs UART message depending on
 * related event.
 *
 * NOTICE THAT: Not all events are being
 * used due to significant set up 
 * of LMiC environment.
 *
 * Input parameters: ev_t event. 
 *
 */ 
void onEvent (ev_t ev) {

    switch(ev) { // Switch events.
        case EV_SCAN_TIMEOUT:
            printf("EV_SCAN_TIMEOUT\n"); // Scan timeout.
            break;
        case EV_BEACON_FOUND:
            printf("EV_BEACON_FOUND\n"); // Beacon found.
            break;
        case EV_BEACON_MISSED:
            printf("EV_BEACON_MISSED\n"); // Beacon missed.
            break;
        case EV_BEACON_TRACKED:
            printf("EV_BEACON_TRACKED\n"); // Beacon tracked.
            break;
        case EV_JOINING:
            printf("EV_JOINING\n"); // Joining the network.
            break;
        case EV_JOINED:
            printf("EV_JOINED\n"); // Network joined.
            break;
        case EV_RFU1:
            printf("EV_RFU1\n"); // RFU1 event.
            break;
        case EV_JOIN_FAILED:
            printf("EV_JOIN_FAILED\n"); // Joining failed.
            break;
        case EV_REJOIN_FAILED:
            printf("EV_REJOIN_FAILED\n"); // Re-joining failed.
            break;
        case EV_TXCOMPLETE:
            printf("EV_TXCOMPLETE\n"); // Transmission complete.
            if (LMIC.txrxFlags & TXRX_ACK) // Check if acknowledgment received.
            {
                printf("Received ack\n");
            }
            if(LMIC.dataLen) // Output playload's data length.
            {
                printf("Received ");
                printf("%u", LMIC.dataLen);
                printf(" bytes of payload\n");
            }
            break;
        case EV_LOST_TSYNC:
            printf("EV_LOST_TSYNC\n"); // Lost transmision sync.
            break;
        case EV_RESET:
            printf("EV_RESET\n"); // Reset.
            break;
        case EV_RXCOMPLETE:
            printf("EV_RXCOMPLETE\n"); // Reception complete.
            break;
        case EV_LINK_DEAD:
            printf("EV_LINK_DEAD\n"); // Link dead.
            break;
        case EV_LINK_ALIVE:
            printf("EV_LINK_ALIVE\n"); // Link alive.
            break;
       default:
            printf("Unknown event\n"); // Default unknown event.
            break;
    }
    printf("\n"); // New line.
}// end of onEvent callback.

///////////////////////////////////////////////////
// LOCAL FUNCTIONS DECLARATIONS                 //
/////////////////////////////////////////////////

/* 
 * setUp function of type void.
 *
 * Initializes mbed OS, LMiC OS as well as
 * disabling all channels but 0 (868.1 MHz) for single-channel 
 * gateway compatibility.  
 *
 * Input parameters: None.
 *
 */ 
void setUp() {
    
    #if DEBUG_LEVEL == 1
        printf("IoT smart monitoring device for agriculture using LoRaWAN technology\n\n");
    #endif
    // Initializes OS.
    os_init(); 
    
    #if DEBUG_LEVEL == 1
        printf("OS_INIT\n\n");
    #endif
    
    // Reset the MAC state. Session and pending data transfers are being discarded.
    LMIC_reset();
  
    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    LMIC_setSession (NETID, DEVADDR, (uint8_t*)NWKSKEY, (uint8_t*)APPSKEY);
  
    // Disable data rate adaptation.
    LMIC_setAdrMode(disableAdrMode);
  
    // Disable link check validation.
    LMIC_setLinkCheckMode(disableLinkCheck);
  
    // Disable beacon tracking.
    LMIC_disableTracking();
  
    // Stop listening for downstream data (periodical reception) as LoRa Node
    // is only transmitting data to the Gateway.
    LMIC_stopPingable();
     
    // Set data rate and transmit power.
    LMIC_setDrTxpow(DR_SF7,txPower);

    //fflush(stdout);
    
    // If single-channel gateway is being used disable 
    // all the other channels except channel 0. 
    #ifdef SINGLE_CHANNEL_GATEWAY
        #if DEBUG_LEVEL == 1
            printf("      ----->Disabling all channels but 0 (868.1 MHz) for single-channel gateway compatibility\n\n\n");
        #endif
    
        for (int i=1; i < MAX_EU_CHANNELS; i++)
        {
            LMIC_disableChannel(i);
        }
    #endif
  
    #if DEBUG_LEVEL == 1
        printf("//////////Entering into TIME-TRIGGERED packet sending through LoRaWAN//////////\n");
        printf("---------------------Packets to be sent every %u seconds----------------------\n\n", transmit_interval);
    #endif
}// end of setUp function.

/* 
 * getTemperatureHumidity function of type void.
 *
 * Gets temperature (celcius) and humidity (relative humidity %)
 * measurements using DHT library. Otherwise, print an error.
 *
 * Input parameters: float temperature
 *                   float humidity
 *
 */ 
void getTemperatureHumidity(float& temperature, float& humidity) {

    // Set err variable to 0 (none).
    uint8_t err = ERROR_NONE;
    // Set humidity variable to 0.
    humidity = 0.0f; 
    // Set temperature variable to 0.
    temperature = 0.0f;
    
    // Store sensor data (40 bits(16-bit temperature, 16-bit humidity and 8-bit
    // CRC checksum)) into err variable.
    err = sensorTempHum.readData();
    
    if (err == ERROR_NONE) // if err equals to 0.
    { 
        // Store float temperature value in celcius.
        temperature = sensorTempHum.ReadTemperature(CELCIUS);
        // Store float humidity value. 
        humidity = sensorTempHum.ReadHumidity();
        
        // Output temperature and humidity values on UART Terminal
        #if DEBUG_LEVEL == 1
            printf("Temperature:   %4.2f Celsius \r\n", temperature);
            printf("Humidity:      %4.2f Relative Humidity \r\n", humidity);
        #endif
    }
    else // if err occurs.
    {
        // Output error message on UART Terminal and flash the RED LED.
        #if DEBUG_LEVEL == 1
            printf("Error: %d\r\n", err);
        #endif    
    }
}// end of getTemperatureHumidity function.

/* 
 * getLightIntensity function of type void.
 *
 * Gets the light's intensity analogue value at first instance
 * and then converts it using 16-bit ADC converter into voltage
 * counting from 0.0 to 5.0. 
 *
 * Input parameters: float lightIntensityVoltage
 *
 */ 
void getLightIntensity(float& lightIntensityVoltage) {
    
    // Set light intensity voltage variable to 0.
    lightIntensityVoltage = 0.0f;
    // Set light intensity analogue value to 0.
    uint16_t lightIntensityAnalogue = 0;
   
    // Read light intensity 16-bit analogue value.
    lightIntensityAnalogue = sensorLight.read_u16();
    //Convert the light intensity analog reading (which goes from 0 - 65536) to a voltage (0 - 5V).
    lightIntensityVoltage = (float) lightIntensityAnalogue*(5.0/65536.0);
    
    // Output light intensity voltage as well as resistance value on UART Terminal.
    #if DEBUG_LEVEL == 1
        float resistance = 0.0f;
        // Groove's calculation for resistance value.
        resistance = (float)(65536-lightIntensityAnalogue)*10/lightIntensityAnalogue;
        printf("Light Intensity:  %2.2f Volts -- ", lightIntensityVoltage);
        printf("Resistance: %2.2f Kiloohm \r\n", resistance);
    #endif
}// end of getLightIntensity function.

/* 
 * getSoilMoisture function of type void.
 *
 * Gets the soil's moisture analogue value at first instance
 * and then converts it using 16-bit ADC converter into voltage
 * counting from 0.0 to 5.0. 
 *
 * Input parameters: float lightOutputVoltage
 *
 */ 
void getSoilMoisture(float& soilMoistureVoltage) {
    
    // Set soil moisture voltage variable to 0.
    soilMoistureVoltage = 0.0f;
    // Set soil moisture analogue value to 0.
    uint16_t soilMoistureAnalogue = 0;
    
    // Read soil moisture 16-bit analogue value.
    soilMoistureAnalogue = sensorSoilMoisture.read_u16();
    // Convert the soil moisture analog reading (which goes from 0 - 65536) to a voltage (0 - 5V).
    soilMoistureVoltage = (float) soilMoistureAnalogue*(5.0/65536.0);
    
    // Output soil moisture voltage as well as soil moisture analogue value on UART Terminal.
    #if DEBUG_LEVEL == 1
        printf("Soil Moisture: %2.2f Volts -- ", soilMoistureVoltage);
        printf("Analogue Value: %d \r\n", soilMoistureAnalogue);
    #endif
}// end of getSoilMoisture function.

/* 
 * transmit function of type void.
 *
 * Checking if channel is ready.
 * If no, waiting until channel becomes free.
 * If yes, calling getTemperatureHumidity,
 * getLightIntensity and getSoilMoisture functions 
 * to gather measuring parameters. Then, prepares 
 * the LoRa packet and finally sending the packet
 * using os_setTimedCallback LMiC callback.
 * 
 * Input parameters: osjob_t* j
 *
 */ 
void transmit(osjob_t* j)
{
    // Define sensor measuring parameters.
    float temperature, humidity, lightIntensity, soilMoisture;
    
    #if DEBUG_LEVEL == 1
        printf("txChannel: %u , Channel Ready? ", LMIC.txChnl);
    #endif
    
    if (LMIC.opmode & (1 << 7)) // Is channel ready for transmission?
    {
        printf("NO, waiting...\n\n");
    } 
    else 
    {
        #if DEBUG_LEVEL == 1
            printf("YES, sensor readings...\n\n");
        #endif
        
        // Gather sensor readings.
        getTemperatureHumidity(temperature, humidity);
        getLightIntensity(lightIntensity);
        getSoilMoisture(soilMoisture); 
      
        #if DEBUG_LEVEL == 1
            printf("      ----->Preparing LoRa packet...\n");
        #endif
        
        // Prepare upstream data transmission at the next possible time.
        // Multiply all sensor readings by 100.
        int16_t temp = temperature*100;
        int16_t hum = humidity*100;
        int16_t light = lightIntensity*100;
        int16_t soil = soilMoisture*100;
      
        // Allocate measurements into LMIC frame array ready for transmission.
        // Each sensor measurement allocates 2 positions in frame array.
        // First, value is right shifted for 8 bits, while on the next 
        // array's position least significant bits are taken as the desired 
        // value for each sensor measurement.
        // This procedure is required in order to send playload data
        // to The Things Network Cloud Server.
        // Data will then be converted in a meaningful way through 
        // All Things Talk ABCL custom JSON binary conversion script.  
        LMIC.frame[0] = temp >> 8;
        LMIC.frame[1] = temp & 0xFF;
        LMIC.frame[2] = hum >> 8;
        LMIC.frame[3] = hum & 0xFF; 
        LMIC.frame[4] = light >> 8;
        LMIC.frame[5] = light & 0xFF;
        LMIC.frame[6] = soil >> 8;
        LMIC.frame[7] = soil & 0xFF;
      
        // Set the transmission data.
        LMIC_setTxData2(LMIC_PORT, LMIC.frame, LMIC_FRAME_LENGTH, LMIC_CONFIRMED);
        
        #if DEBUG_LEVEL == 1
            printf("      ----->LoRa Packet READY\n\n");
            printf("      ----->Sending LoRa packet %u of byte size %u\n\n", packetCounter++, LMIC_FRAME_LENGTH);
        #endif
    }
    // Schedule a time-triggered job to run based on TRANSMIT_INTERVAL time value.
    os_setTimedCallback(j, os_getTime()+sec2osticks(TRANSMIT_INTERVAL), transmit);
    
}// end of transmit function.

/* 
 * loop function of type void.
 *
 * Calling transmit as well as os_runloop_once functions
 * for a repeatedly time-triggered behaviour 
 * program execution.
 *
 * Input parameters: None.
 *
 */ 
void loop()
{
    // Calling transmit local function for acquiring next transmission
    // job of LoRa Node.
    transmit(&sendjob);

    // Super loop running os_runloop_once LMiC callback in a time-triggered behaviour. 
    while(1)
    {
        // Calling LMiC os_runloop_once callback.
        os_runloop_once();
        // Delay of 20 ms.
        wait_ms(20);
    }
    // Never arives here!
    
}// end of loop function.

/* 
 * main function of type integer.
 *
 * Calling setUp as well as loop functions
 * for a repeatedly time-triggered behaviour 
 * program execution.
 *
 * Input parameters: integer argc.
 *                   char **argv. 
 *
 */ 
int main(int argc, char **argv) 
{
    // Calling setUp local function for OS initialization.
    setUp();
    
    // Super loop running loop local funcion in a time-triggered behaviour.
    while(1)
    { 
        loop();
    }    
    // Never arrives here!
    
} // end of main function. 