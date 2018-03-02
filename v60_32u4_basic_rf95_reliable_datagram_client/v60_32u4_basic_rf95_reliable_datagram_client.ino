/*
https://learn.adafruit.com/adafruit-feather-32u4-radio-with-lora-radio-module?view=all
// v60_32u4_R_Datagram_Client_with_BME280
// rf95_reliable_datagram_client.pde

//adding sleep till interrupt and timer checking, need to add packet counter and sensor status
//this is working except packet counter isn't setup correctly. follow example again or loot at v40-ish version
//intervals are set too short, just for testing
The Feather 32u4 Radio uses the extra space left over to add an RFM9x LoRa 433 or 868/915 MHz radio module. 
These radios are not good for transmitting audio or video, but they do work quite well for small data packet transmission 
when you need more range than 2.4 GHz (BT, BLE, WiFi, ZigBee)

SX1272 LoRa® based module with SPI interface, Packet radio with ready-to-go Arduino libraries
Uses the amateur or license-free ISM bands: 433MHz is ITU "Europe" license-free ISM or ITU "American" 
amateur with limitations. 900MHz is license free ISM for ITU "Americas"
+5 to +20 dBm - up to 100 mW Power Output Capability (power output selectable in software)
~300uA during full sleep, ~120mA peak during +20dBm transmit, ~40mA during active radio listening.
Simple wire antenna or spot for uFL connector
Our initial tests with default library settings: over 1.2mi/2Km line-of-sight with wire quarter-wave antennas. 
(With setting tweaking and directional antennas, 20Km is possible)

This is the general purpose I/O pin set for the microcontroller. All logic is 3.3V

#0 / RX - GPIO #0, also receive (input) pin for Serial1 and Interrupt #2
#1 / TX - GPIO #1, also transmit (output) pin for Serial1 and Interrupt #3
#2 / SDA - GPIO #2, also the I2C (Wire) data pin. There's no pull up on this pin by default so when using with I2C, 
you may need a 2.2K-10K pullup. Also Interrupt #1
#3 / SCL - GPIO #3, also the I2C (Wire) clock pin. There's no pull up on this pin by default so when using with I2C, 
you may need a 2.2K-10K pullup. Can also do PWM output and act as Interrupt #0.
#5 - GPIO #5, can also do PWM output
#6 - GPIO #6, can also do PWM output and analog input A7
#9 - GPIO #9, also analog input A9 and can do PWM output. This analog input is connected to a voltage divider for the 
lipoly battery so be aware that this pin naturally 'sits' at around 2VDC due to the resistor divider
#10 - GPIO #10, also analog input A10 and can do PWM output.
#11 - GPIO #11, can do PWM output.
#12 - GPIO #12, also analog input A11 and can do PWM output.
#13 - GPIO #13, can do PWM output and is connected to the red LED next to the USB jack
A0 thru A5 - These are each analog input as well as digital I/O pins.
SCK/MOSI/MISO - These are the hardware SPI pins, used by the RFM radio module too! You can use them as everyday GPIO pins 
if you don't activate the radio and keep the RFM CS pin high. However, we really recommend keeping them free as they should 
be kept available for the radio module. If they are used, make sure its with a device that will kindly share the SPI bus! 
Also used to reprogram the chip with an AVR programmer if you need.
Since not all pins can be brought out to breakouts, due to the small size of the Feather, 
we use these to control the radio module

#8 - used as the radio CS (chip select) pin
#7 - used as the radio GPIO0 / IRQ (interrupt request) pin.
#4 - used as the radio Reset pin
Other Pins!
RST - this is the Reset pin, tie to ground to manually reset the AVR, as well as launch the bootloader manually
ARef - the analog reference pin. Normally the reference voltage is the same as the chip logic voltage (3.3V) but if you 
need an alternative analog reference, connect it to this pin and select the external AREF in your firmware. Can't go higher than 3.3V!

 */
  
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "LowPower.h"
//after testing try to use https://github.com/adafruit/Adafruit_SleepyDog
//#include "Adafruit_SleepyDog.h" 


// **** END OF INCLUDES *****

#define SENDEVERYXLOOPS   8 //each loop sleeps 8 seconds, so send status message every this many sleep cycles (default "4" = 32 seconds)
//the send every is from lowpowerlabs, the timeouts are probably different for this processor
//this might work
//*********************************************************************************************
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI -75
//*********************************************************************************************
//need to look at ATC

//
//To make this easy we stuck a double-100K resistor divider on the BAT pin, and connected it 
//to D9 (a.k.a analog #7 A7, this is probbaly a wrong comment). You can read this pin's voltage, then double it, to get the battery voltage.
  
