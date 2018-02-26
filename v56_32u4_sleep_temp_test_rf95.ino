//V52_32U4_client_sensor side
//need to add in sleep till interrupt -or- timer - example in moteino https://github.com/LowPowerLab/MailboxNotifier
//http://gammon.com.au/interrupts for more information on interrupts

//something wrong with temp and humid vaules
//need to install RH library
//install lowpower library
//install bme280 zip
//install adafruit library

// **** INCLUDES *****
#include "LowPower.h"//get it here: https://github.com/LowPowerLab/LowPower
#include <RH_RF95.h>
#include <SPI.h>
// #include <Wire.h> //from bme 280 library

#include <Adafruit_Sensor.h>

#include <Adafruit_BME280.h>

//using BME280 to also get pressure at the expense of less temp accuracy +-1 deg C versus +-0.1  //#include "Adafruit_Si7021.h"
#include <RHReliableDatagram.h>
// **** END OF INCLUDES *****

/************ Radio Setup for feather 32u4***************/
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0
#define DEST_ADDRESS   1 //server M0 - not this board
#define MY_ADDRESS     2  //client - sensor - this one 32u4

//for BME 280 sensor//
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 sensor; ////instatiates a sensor called sensor (or bme as in example)on i2c for here, could use SPI
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI


//BME 280 //


// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above
/************ End of Radio Setup for feather 32u4***************/

/************ Si7021 Temp sensor setup ***************/
//Adafruit_Si7021 sensor = Adafruit_Si7021(); //for now this one
/************End of  Si7021 Temp sensor setup ***************/

// **** Global ConstantsS *****

// 32u4 interrupt mapping for wake up pin choice
//      2 = INT2 on RxD1 #0 pin 20 on ic 
//      3 = INT3 on TXD1 #1 pin 21 on ic
//      0 = INT0 on SCL #3 pin 18 on ic
//      1 = INT1 on SDA #2 pin 19 on ic
const int wakeUpPin = 2; //this will be INT2 - use for mailbox door alarm
const long interval = 10000; // 1500000 = 150 secs interval to wait till starting sleep (milliseconds), but timer sleeps when unit sleeps
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
   delay(9000); //needed to avoid no serial issue on boot to reload program
   Serial.begin(9600);
  while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

//remove//
  pinMode(LED_BUILTIN, OUTPUT);   // initialize led as an output.
//remove//

  pinMode(RFM95_RST, OUTPUT);   // reset pin (4 on 32u4) as an output.
  digitalWrite(RFM95_RST, HIGH);
  pinMode(wakeUpPin, INPUT);   // Configure wake up pin as input ,consumes few uA of current.
  Serial.println("BME280, 32u4 Feather RFM95 TX - Client Test v52! sleep loop/interrupt");
    
    delay(1500); //let sensor boot
    
    Serial.println(F("BME280 test v52"));

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = sensor.begin();  //was bme.begin
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }



   
// manual reset
  digitalWrite(RFM95_RST, LOW); delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

if (!rf95_manager.init()) {
    Serial.println("RFM95 radio init failed");
    while (1);
  }//end rf95_manager
  Serial.println("RFM95 radio init OK!");
 
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
  }//end rf95 set freq

  rf95.setTxPower(23, false); // RFM95 with the PA_BOOST tx pin, transmitter powers from 5 to 23 dBm
  Serial.print("RFM95 radio @");  Serial.print((int)RF95_FREQ);  Serial.println(" MHz");
  delay (2000);

}//end setup

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t data[] = "  OK";


void loop() 
{
 
 //send a message
    unsigned long currentMillis = millis(); //long time since transmit? Yes=transmit, no=skip and sleep
    currentMillis = currentMillis + SleepMillis; //add the sleeptime to current 
   // if (currentMillis - previousMillis >= interval){
    //  transmitStatus();  
  
   sendTest();
   //   previousMillis = currentMillis;  // save the last time you slept
   //   SleepMillis = 0;//reset sleepcount
   //  } //end if
    

//sleep
// sleepyMCU(); //do the sleep thing
 delay(200); // wait
 Serial.print("SleepyMCU: ");
 Serial.println(SleepMillis);
 delay(200); // wait a bit
}//end main void loop


