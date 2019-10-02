#include <AltSoftSerial.h> //AltSoftSerial BTSerial;

AltSoftSerial BTSerial(8, 9); // Pins: RX=8, TX=9
// AT commands and responses for HM10/11
char* ATcmds[] = {"AT", "AT+RENEW", "AT+IMME1", "AT+ROLE1"};
char* ATresps[] = {"OK", "OK+RENEW", "OK+Set:1", "OK+Set:1"};
String response = ""; // ascii string from HM10/11
int SearchScan(String UUID);
int SendCmd(void);
void ReadScan(unsigned long duration);

/*////////////////////////////////////////////////////////////////////////////////////////////
Name: Sam Winchcombe 
Date: April 2019
Notes:Initial AT setting are set. Scans for advertisers with the expected UUID.
      If they are found then the bits which contain the temperature values are extracted and
      convert from HEX to decimal. The values are then displayed. 
/////////////////////////////////////////////////////////////////////////////////////////////*/


void setup() {
  Serial.begin(9600); // Open serial communications
  BTSerial.begin(9600); //Open Bluetooth SoftSerial communication
  ReadScan(3500); // This will effectively flush the sSerial buffer
  Serial.println("Master Module (V3.2)");
}
void loop() {
  // fixed UUID string for temperature sensors
  String UUIDs [] = {"74278BDAB64445208F0C720EAF059935", "12345678B64445208F0C720EAF059935"}; // Enter UUID's of Slaves in here
  char* SenNam [] = {"Sensor1", "Sensor2"};
  int SensNum;
  if (SendCmd()) {
    Serial.println("Error sending AT commands");
    ReadScan(3500); // This will effectively flush the sSerial buffer
    delay(1000);
  }
  else {
    while (1) {
      BTSerial.write("AT+DISI?"); //scans for advertisers
      ReadScan(4000); // 5 seconds works well
      // uncomment the next line to print the entire response screen
      //Serial.println(response);
      if (BTSerial.overflow()) {
        Serial.println("** SoftwareSerial overflow! **");
      }
      // SearchScan() will do all the work and return either
      // 0 - no sensor in the scan.
      // 1 - sensor found.
      // Look for sensor with UUIB Declared.
      for (SensNum = 0; SensNum < 2; SensNum++) {
        switch (SearchScan(UUIDs [SensNum], SenNam [SensNum] )) {
          case 0:
            Serial.print(SenNam [SensNum]);
            Serial.println(" not found");
            //Serial.println("");
            break;
          case 1:
            Serial.print(SenNam [SensNum]);
            Serial.print(" found");
            Serial.println(":");
            break;
          default:
            Serial.println();
            Serial.println("!!!unknown Sensor!!!");
            break;
        }
      }
    }
  }
}
//-------------------------------------------
// Send the AT commands to set up the HM10/11
// must return the correct responses
// ok if 0 is returned, error otherwise
int SendCmd(void) {
  int x, ecount;
  ecount = 0;
  for (x = 0; x < 4; x++) {
    BTSerial.write(ATcmds[x]); //sends command to Bluetooth module
    Serial.print(ATcmds[x]);
    ReadScan(1000);
    Serial.print("->");
    Serial.println(response);
    //checks repsonse against expected response
    if (response != ATresps[x]) {
      ecount++;
    }
  }
  BTSerial.write("ATDELO1"); // send ATDELO1 which does not have a response
  delay(500);
  return (ecount);
}
//-------------------------------------------
// Read the response string from the HM10/11 stays for duration (seconds) to get a complete string
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
//-------------------------------------------
int SearchScan(String UUID, char* Name) {
  int UUIDIndex = 0;
  int supdate = 0;
  char SsenorTEM[] = {"0000"};
  char SsensorID[] = {"0000"};
  float TempC, Sensor1Temp, Temp;
  String sTemp, sID;
  unsigned long sensorTemp, sensorID;
  int sliceStart, sliceEnd;
  String sliceP2;
  if ((UUIDIndex = response.indexOf(UUID)) == -1) {
    return (supdate);
  }
  // a sensor UUID was found and uIndex points to
  // the begining of the string - search for the next ':'
  //marks start of Data field
  if ((sliceStart = response.indexOf(':', UUIDIndex)) == -1) { 
    return (supdate);     // if no colon found assume error and dump the string
  }
  // get the next colon
  //marks start of temperature data
  if ((sliceEnd = response.indexOf(':', sliceStart + 1)) == -1) { 
    // if no colon found assume error and dump the string
    return (supdate);
  }
  // get the substring bounded by the two colons
  sliceP2 = response.substring(sliceStart + 1, sliceEnd);
  if (strlen(sliceP2 != 10)) {
    // P2 needs to be exactly 10 characters, if not, assume error and dump the string
    return (supdate);
  }
    for (int i = 0; i < 7; i++) //seperates the relevant characters into a substrings
  {
    Serial.print(Name[i]);
  }
  Serial.println("");
  sID = sliceP2.substring(0, 4);
  sTemp = sliceP2.substring(6, 8);
  sID.toCharArray(SsensorID, 5);  // copy them to their constant char analogs
  sTemp.toCharArray(SsenorTEM, 4);
  Serial.print("Hex value:");
  Serial.println(sTemp);
  sensorTemp = strtoul(SsenorTEM, NULL, 16);   // get hex ascii string to unsigned longs
  Temp = sensorTemp * 0.004882814;
  TempC = ((Temp - 0.5) * 100.0);
  Serial.print("Temperature:  ");
  Serial.print(TempC);
  Serial.println(" Degrees Celsius");
  return (1);
}