#define VBATPIN A9 //maybe on the 32u4 this is A9 (aka D9)
/************ Radio Setup for feather 32u4***************/
#define CLIENT_ADDRESS 1 //this is me the 32u4 peripheral
#define SERVER_ADDRESS 2 //this is the server m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0

//for BME 280 sensor//
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 sensor;//use I2C, this instatiates a sensor called "sensor" 
//Adafruit_BME280 bme(BME_CS); // uses hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); //uses  software SPI

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS,RFM95_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, CLIENT_ADDRESS);

// **** Global ConstantsS *****

// 32u4 interrupt mapping for wake up pin choice
//      2 = INT2 on RxD1 #0 pin 20 on ic 
//      3 = INT3 on TXD1 #1 pin 21 on ic
//      0 = INT0 on SCL #3 pin 18 on ic
//      1 = INT1 on SDA #2 pin 19 on ic
const int wakeUpPin = 2; //this will be INT2 - use for mailbox door alarm  // changed wake up to be 
const long interval = 100000; // 1500000 = 150 secs interval to wait till starting sleep (milliseconds), but timer sleeps when unit sleeps
// **** End of Global ConstantsS *****

//**** global variables ****//
int aLert = 0; //holder for magnetic, reed or other motion sensor
volatile byte state = LOW;
int16_t packetnum = 0;  // packet counter, we increment per xmission
unsigned long previousMillis = 0; // used for sleep timer
unsigned long SleepMillis = 0;
//**** Global variables ****//


void setup() 
{
delay(8000);

  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init()) //start radio
    Serial.println("RF95 radio init failed");
  //rf95.setTxPower(23, false); // RFM95 with the PA_BOOST tx pin, transmitter powers from 5 to 23 dBm
  Serial.print("RFM95 radio @");  Serial.print((int)RF95_FREQ);  Serial.println(" MHz");
  delay (500);

    
//remove//
  pinMode(LED_BUILTIN, OUTPUT);   // initialize led as an output.
//remove//

  pinMode(wakeUpPin, INPUT);   // Configure wake up pin as input ,consumes few uA of current.
  Serial.println("BME280 and Feather Addressed RFM95 TX - Client 32u4 Test v60! sleep till for 8s loops or interrupt");
  delay(500); //let sensor boot
  Serial.println(F("BME280 test"));
  bool status;
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = sensor.begin();  //was bme.begin
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }//end sensor status if
}//end setup

uint8_t data[] = "Hello from 32u4 peripheral V60";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
  
 //send a message if its time
    unsigned long currentMillis = millis(); //long time since transmit? Yes=transmit, no=skip and sleep
    currentMillis = currentMillis + SleepMillis; //add the sleeptime to current 
    if (currentMillis - previousMillis >= interval){
     // transmitStatus();  //actually send a message
      previousMillis = currentMillis;  // save the last time you slept
      SleepMillis = 0;//reset sleepcount
     } //end if current millis check and do routine 
    
  Serial.print("Sending: ");
  transmitStatus();  //actually send a message 

//sleep
 sleepyMCU(); //do the sleep thing
 delay(200); // wait
 Serial.print("SleepyMCU: ");
 Serial.println(SleepMillis);
 delay(200); // wait a bit
  
 /*
 //send readings every SENDEVERYXLOOPS
  if (sendLoops>=SENDEVERYXLOOPS)
  {
    sendLoops=0;
    char periodO='X', periodC='X';
    unsigned long lastOpened = (time - MLO) / 1000; //get seconds
    unsigned long LO = lastOpened;
    
    if (lastOpened <= 59) periodO = 's'; //1-59 seconds
    else if (lastOpened <= 3599) { periodO = 'm'; lastOpened/=60; } //1-59 minutes
    else if (lastOpened <= 259199) { periodO = 'h'; lastOpened/=3600; } // 1-71 hours
    else if (lastOpened >= 259200) { periodO = 'd'; lastOpened/=86400; } // >=3 days

    if (periodO == 'd')
      sprintf(MLOstr, "LO:%ldd%ldh", lastOpened, (LO%86400)/3600);
    else if (periodO == 'h')
      sprintf(MLOstr, "LO:%ldh%ldm", lastOpened, (LO%3600)/60);
    else sprintf(MLOstr, "LO:%ld%c", lastOpened, periodO);
 
 //end sendeveryloops
 */ 
  
  
  
  
}//end main loop

