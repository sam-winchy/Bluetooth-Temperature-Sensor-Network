#include <AltSoftSerial.h> //AltSoftSerial BTSerial;

AltSoftSerial BTSerial(8, 9); // Pins: RX=8, TX=9
int pos = 0;
char c = ' ';
char reply[30];
char Mino[] = {"0000"};
char MinoResps;
String HexTemp, MinoTemp, MinoRespStr;
String MinoPre = "AT+MINO0x00";
String MinoRespsPre = "OK+Set:00";
String response = ""; // ascii string from HM10/11
// AT commands and responses for HM10/11
char* ATcmds[] = {"AT", "AT+RENEW", "AT+RESET", "AT", "AT+POWE0",   "AT+MARJ0x1234",  "AT+MINO0xFA01", "AT+IBE074278BDA", "AT+ADVIF", "AT+NAMEsensor1",  "AT+ADTY3", "AT+IBEA1", "AT+DELO2", "AT+RESET"};    //7000ms advertising interval
char* ATresps[] = {"OK", "OK+RENEW", "OK+RESET", "OK", "OK+Set:0", "OK+Set:0x1234",    "OK+Set:0xFA01", "OK+Set:0x74278BDA",  "OK+Set:F", "OK+Set:sensor1", "OK+Set:3",  "OK+Set:1", "OK+DELO2", "OK+RESET"}; 

void ReadScan(unsigned long duration);
String decToHex(unsigned int decValue, byte desiredStringLength);
int SendCmd();
int SendData();

/*////////////////////////////////////////////////////////////////////////////////////////////
Name:  Sam Winchcombe 
Date:  23/04/2019
Notes: Initial AT setting are set. Once set the HM-10 modules begins broadcasting.
       Takes voltage reading from thermostat and converts to HEX. Then converts HEX values which
       areletters (E.G A,B,C etc...) to Capital versions of themselvesas the HM-10 expects ASCII
       charactersand not HEX in reality. The first part on the AT command for the temperature
       string is fixedand the vairiable temperature bits are affixed to the end. This is thenn
       converted to a stringand set in the AT commands. 
/////////////////////////////////////////////////////////////////////////////////////////////*/

//---------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  Serial.print("Sketch:   ");   Serial.println(__FILE__);
  Serial.print("Uploaded: ");   Serial.println(__DATE__);
  Serial.println(" ");
  pinMode (A0, INPUT);
  BTSerial.begin(9600);
  Serial.println("BTserial started at 9600");
  Serial.println(" ");
  ReadScan(3500); // This will effectively flush the BTSerial buffer
}
//---------------------------------------------------------------------------
void loop() {
  int VoltageValue;
  if (SendCmd()) {
    Serial.println("Error sending AT commands");
    ReadScan(3500); // This will effectively flush the sSerial buffer
    delay(1000);
  }
  else {
    while (1) {
      VoltageValue = analogRead(A0); //Voltage reading from thermistor
      Serial.print("ADC reading:");
      Serial.println(VoltageValue);
      HexTemp = decToHex(VoltageValue, 2); //String(VoltageValue, HEX);
      HexTemp = CaseChk(HexTemp);
      delay(500);
      MinoTemp = MinoPre + HexTemp; // Conmbine prefix to the variable Mino temperature bits 
      MinoRespStr = MinoRespsPre + HexTemp; //Create expected response string to AT Command
      MinoRespStr.toCharArray(MinoResps, 14);
      MinoTemp.toCharArray(Mino, 14);
      SendData();
      delay(5000);
      // Currently won't take mV readings over 255 as that will require 3 Hex digits.
    }
  }
}
//---------------------------------------------------------------------------
// Read the response string from the HM10/11. stays for duration (seconds) to get a complete string
void ReadScan(unsigned long duration) {
  char chr;
  response = "";
  // for timer
  unsigned long starttick = 0;
  unsigned long curtick = 0;
  //Stays here for duration of time reading the software serial
  starttick = millis();
  while ( ( (curtick = millis() ) - starttick) < duration ) {
    while (BTSerial.available() != 0) {
      chr = BTSerial.read();
      response.concat(chr);
    }
  }
}
//---------------------------------------------------------------------------
// AT command responses are compared to expected responses to test for errors.
int SendCmd() {
  int x, ecount;
  ecount = 0;
  for (x = 0; x < 13; x++) {
    BTSerial.write(ATcmds[x]); //sends command to Bluetooth module
    Serial.print(ATcmds[x]);
    ReadScan(1500);
    Serial.print("->");
    Serial.println(response);
    //checks repsonse against expected response
    if (response != ATresps[x]) {
      Serial.println(ecount); //Debugging line
      ecount++;
      Serial.println(ecount); //Debugging line
    }
  }
  // send ATDELO1 which does not have a response
  BTSerial.write("ATDELO1");
  delay(500);
  return (ecount);
}
//------------------------------------------------------------
String decToHex(unsigned int decValue, byte desiredStringLength) {
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  return hexString;
}
//----------------------------------------------------------------------
int SendData() {
  int x, ecount;
  ecount = 0;
  BTSerial.write(Mino);
  Serial.print(Mino);
  ReadScan(1000);
  Serial.print("->");
  Serial.println(response);
  if (response != MinoResps) {
    //Debugging line
    Serial.println(ecount);
    ecount++;
    //Debugging line
    Serial.println(ecount);
  }
  delay(500);
  return (ecount);
}
//---------------------------------------------------
String CaseChk( String Hex) {
  for (int x = 0; x < 2; x++) {
    if (Hex[x] == 'a') {
      Hex [x] = 'A';
    }
    else if (Hex[x] == 'b') {
      Hex [x] = 'B';
    }
    else if (Hex[x] == 'c') {
      Hex [x] = 'C';
    }
    else if (Hex[x] == 'd') {
      Hex [x] = 'D';
    }
    else if (Hex[x] == 'e') {
      Hex [x] = 'E';
    }
    else if (Hex[x] == 'f') {
      Hex [x] = 'F';
    }
  }
  return (Hex);
}
