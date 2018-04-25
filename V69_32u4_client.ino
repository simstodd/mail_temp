// v69_32u4_R_Datagram_Client_with_BME280
// rf95_reliable_datagram_client.pde
//adding sleep till interrupt and timer checking, need to add packet counter and sensor status
//this is working except packet counter isn't setup correctly. follow example again or loot at v40-ish version
//intervals are set too short, just for testing
//need to remove serial debug strings
//need to examine sleep and wake up, it seems to take a couple jiggles and decay too soon

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
RH_RF95 driver(RFM95_CS,RFM95_INT); // Singleton instance of the radio driver
RHReliableDatagram manager(driver, CLIENT_ADDRESS);// Class to manage message delivery and receipt, using the driver declared above

const int wakeUpPin = 2; //this will be INT2 - use for mailbox door alarm  // changed wake up to be 
const long interval = 250000; // 1500000 = 150 secs interval to wait till starting sleep (milliseconds), but timer sleeps when unit sleeps
int aLert = 0; //holder for magnetic, reed or other motion sensor
volatile byte state = LOW;
int16_t packetnum = 0;  // packet counter, we increment per xmission
unsigned long previousMillis = 0; // used for sleep timer
unsigned long SleepMillis = 0;

void setup() 
{
delay(6000);
  Serial.begin(9600); //debug only
  while (!Serial) ; // Wait for serial port to be available
  if (!manager.init()) //start radio
    Serial.println("RF95 radio init failed");
  //rf95.setTxPower(23, false); // RFM95 with the PA_BOOST tx pin, transmitter powers from 5 to 23 dBm
  Serial.print("RFM95 radio @");  Serial.print((int)RF95_FREQ);  Serial.println(" MHz");
  delay (100);
//remove after debug//
  pinMode(LED_BUILTIN, OUTPUT);   // initialize led as an output.
//remove//

  pinMode(wakeUpPin, INPUT);   // Configure wake up pin as input ,consumes few uA of current.
  Serial.println("BME280 and Feather Addressed RFM95 TX - Client 32u4 Test v69! sleep till for 8s loops or interrupt");
  delay(200); //let sensor boot
  bool status;
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = sensor.begin();  //was bme.begin
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }//end sensor status if
}//end setup

uint8_t data[] = "Hello from 32u4 peripheral V66";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop()
{
 //send a message if its time
    unsigned long currentMillis = millis(); //long time since transmit? Yes=transmit, no=skip and sleep
    currentMillis = currentMillis + SleepMillis; //add the sleeptime to current 
    if (currentMillis - previousMillis >= interval){
     transmitStatus();  //actually send a message
      previousMillis = currentMillis;  // save the last time you slept
      SleepMillis = 0;//reset sleepcount
     } //end if current millis check and do routine 
     if (aLert > 2){
      transmitStatus();  //also transmit without waiting for interval time    
     }
     
 sleepyMCU(); //do the sleep thing
 delay(100); // wait a bit
}//end main loop

void transmitStatus()//test function
{
   //get BME data
      float temperatureF = ((sensor.readTemperature() * 9.0 / 5.0) + 32.0);
      delay(150);
      float HumidVal = sensor.readHumidity(); //%
      delay(150);
      float toddHumid = HumidVal;
      float PressureHPA = (sensor.readPressure() / 100.0F);
       delay(150);
      float AltitudeM = (sensor.readAltitude(SEALEVELPRESSURE_HPA));
       delay(150);
      char databufT[7]; char databufH[7]; char databufP[7]; char databufHH[7];
      dtostrf(HumidVal, 3, 2, databufH);//results returned to databuf //4 is mininum width, 2 is precision; float value is copied onto buff   
      dtostrf(temperatureF, 3, 2, databufT);//results returned to databuf //4 is mininum width, 2, 4 optional  is precision; float value is copied onto buf
      dtostrf(PressureHPA, 3, 2, databufP);
      dtostrf(toddHumid, 3, 2, databufHH);
      if (aLert >0) {
        aLert--;
        }
     char radiopacket[40] = "Hello World v69: #         ";
     sprintf(radiopacket,"Status: A:%d T:%s H:%s P:%s #\"", aLert, databufT, databufHH, databufP); // craft message packet
     itoa(packetnum++, radiopacket+39, 10); //itoa(packetnum++, radiopacket+29, 10); //itoa( ) function converts int to string
 
  if (manager.sendtoWait((uint8_t *)radiopacket, strlen(radiopacket), SERVER_ADDRESS)) {
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
     {
      //sent a packet?
      }
    } //end send packet
  }//end transmit status function

void wakeUp(){
   Serial.println("wakeup from interrupt ");
    aLert = 3; //set alert
  }//end ISR wakeup

void sleepyMCU()
{
    Serial.print("Slept for: ");
    attachInterrupt(wakeUpPin, wakeUp, LOW); // Allow wake up pin to trigger interrupt on low.
    driver.sleep(); //sleep radio first
    delay(100);
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);   // was _FOREVER Enter power down state with ADC and BOD module disabled.
    detachInterrupt(wakeUpPin);  // Wake up when wake up pin is low, disble wake up pin when awake
    SleepMillis = SleepMillis + 8000;
    Serial.println(SleepMillis);
}//end sleepy MCU
//end program