void transmitStatus()//test function
{
  
  //get BME data
      float temperatureF = ((sensor.readTemperature() * 9.0 / 5.0) + 32.0);
      float HumidVal = sensor.readHumidity(); //%
      float PressureHPA = (sensor.readPressure() / 100.0F);
      float AltitudeM = (sensor.readAltitude(SEALEVELPRESSURE_HPA));
      char databufT[7]; char databufH[7];
      dtostrf(HumidVal, 3, 2, databufH);//results returned to databuf //4 is mininum width, 2 is precision; float value is copied onto buff   
      dtostrf(temperatureF, 3, 2, databufT);//results returned to databuf //4 is mininum width, 2, 4 optional  is precision; float value is copied onto buf
      if (aLert >0) {
        aLert--;
        }
      printValues();
  // Send a message to manager_server
  
  
  char radiopacket[40] = "Hello World v60: #         ";
  sprintf(radiopacket,"Status: A:%d T:%s H:%s #", aLert, databufT, databufH);
  Serial.print("Sending "); Serial.println(radiopacket);  
  
  if (manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), SERVER_ADDRESS))
  //if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
   //send data, radiopacket
   
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
      Serial.println("reading BME280");
      
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
}//end transmit status function


void wakeUp()
{
    // Just a handler for the pin interrupt. //doesn't seem to allow anything to work here
    Serial.println("wakeup from interrupt ");
    Serial.println("setting alert ....");
     delay(200);
     aLert = 9;
    for (byte i=0; i<4; i++)  {
    digitalWrite(LED_BUILTIN,HIGH);
    delay(250);
    digitalWrite(LED_BUILTIN,LOW);
    delay(250);
    } //end for blink loop
     //Blink(LED_BUILTIN, 100, 6); //blink LED 6 times, 100ms between blinks
     //Blink(LED_BUILTIN, 880, 3); //blink LED 3 times, 880ms between blinks
}

void sleepyMCU()
{
  /////sleep code/////
    //remove
    Serial.println("Sleeping radio");
    //attach interrupt so pin wakes up
    attachInterrupt(wakeUpPin, wakeUp, LOW); // Allow wake up pin to trigger interrupt on low.
    //remove
    delay(200);
  //alternative sleepy dog method https://github.com/adafruit/Adafruit_SleepyDog
  //radio.sleep();
  //Watchdog.sleep(1000);
  //test after trying other method lowpower
  /*
  
During the super sleepy mode you're using only 300uA (0.3mA)!
To put the chip into ultra-low-power mode. 
Note that USB will disconnect so do this after you have done all your debugging!
While its not easy to get the exact numbers for all of what comprise 
the 300uA there are a few quiescent current items on the Feather 32u4:
    2 x 100K resistors for VBAT measurement = 25uA
    AP2112K 3.3V regulator = 55uA
    MCP73871 batt charger = up to 100uA even when no battery is connected
The rest is probably the Atmega32u4 peripherals including the brown-out 
detect and bandgap circuitry, ceramic oscillator, etc. 
According to the datasheet, with the watchdog and BrownOutDetect enabled, 
the lowest possible current is ~30uA (at 5V which is what we're testing at)
Tie En pin to ground - ENable pin
If you'd like to turn off the 3.3V regulator, you can do that with the EN(able) pin.
Simply tie this pin to Ground and it will disable the 3V regulator. 
The BAT and USB pins will still be powered
  
  */
  
  
    driver.sleep(); //sleep radio first
      //SLEEP 32U4
        //remove
       Serial.println("Sleeping MCU for 8s");
        //remove
        delay(200);
  //now mcu sleep after radio sleep
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);   // was _FOREVER Enter power down state with ADC and BOD module disabled.
        detachInterrupt(wakeUpPin);  // Wake up when wake up pin is low, disble wake up pin when awake
        SleepMillis = SleepMillis + 8000;
}

void printValues() {
    Serial.print("Temperature = ");
    Serial.print(sensor.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(sensor.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(sensor.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(sensor.readHumidity());
    Serial.println(" %");

    Serial.println();
}

/*
If you're running off of a battery, chances are you wanna know what the voltage is at! 
That way you can tell when the battery needs recharging. Lipoly batteries are 'maxed out' at 4.2V 
and stick around 3.7V for much of the battery life, then slowly sink down to 3.2V or so before the 
protection circuitry cuts it off. By measuring the voltage you can quickly tell when you're heading 
below 3.7V

To make this easy we stuck a double-100K resistor divider on the BAT pin, and connected it 
to D9 (a.k.a analog #7 A7). You can read this pin's voltage, then double it, to get the battery voltage.

float measuredvbat = analogRead(VBATPIN);
measuredvbat *= 2;    // we divided by 2, so multiply back
measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
measuredvbat /= 1024; // convert to voltage
Serial.print("VBat: " ); Serial.println(measuredvbat);

}//end batValues
*/

//end program




