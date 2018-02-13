#define NODEID      99
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF69_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack

#define RH_RF69_DATAMODUL_DATAMODE_PACKET                   0x00
#define RH_RF69_DATAMODUL_MODULATIONTYPE_FSK                0x00
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE        0x00
#define RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0       0x01
#define RH_RF69_PACKETCONFIG1_DCFREE_NONE                   0x00
#define RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER             0x20
#define RH_RF69_PACKETCONFIG1_DCFREE_WHITENING              0x40
#define RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE         0x80
#define RH_RF69_PACKETCONFIG1_CRC_ON                        0x10
#define RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE         0x00

//moteino{ REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 },
#define CONFIG_FSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_NONE)
#define CONFIG_GFSK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_FSK | RH_RF69_DATAMODUL_MODULATIONSHAPING_FSK_BT1_0)
//#define CONFIG_OOK (RH_RF69_DATAMODUL_DATAMODE_PACKET | RH_RF69_DATAMODUL_MODULATIONTYPE_OOK | RH_RF69_DATAMODUL_MODULATIONSHAPING_OOK_NONE)

// Choices for RH_RF69_REG_37_PACKETCONFIG1:
//moteino{ REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
#define CONFIG_NOWHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_NONE | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
#define CONFIG_WHITE (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_WHITENING | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
//#define CONFIG_MANCHESTER (RH_RF69_PACKETCONFIG1_PACKETFORMAT_VARIABLE | RH_RF69_PACKETCONFIG1_DCFREE_MANCHESTER | RH_RF69_PACKETCONFIG1_CRC_ON | RH_RF69_PACKETCONFIG1_ADDRESSFILTERING_NONE)
//  02,        03,   04,   05,   06,   19,   1a,  37
typedef struct 
{
  uint8_t    reg_02;   ///< Value for register RH_RF69_REG_02_DATAMODUL
  uint8_t    reg_03;   ///< Value for register RH_RF69_REG_03_BITRATEMSB
  uint8_t    reg_04;   ///< Value for register RH_RF69_REG_04_BITRATELSB
  uint8_t    reg_05;   ///< Value for register RH_RF69_REG_05_FDEVMSB
  uint8_t    reg_06;   ///< Value for register RH_RF69_REG_06_FDEVLSB
  uint8_t    reg_19;   ///< Value for register RH_RF69_REG_19_RXBW
  uint8_t    reg_1a;   ///< Value for register RH_RF69_REG_1A_AFCBW
  uint8_t    reg_37;   ///< Value for register RH_RF69_REG_37_PACKETCONFIG1

} 
ModemConfig;


 typedef enum ModemConfigChoice 
{
  FSK_Rb2Fd5 = 0,    ///< FSK, Whitening, Rb = 2kbs,    Fd = 5kHz
  FSK_Rb2_4Fd4_8,    ///< FSK, Whitening, Rb = 2.4kbs,  Fd = 4.8kHz 
  FSK_Rb4_8Fd9_6,    ///< FSK, Whitening, Rb = 4.8kbs,  Fd = 9.6kHz 
  FSK_Rb9_6Fd19_2,   ///< FSK, Whitening, Rb = 9.6kbs,  Fd = 19.2kHz
  FSK_Rb19_2Fd38_4,  ///< FSK, Whitening, Rb = 19.2kbs, Fd = 38.4kHz
  FSK_Rb38_4Fd76_8,  ///< FSK, Whitening, Rb = 38.4kbs, Fd = 76.8kHz
  FSK_Rb57_6Fd120,   ///< FSK, Whitening, Rb = 57.6kbs, Fd = 120kHz
  FSK_Rb125Fd125,    ///< FSK, Whitening, Rb = 125kbs,  Fd = 125kHz
  FSK_Rb250Fd250,    ///< FSK, Whitening, Rb = 250kbs,  Fd = 250kHz
  FSK_Rb55555Fd50,   ///< FSK, Whitening, Rb = 55555kbs,Fd = 50kHz for RFM69 lib compatibility

}ModemConfigChoice;

PROGMEM static const ModemConfig MODEM_CONFIG_TABLE[]=
{
  { 
    CONFIG_FSK,  0x3e, 0x80, 0x00, 0x52, 0xf4, 0xf4, CONFIG_WHITE  }
  , // FSK_Rb2Fd5      
  { 
    CONFIG_FSK,  0x34, 0x15, 0x00, 0x4f, 0xf4, 0xf4, CONFIG_WHITE  }
  , // FSK_Rb2_4Fd4_8
  { 
    CONFIG_FSK,  0x1a, 0x0b, 0x00, 0x9d, 0xf4, 0xf4, CONFIG_WHITE  }
  , // FSK_Rb4_8Fd9_6

  { 
    CONFIG_FSK,  0x0d, 0x05, 0x01, 0x3b, 0xf4, 0xf4, CONFIG_WHITE  }
  , // FSK_Rb9_6Fd19_2
  { 
    CONFIG_FSK,  0x06, 0x83, 0x02, 0x75, 0xf3, 0xf3, CONFIG_WHITE  }
  , // FSK_Rb19_2Fd38_4
  { 
    CONFIG_FSK,  0x03, 0x41, 0x04, 0xea, 0xf2, 0xf2, CONFIG_WHITE  }
  , // FSK_Rb38_4Fd76_8

  { 
    CONFIG_FSK,  0x02, 0x2c, 0x07, 0xae, 0xe2, 0xe2, CONFIG_WHITE  }
  , // FSK_Rb57_6Fd120
  { 
    CONFIG_FSK,  0x01, 0x00, 0x08, 0x00, 0xe1, 0xe1, CONFIG_WHITE  }
  , // FSK_Rb125Fd125
  { 
    CONFIG_FSK,  0x00, 0x80, 0x10, 0x00, 0xe0, 0xe0, CONFIG_WHITE  }
  , // FSK_Rb250Fd250
  { 
    CONFIG_FSK,  0x02, 0x40, 0x03, 0x33, 0x42, 0x42, CONFIG_WHITE  }
  , // FSK_Rb55555Fd50 
};