void transmitStatus()
{
  //read temp and humid then convert
      //float temperatureF = (sensor.readTemperature() * 9.0 / 5.0) + 32.0; //also available Serial.print(sensor.readHumidity(), 2);
      //test code//
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
      
delay(500);

      float tempF = ((sensor.readTemperature() * 9.0 / 5.0) + 32.0);

      float HumidVal = sensor.readHumidity(); //%
      float PressureHPA = (sensor.readPressure() / 100.0F);
      float AltitudeM = (sensor.readAltitude(SEALEVELPRESSURE_HPA));
   
     char HumidValC[8];
      dtostrf(HumidVal, 4, 2, HumidValC);
     
     char databufT[7]; 
      dtostrf(tempF, 3, 2, databufT);//results returned to databuf //4 is mininum width, 2, 4 optional  is precision; float value is copied onto buf
     
     char databufP[7];
      dtostrf(PressureHPA, 3, 2, databufP);

     char tempC[7]; 
      dtostrf((sensor.readTemperature()* (9.0 / 5.0) + 32.0), 3, 2, tempC);
   
   Serial.print("Humid: ");
   Serial.println(HumidValC);
   Serial.print("Temp: ");
   Serial.println(tempF);
   Serial.print("Temp2: ");
   Serial.println(tempC);
   Serial.print("Pressure: ");
   Serial.println(databufP);    
    
      if (aLert >0) {
        aLert--;
        }
     

    char radiopacket[45] = "Hello World: #         ";
    
    sprintf(radiopacket,"S: A:%d T:%s H:%s P:%s #", aLert, tempC, HumidValC, databufP);
    Serial.print("The radiopacket will be: ");
    Serial.println(radiopacket);
    //Serial.print("The databufH is: ");
    //Serial.println(databufH);
    itoa(packetnum++, radiopacket+30, 10); //itoa(packetnum++, radiopacket+29, 10);
    Serial.print("Sending "); Serial.println(radiopacket);
 
    // Send a message to the DESTINATION!
      if (rf95_manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), DEST_ADDRESS)) {
        // Now wait for a reply from the server
        uint8_t len = sizeof(buf);
        uint8_t from;   
        if (rf95_manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
        buf[len] = 0; // zero out remaining string  
          Serial.print("Got reply from server #"); Serial.print(from);Serial.print(" [RSSI :");
         Serial.print(rf95.lastRssi());  Serial.print("] : "); Serial.println((char*)buf);     
       //   Blink(LED_BUILTIN, 40, 3); //blink LED 3 times, 40ms between blinks
    }//endif rf95_manager.revfromAck
        else {
         Serial.println("No reply, is anyone listening?");
            }//end else
      }//end rf95_manager.sendtowait 
    else {
        Serial.println("Sending failed (no ack)");
        }//end else
}//end transmitStatus ()

void sendTest(){
Serial.println("Sending to rf95_reliable_datagram_server");
    
  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
  {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      Serial.print("got reply from : 0x");
      Serial.print(from, HEX);
      Serial.print(": ");
      Serial.println((char*)buf);
    }
    else
    {
      Serial.println("No reply, is rf95_reliable_datagram_server running?");
    }
  }
  else
    Serial.println("sendtoWait failed");
  delay(500);
  
}

void wakeUp()
{
    // Just a handler for the pin interrupt. //doesn't seem to allow anything to work here
    Serial.println("wakeup from interrupt ");
    Serial.println("setting alert ....");
     delay(200);
     aLert = 4;
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
    rf95.sleep(); //sleep radio first
      //SLEEP 32U4
        //remove
       Serial.println("Sleeping MCU for 8s");
        //remove
        delay(200);
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);   // was _FOREVER Enter power down state with ADC and BOD module disabled.
        detachInterrupt(wakeUpPin);  // Wake up when wake up pin is low, disble wake up pin when awake
        SleepMillis = SleepMillis + 8000;
}

//end program

