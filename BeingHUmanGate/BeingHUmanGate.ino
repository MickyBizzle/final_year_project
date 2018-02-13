#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include "fast_send.h"

bool debug = false;

RFM69 radio;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

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
Payload theData;

void setup() {
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,1,100);
  radio.setHighPower(); //uncomment only for RFM69HW!
  radio.encrypt(KEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
//  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
//  Serial.println(buff);
//  if (flash.initialize())
//    Serial.println("SPI Flash Init OK!");
//  else
//    Serial.println("SPI Flash Init FAIL! (is chip present?)");

  if(!setModemConfig(FSK_Rb250Fd250))
  {
    Serial.println("Error setting up modem");  
  }
    
}

byte ackCount=0;
void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input == 'r') //d=dump all register values
      radio.readAllRegs();
    if (input == 'E') //E=enable encryption
      radio.encrypt(KEY);
    if (input == 'e') //e=disable encryption
      radio.encrypt(null);
    if (input == 'p')
    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
    }
    
    if (input == 'd') //d=dump flash area
    {
      Serial.println("Flash content:");
      int counter = 0;

      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        Serial.print('.');
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'D')
    {
      Serial.print("Deleting Flash chip content... ");
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
  }

  if (radio.receiveDone())
  {
    if(debug) 
    {   Serial.print('[');
        Serial.print(radio.SENDERID, DEC);Serial.print("] ");
        Serial.print(" [RX_RSSI:");
        Serial.print(radio.readRSSI());
        Serial.print("]");
    }
    
    if (promiscuousMode)
    {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }

    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
    if(debug)   Serial.print(" nodeId=");
      Serial.print(radio.SENDERID);
      Serial.print(",");
    if(debug)   Serial.print(" packetId=");
      Serial.print(theData.packetId);
      Serial.print(",");
    if(debug)   Serial.print(" bpm=");
      Serial.print(theData.bpm);
      Serial.print(",");
    if(debug)   Serial.print(" mov=");
      Serial.print(theData.mov);
      Serial.print(",");
    if(debug)   Serial.print(" batt=");
      Serial.print((int)(theData.batt * 100));
      Serial.print(",");
    if(debug)   Serial.print(" temp=");
      Serial.print((int)(theData.temp * 100));
      Serial.print(",");
    if(debug)   Serial.print(" gsr=");
      Serial.println(theData.gsr);
    }
    
    if (radio.ACKRequested())
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
 /*
      Serial.print(" - ACK sent.");

      // When a node requests an ACK, respond to the ACK
      // and also send a packet requesting an ACK (every 3rd one only)
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ackCount++%3==0)
      {
        Serial.print(" Pinging node ");
        Serial.print(theNodeID);
        Serial.print(" - ACK...");
        delay(3); //need this when sending right after reception .. ?
        if (radio.sendWithRetry(theNodeID, "ACK TEST", 8, 0))  // 0 = only 1 attempt, no retries
          Serial.print("ok!");
        else Serial.print("nothing");
      }
    */
     }
  // Serial.println();
    Blink(LED,3);
  }
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
void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
