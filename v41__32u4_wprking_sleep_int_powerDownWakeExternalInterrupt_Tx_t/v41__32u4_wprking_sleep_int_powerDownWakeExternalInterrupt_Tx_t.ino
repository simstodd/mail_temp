//V40_32U4_client_sensor side
//need to add in sleep till interrupt -or- timer - example in moteino https://github.com/LowPowerLab/MailboxNotifier

// **** INCLUDES *****
#include "LowPower.h"
#include <RH_RF95.h>
#include <SPI.h>
#include "Adafruit_Si7021.h"
#include <RHReliableDatagram.h>

/************ Radio Setup for feather 32u4***************/
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RF95_FREQ 915.0
#define DEST_ADDRESS   1 //server M0 - not this board
#define MY_ADDRESS     2  //client - sensor - this one 32u4

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS); // Class to manage message delivery and receipt, using the driver declared above
/************ End of Radio Setup for feather 32u4***************/

/************ Si7021 Temp sensor setup ***************/
Adafruit_Si7021 sensor = Adafruit_Si7021(); //for now this one
/************End of  Si7021 Temp sensor setup ***************/

//global constants//
// 32u4 interrupt mapping for wake up pin choice
//      2 = INT2 on RxD1 #0 pin 20 on ic 
//      3 = INT3 on TXD1 #1 pin 21 on ic
//      0 = INT0 on SCL #3 pin 18 on ic
//      1 = INT1 on SDA #2 pin 19 on ic
const int wakeUpPin = 2; //this will be INT2
const long interval = 3000;           // interval to wait till starting sleep (milliseconds)
//global variables//
int aLert = 0; //holder for magnetic, reed or other motion sensor
volatile byte state = LOW;
int16_t packetnum = 0;  // packet counter, we increment per xmission
unsigned long previousMillis = 0;        // used for sleep timer


void setup()
{
    delay(5500); //needed to avoid no serial issue on boot to reload program
    Serial.begin(9600);
  while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED_BUILTIN, OUTPUT);   // initialize led as an output.
  pinMode(RFM95_RST, OUTPUT);   // reset pin (4 on 32u4) as an output.
  digitalWrite(RFM95_RST, HIGH);
  pinMode(wakeUpPin, INPUT);   // Configure wake up pin as input ,consumes few uA of current.
  Serial.println("Feather Addressed RFM95 TX - Client 32u4 Test v41! sleep till for 8s loops or interrupt");
   
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

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }//end for
}//end blink

void loop() 
{
 attachInterrupt(wakeUpPin, wakeUp, LOW); // Allow wake up pin to trigger interrupt on low.

 unsigned long currentMillis = millis(); //why have this in loop?
    if (currentMillis - previousMillis >= interval){
      transmitStatus();  
      previousMillis = currentMillis;  // save the last time you slept
     } //end if

/////sleep code/////
    Serial.println("Sleeping radio");
    delay(200);
    rf95.sleep(); //sleep radio
      //SLEEP 32U4
        Serial.println("Sleeping MCU for 8s");
        delay(200);
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);   // _FOREVER Enter power down state with ADC and BOD module disabled.
        detachInterrupt(wakeUpPin);  // Wake up when wake up pin is low, disble wake up pin when awake
        delay(2000);                       // wait for a second
}//end main void loop

void transmitStatus()
{
  //read temp and humid then convert
      float temperatureF = (sensor.readTemperature() * 9.0 / 5.0) + 32.0; //also available Serial.print(sensor.readHumidity(), 2);
      float HumidVal = sensor.readHumidity(); 
      char databufT[7]; char databufH[7];
      if (aLert >0) {
        aLert--;
        }
      // float to a char array
      dtostrf(HumidVal, 3, 2, databufH);//results returned to databuf //4 is mininum width, 2 is precision; float value is copied onto buff   
      dtostrf(temperatureF, 3, 2, databufT);//results returned to databuf //4 is mininum width, 2, 4 optional  is precision; float value is copied onto buff
    //send packet
    char radiopacket[40] = "Hello World: #         ";
    sprintf(radiopacket,"Status: A:%d T:%s H:%s #", aLert, databufT, databufH);
    itoa(packetnum++, radiopacket+29, 10);
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
          Blink(LED_BUILTIN, 40, 3); //blink LED 3 times, 40ms between blinks
    }//endif rf95_manager.revfromAck
        else {
          Serial.println("No reply, is anyone listening?");
            }//end else
      }//end rf95_manager.sendtowait 
    else {
         Serial.println("Sending failed (no ack)");
}//end transmitStatus
}

void wakeUp()
{
    // Just a handler for the pin interrupt. //doesn't seem to allow anything to work here
     Serial.println("wakeup from interrupt ");
     Serial.println("setting alert ....");
     delay(200);
     aLert = 5;
     Blink(LED_BUILTIN, 100, 6); //blink LED 6 times, 100ms between blinks
     Blink(LED_BUILTIN, 880, 3); //blink LED 3 times, 880ms between blinks
}

//end program

