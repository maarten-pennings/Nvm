/*
  nvmdemo.ino - Demo for saving named strings (ssid, password, runs) in the ESP8266 EEPROM (non-volatile memory)
  Created by Maarten Pennings 2017 Oct 29
*/
#include "Nvm.h"


// This is a hack to force Serial.begin(115200) to be called before main().
// The declaration 'Nvm nvm(...)' runs the constructor, which might call Serial.printf(), so this hack makes them visible.
// The alternative is to have 'Nvm *nvm', and 'nvm=new Nvm(...)'.
class SerialBegin { public: SerialBegin() { Serial.begin(115200); } }; SerialBegin serialbegin;


// The layout and constructor
static NvmField fields[] = {
  {"ssid"    , "The ssid of the wifi AP"     , 32, 0},
  {"password", "The password of the wifi AP" , 32, 0},
  {"runs"    , "Number of runs"              ,  4, 0},
  {0         , 0                             ,  0, 0}, // Mandatory sentinel
};

Nvm nvm(fields);


// Test the Nvm
void setup() {
  char val[NVM_MAX_LENZ];
  Serial.printf("\n\nWelcome to NvmDemo\n\n");

  // Dump the NVM
  Serial.printf("Dump\n");
  nvm.dump();
  Serial.printf("\n");

  // Get all fields
  Serial.printf("Get\n");
  #define name "ssid"
  nvm.get(name,val);
  Serial.printf("  '%s' -> '%s'\n",name,val);
  #define name "password"
  nvm.get(name,val);
  Serial.printf("  '%s' -> '%s'\n",name,val);
  #define name "runs"
  nvm.get(name,val);
  Serial.printf("  '%s' -> '%s'\n",name,val);
  Serial.printf("\n");

  // Update runs
  nvm.get("runs",val);
  int runs= atoi(val) + 1;
  if( runs>9999 ) runs=0;
  Serial.printf("Runs\n");
  Serial.printf("  Update string 'runs' to %d\n",runs);
  itoa(runs,val,10); 
  nvm.put("runs",val);
  Serial.printf("  Power cycle (reset) and check 'runs'\n");

  // Other writes
  //nvm.put("ssid","MySSID");
  //nvm.put("password","The big secret");
}


void loop() {
  // Nothing to do
}

