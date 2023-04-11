/*
works with simple esp32 boards and mcp2515 modules
*/
#include <SPI.h>
#include <mcp2515.h>
#include <WiFi.h>
const char* password = "";
const char* ssid="ANT BMS debugger";
IPAddress local_IP(192, 168, 1, 1); // Set your desired static IP address
IPAddress gateway(192, 168, 1, 1); // Set your gateway IP address
IPAddress subnet(255, 255, 255, 0); // Set your subnet mask
WiFiServer server(80);

// Define the CAN bus speed
MCP2515 mcp2515(10);

uint16_t SOH = 100; // SOH place holder
//unsigned char alarm[4] = {0, 0, 0, 0};
unsigned char warning[4] = {0, 0, 0, 0};
unsigned char mes[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char bmsname[8] = { '@', 'A', 'n', 't', 'B', 'm', 's', '@'};

//variables for AntBms

byte cmd[] = {0x5A, 0x5A, 0x00, 0x00, 0x01, 0x01};
byte startMark [] = { 0xAA, 0x55, 0xAA, 0xFF};
#define DATA_LENGTH           140
byte incomingByte[DATA_LENGTH] = {0};
int nbr_cell,soc, cell_av,cell_min,cell_max,cell_1,cell_2,cell_3,cell_4,cell_5,cell_6,cell_7,cell_8,
      cell_9,cell_10,cell_11,cell_12,cell_13,cell_14,cell_15,cell_16,cell_17,cell_18,cell_19,cell_20,
      cell_21,cell_22,cell_23,cell_24,t_bal, t_fet, t_1, t_2, t_3, t_4, puiss, HighTemp, LowTemp;
int cell[33];  //Maximum cell number is 32 for ANT BMS, the counting will start at 1 for simplicity    
float volt,amp, Ah_install, Ah_rest, Ah_Total,ChargeVsetpoint, DischVsetpoint ;
long int trame_ok, trame_nok;

void send_can(){
  struct can_frame canMsg;
  canMsg.can_id  = 0x351;
  canMsg.can_dlc = 8;
  canMsg.data[0] = lowByte(uint16_t(ChargeVsetpoint * nbr_cell)/100);
  canMsg.data[1] = highByte(uint16_t(ChargeVsetpoint * nbr_cell)/100);
  canMsg.data[2] = 0x00;//lowByte(chargecurrent);//  chargecurrent
  canMsg.data[3] = 0x00;//highByte(chargecurrent);//  chargecurrent
  canMsg.data[4] = 0x00;//lowByte(discurrent );//discurrent
  canMsg.data[5] = 0x00;//highByte(discurrent);//discurrent
  canMsg.data[6] = lowByte(uint16_t(DischVsetpoint * nbr_cell)/100);
  canMsg.data[7] = highByte(uint16_t(DischVsetpoint * nbr_cell)/100);
  mcp2515.sendMessage(&canMsg);

  delay(2);
  canMsg.can_id  = 0x355;
  canMsg.can_dlc = 8;
  canMsg.data[0] = lowByte(soc);//SOC
  canMsg.data[1] = highByte(soc);//SOC
  canMsg.data[2] = lowByte(SOH);//SOH
  canMsg.data[3] = highByte(SOH);//SOH
  canMsg.data[4] = lowByte(soc * 10);//SOC
  canMsg.data[5] = highByte(soc * 10);//SOC
  canMsg.data[6] = 0;
  canMsg.data[7] = 0;
  mcp2515.sendMessage(&canMsg);
//
  delay(2);
  canMsg.can_id  = 0x356;
  canMsg.can_dlc = 8;
  canMsg.data[0] = lowByte(uint16_t(volt * 10));
  canMsg.data[1] = highByte(uint16_t(volt * 10));
  canMsg.data[2] = lowByte(long(-amp ));
  canMsg.data[3] = highByte(long(-amp ));
  canMsg.data[4] = lowByte(int16_t(t_fet * 10));
  canMsg.data[5] = highByte(int16_t(t_fet * 10));
  canMsg.data[6] = 0;
  canMsg.data[7] = 0;
  mcp2515.sendMessage(&canMsg);
//
  delay(2);
  canMsg.can_id  = 0x35A;
  canMsg.can_dlc = 8;
  canMsg.data[0] = 0x00; //alarm[0];
  canMsg.data[1] = 0x00; //alarm[1];
  canMsg.data[2] = 0x00; //alarm[2];
  canMsg.data[3] = 0x00; //alarm[3];
  canMsg.data[4] = warning[0];
  canMsg.data[5] = warning[1];
  canMsg.data[6] = warning[2];
  canMsg.data[7] = warning[3];
  mcp2515.sendMessage(&canMsg);

  delay(2);
  canMsg.can_id  = 0x35E;
  canMsg.can_dlc = 8;
  canMsg.data[0] = bmsname[0];
  canMsg.data[1] = bmsname[1];
  canMsg.data[2] = bmsname[2];
  canMsg.data[3] = bmsname[3];
  canMsg.data[4] = bmsname[4];
  canMsg.data[5] = bmsname[5];
  canMsg.data[6] = bmsname[6];
  canMsg.data[7] = bmsname[7];
  mcp2515.sendMessage(&canMsg);

//  delay(2);
//  canMsg.can_id  = 0x370;
//  canMsg.can_dlc = 8;
//  canMsg.data[0] = bmsmanu[0];
//  canMsg.data[1] = bmsmanu[1];
//  canMsg.data[2] = bmsmanu[2];
//  canMsg.data[3] = bmsmanu[3];
//  canMsg.data[4] = bmsmanu[4];
//  canMsg.data[5] = bmsmanu[5];
//  canMsg.data[6] = bmsmanu[6];
//  canMsg.data[7] = bmsmanu[7];
//  mcp2515.sendMessage(&canMsg);

  delay(2);
  canMsg.can_id  = 0x373;
  canMsg.can_dlc = 8;
  canMsg.data[0] = lowByte(uint16_t(cell_min));
  canMsg.data[1] = highByte(uint16_t(cell_min));
  canMsg.data[2] = lowByte(uint16_t(cell_max));
  canMsg.data[3] = highByte(uint16_t(cell_max));
  canMsg.data[4] = lowByte(uint16_t(LowTemp + 273.15));
  canMsg.data[5] = highByte(uint16_t(LowTemp + 273.15));
  canMsg.data[6] = lowByte(uint16_t(HighTemp + 273.15));
  canMsg.data[7] = highByte(uint16_t(HighTemp + 273.15));
  mcp2515.sendMessage(&canMsg);

  delay(2);
  canMsg.can_id  = 0x372;
  canMsg.can_dlc = 8;
  canMsg.data[0] = lowByte(nbr_cell);//bms.getNumModules()
  canMsg.data[1] = highByte(nbr_cell);//bms.getNumModules()
  canMsg.data[2] = 0x00;
  canMsg.data[3] = 0x00;
  canMsg.data[4] = 0x00;
  canMsg.data[5] = 0x00;
  canMsg.data[6] = 0x00;
  canMsg.data[7] = 0x00;
  mcp2515.sendMessage(&canMsg);  

}
void read_bms(){
  Serial2.flush(); Serial2.write(cmd, sizeof(cmd)); 
  //while (Serial2.available()) {  Serial.print(Serial2.read()); };
  for (int i = 0; i < DATA_LENGTH; i++) { while (!Serial2.available()); incomingByte[i] = Serial2.read();}
  
if (incomingByte[0] == startMark [0] && incomingByte[1] == startMark [1] && incomingByte[2] == startMark [2] && incomingByte[3] == startMark [3]){
trame_ok += 1;  Serial.println("trame_ok :");
  extract_value();  for(int i = 0; i < DATA_LENGTH; i++) { Serial.print(incomingByte[i], HEX);}
  print_value();
  //VEcan();       // added to show loop() activity 
 }
 else{trame_nok += 1;}
}

void extract_value(){
   uint32_t tmp = ((((uint8_t)incomingByte[70])<< 24) + (((uint8_t)incomingByte[71])<< 16)+ (((uint8_t)incomingByte[72])<< 8)+ ((uint8_t)incomingByte[73]));
    if (tmp > 2147483648){amp= (-(2*2147483648)+tmp);}
    else{ amp = tmp;}
   uint32_t tmp2 = ((((uint8_t)incomingByte[111])<< 24) + (((uint8_t)incomingByte[112])<< 16)+ (((uint8_t)incomingByte[113])<< 8)+ ((uint8_t)incomingByte[114]));
    if (tmp2 > 2147483648){puiss= (-(2*2147483648)+tmp2);}
    else{ puiss = tmp2;}
   nbr_cell = (uint8_t)incomingByte[123]; 
   if(nbr_cell>32){
     Serial.println("Error reading cell number! Cell number set to 32!");
     nbr_cell = 32;
   }
   soc = (uint8_t)incomingByte[74]; 
   volt = ((((uint8_t)incomingByte[4])<< 8) + (uint8_t)incomingByte[5]); 
   cell_av = ((((uint8_t)incomingByte[121])<< 8) + (uint8_t)incomingByte[122]); 
   cell_min = ((((uint8_t)incomingByte[119])<< 8) + (uint8_t)incomingByte[120]); 
   cell_max = ((((uint8_t)incomingByte[116])<< 8) + (uint8_t)incomingByte[117]); 
   t_fet = ((((uint8_t)incomingByte[91])<< 8) + (uint8_t)incomingByte[92]); 
   t_bal = ((((uint8_t)incomingByte[93])<< 8) + (uint8_t)incomingByte[94]); 
   t_1 = ((((uint8_t)incomingByte[95])<< 8) + (uint8_t)incomingByte[96]); 
   t_2 = ((((uint8_t)incomingByte[97])<< 8) + (uint8_t)incomingByte[98]); 
   t_3 = ((((uint8_t)incomingByte[99])<< 8) + (uint8_t)incomingByte[100]); 
   t_4 = ((((uint8_t)incomingByte[101])<< 8) + (uint8_t)incomingByte[102]);
   for(int i=1;i<=nbr_cell;i++){
     cell[i] = ((((uint8_t)incomingByte[4+2*i])<< 8) + (uint8_t)incomingByte[5+2*i]);
   }
   Ah_install = ((((uint8_t)incomingByte[75])<< 24) + (((uint8_t)incomingByte[76])<< 16)+ (((uint8_t)incomingByte[77])<< 8)+ ((uint8_t)incomingByte[78]));
   Ah_rest = ((((uint8_t)incomingByte[79])<< 24) + (((uint8_t)incomingByte[80])<< 16)+ (((uint8_t)incomingByte[81])<< 8)+ ((uint8_t)incomingByte[82]));
   Ah_Total = ((((uint8_t)incomingByte[83])<< 24) + (((uint8_t)incomingByte[84])<< 16)+ (((uint8_t)incomingByte[85])<< 8)+ ((uint8_t)incomingByte[86]));
}

void   print_value(){
  long int milli_start = millis();
  Serial.print(" ");
  Serial.print((nbr_cell), DEC); Serial.println(" cells");
  Serial.print("soc = ");Serial.print((soc), DEC);Serial.println("%");  
  Serial.print("Voltage = ");Serial.print((volt/10), DEC);Serial.println(" V");
  Serial.print("Amperes = ");Serial.print((amp/10), DEC);Serial.println(" A");
  Serial.print("Puissance = ");Serial.print((puiss), DEC);Serial.println(" Wh");
  Serial.print("Cell_av = ");Serial.print((cell_av), DEC);Serial.println(" mV");
  Serial.print("Cell_min = ");Serial.print((cell_min), DEC);Serial.println(" mV");
  Serial.print("Cell_max = ");Serial.print((cell_max), DEC);Serial.println(" mV");
  Serial.print("T_MOSFET = ");Serial.print((t_fet), DEC);Serial.println(" °C");
  Serial.print("T_BALANCE = ");Serial.print((t_bal), DEC);Serial.println(" °C");
  Serial.print("T_1 = ");Serial.print((t_1), DEC);Serial.println(" °C");
  Serial.print("T_2 = ");Serial.print((t_2), DEC);Serial.println(" °C");
  Serial.print("T_3 = ");Serial.print((t_3), DEC);Serial.println(" °C");
  Serial.print("T_4 = ");Serial.print((t_4), DEC);Serial.println(" °C");
  
  for(int i=1;i<=nbr_cell;i++){
    Serial.print("Cell_");Serial.print(i);Serial.print(" = ");Serial.print((cell[i]), DEC);Serial.println(" mV");
  }
  
  Serial.print("Ah_install = ");Serial.print((Ah_install/1000000), DEC);Serial.println(" Ah");
  Serial.print("Ah_rest = ");Serial.print((Ah_rest/1000000), DEC);Serial.println(" Ah");
  Serial.print("Ah_Total = ");Serial.print((Ah_Total/1000), DEC);Serial.println(" Ah");
  Serial.print("trame ok/nok: ");Serial.print(trame_ok);Serial.print(" // ");Serial.print(trame_nok);Serial.print(" Ms: ");Serial.println((millis())-(milli_start));
  Serial.println("_____________________________________________");
  
}




void setup() {
  Serial.begin(115200); Serial2.begin(19200);//(19200,SERIAL_8N1,RXD2,TXD2)
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); //Check for your board, default is MCP_16MHZ
  mcp2515.setNormalMode();

}

void loop() {
  //Serial.println(Serial2.read());
  delay(200);
  read_bms();// read_bms to run every 0.20 seconds
  send_can();
}
