
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include "fast_send.h"

#define ACCEL_Z_PIN A0
#define ACCEL_Y_PIN A1
#define ACCEL_X_PIN A2
#define GSR_PIN A3
#define VOLTAGE_PIN A4
#define PULSE_PIN A5

#define POWER_LED 5
#define PULSE_LED 6

boolean debug = false;

float const aToV = 3.3f / 1024;  
//int TRANSMITPERIOD = 300; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;



typedef struct {		
  uint8_t           nodeId; //store this nodeId
  uint16_t           packetId;
  uint16_t          bpm;
  uint16_t          mov;
  float             batt;
  float             temp;
  uint16_t          gsr;
} 
Payload;

//Payload queue[30];
Payload theData;

//const char STR_TEST[] PROGMEM                     = "TEST123";
//#define TEXTBUFFER_SIZE 25
//char textBuffer[TEXTBUFFER_SIZE];

//  Variables
//int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
//int blinkPin = 13;                // pin to blink led at each beat
//int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
//int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded! 
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat". 
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

int prevBPM = 0;
volatile int maxmov = 0;
// Regards Serial OutPut  -- Set This Up to your needs
//static boolean serialVisual = false;   // Set to 'false' by Default.  Re-set to 'true' to see Arduino Serial Monitor ASCII Visual Pulse 


void setup(){
  // pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  // pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  Serial.begin(115200);             // we agree to talk fast!

  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 

  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);

  radio.setHighPower(); 
  radio.encrypt(KEY);
  radio.promiscuous(false);

  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);

  if (flash.initialize())
    Serial.println("SPI Flash Init OK!");
  else
    Serial.println("SPI Flash Init FAIL! (is chip present?)");

  if(!setModemConfig(FSK_Rb250Fd250))
  {
    Serial.println("Error setting up modem");  
  }

  pinMode(ACCEL_X_PIN, INPUT);
  pinMode(ACCEL_Y_PIN, INPUT);
  pinMode(ACCEL_Z_PIN, INPUT);

  pinMode(VOLTAGE_PIN, INPUT);
  pinMode(PULSE_PIN, INPUT);
  pinMode(GSR_PIN, INPUT);

  pinMode(POWER_LED, OUTPUT);
  pinMode(PULSE_LED, OUTPUT);
   digitalWrite(POWER_LED,LOW);
   digitalWrite(PULSE_LED,LOW);

  for(int t = 0 ; t < 5; t++)
  {
   digitalWrite(POWER_LED,HIGH);
    delay(200);
   digitalWrite(POWER_LED,LOW);
    delay(200);
  }


 
  delay(500);
  initTemperature();
  initTempReading();
  delay(1000);
  // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
  // AND APPLY THAT VOLTAGE TO THE A-REF PIN
  //  analogReference(EXTERNAL);   

  //------------------------------------------------
  //theData.temp = 25.55;
  //theData.bpm = 100;
  //theData.mov = 50;//x+y+z;
  //------------------------------------------------

}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

boolean p = false;
float prevBatt = 0;
unsigned long startTime = 0;

bool batlow = false;
bool batignore_state = true;
bool batled_state = false;
byte batignore_count = 0;
#define BATSTATE_IGNORE 10
bool pulseled_state = LOW;

void loop(){

  theData.temp = getTempResult();
  theData.bpm = 0;
  theData.mov = maxmov;//x+y+z;
  theData.packetId += 1;
  maxmov = 0;
  p = false;


  theData.batt = analogRead(VOLTAGE_PIN);
  theData.batt = 2*(theData.batt * aToV);
  theData.batt = (0.8*prevBatt)+(0.2*theData.batt);
  prevBatt = theData.batt;

  
  if(prevBatt < 3.0)
    batlow = true;
  else
    batlow = false;
  
  if(batignore_state == true)
  {
    batignore_count++;
    if(batignore_count > BATSTATE_IGNORE)
      batignore_state = false;
  }

  if(batlow == true && batignore_state == false)  // ignore battery state for a few seconds after switch on
  {
    digitalWrite(POWER_LED,batled_state);
    batled_state = !batled_state;
  }
    
  delay(2);
  theData.gsr = analogRead(GSR_PIN); 

  digitalWrite(PULSE_LED,LOW);
  if (QS == true)
  {     //  A Heartbeat Was Found
    theData.bpm = BPM;
    prevBPM = BPM;
    QS = false;                      // reset the Quantified Self flag for next time    

    digitalWrite(PULSE_LED,HIGH);
    delay(50);
//    pulseled_state = !pulseled_state; 
  
  } 
  else
  {
    theData.bpm = prevBPM;
    p = true;
  }
  digitalWrite(PULSE_LED,LOW);




  initTempReading();


  startTime = millis();

  while(millis()-startTime < 500)
  {

    if(radio.sendWithRetry(GATEWAYID, (const void*)(&theData), sizeof(theData)))
    {
      //Serial.println("Success");
      break;
    }
   // Serial.println("Failed trying again");
  }
  
 //    theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(theData.nodeId);
      Serial.print(" packetId=");
      Serial.print(theData.packetId);
      Serial.print(" bpm=");
      Serial.print(theData.bpm);
      Serial.print(" mov=");
      Serial.print(theData.mov);
      Serial.print(" batt=");
      Serial.print(theData.batt);
      Serial.print(" temp=");
      Serial.print(theData.temp);
      Serial.print(" gsr=");
      Serial.println(theData.gsr);


  int d = (1000-(millis()-startTime));
  if(d<0)
  {
    d = 1;
  }
  delay(d);                             //  take a brea
}


void inline debugPrint()
{
  Serial.print(theData.packetId);
  Serial.print("=");
  Serial.print(theData.bpm);
  Serial.print("(");
  Serial.print(p);
  Serial.print(") , ");
  Serial.print(theData.mov);
  Serial.print(" , ");
  Serial.print(theData.temp);
  Serial.print(" , ");

  Serial.print(theData.batt);
  Serial.print(" , ");
  Serial.println(theData.gsr);
}



bool setModemConfig(ModemConfigChoice index)
{
  if (index > (signed int)(sizeof(MODEM_CONFIG_TABLE) / sizeof(ModemConfig)))
    return false;

  ModemConfig cfg;
  memcpy_P(&cfg, &MODEM_CONFIG_TABLE[index], sizeof(ModemConfig));

  radio.writeReg(0x02,cfg.reg_02);
  radio.writeReg(0x03,cfg.reg_03);
  radio.writeReg(0x04,cfg.reg_04);
  radio.writeReg(0x05,cfg.reg_05);
  radio.writeReg(0x06,cfg.reg_06);
  radio.writeReg(0x19,cfg.reg_19);
  radio.writeReg(0x1a,cfg.reg_1a);
  radio.writeReg(0x37,cfg.reg_37);

  return true;
}







