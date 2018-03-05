// v60_32u4_R_Datagram_Client_with_BME280
// rf95_reliable_datagram_client.pde
//adding sleep till interrupt and timer checking, need to add packet counter and sensor status
//this is working except packet counter isn't setup correctly. follow example again or loot at v40-ish version
//intervals are set too short, just for testing

#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "LowPower.h"

// **** END OF INCLUDES *****

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
delay(10000);

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
      Serial.print(", RSSI:");
      Serial.print(driver.lastRssi());
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
    driver.sleep(); //sleep radio first
      //SLEEP 32U4
        //remove
       Serial.println("Sleeping MCU for 8s");
        //remove
        delay(200);
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

//end program




