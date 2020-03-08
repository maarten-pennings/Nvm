/*
  Nvm.cpp - Library for saving named strings in EEPROM (non-volatile memory)
  Created by Maarten Pennings 2017 April 17, Updated comments 2017 Oct 29, Allows len==0 2020 March 07
*/


#include <Arduino.h>
#include <EEPROM.h>
#include "Nvm.h"


// The initial vector for the checksum (of the fields). 
#define NVM_SUMINIT 0xAA // This value ensures that an all-0 or all-1 EEPROM does not have a matching checksum


// Constructor, passing the NVM layout.
// Prints errors in layout to Serial (so have that open).
Nvm::Nvm(NvmField*fields) {
  if( NVM_MAX_LENZ-1>255 ) Serial.printf("ERROR: NVM_MAX_LENZ (%d) shall be max 256\n", NVM_MAX_LENZ);
  // Store the layout
  _fields = fields;
  // Count the number of fields and run some checks.
  _fieldcount = 0;
  NvmField * f = fields;
  while( f->name!=0 ) {
    if( strlen(f->name)>NVM_MAX_LENZ-1 ) Serial.printf("ERROR: Nvm field '%s' has a name that exceeds len %d\n", f->name, NVM_MAX_LENZ-1);
    if( f->len         >NVM_MAX_LENZ-1 ) Serial.printf("ERROR: Nvm field '%s' has len %d (but %d is max)\n", f->name, f->len, NVM_MAX_LENZ-1);
    if( strlen(f->dft) >f->len         ) Serial.printf("ERROR: Nvm field '%s' has default '%s' with len %d which exceeds len %d\n", f->name, f->dft, strlen(f->dft), f->len);
    f++;
    _fieldcount++;
  }
  // Check sentinel for consistency
  if( f->name  !=0 ) Serial.printf("ERROR: Nvm sentinel field has non-zero name '%s'\n", f->name);
  if( f->dft   !=0 ) Serial.printf("ERROR: Nvm sentinel field has non-zero default '%s'\n", f->dft);
  if( f->len   !=0 ) Serial.printf("ERROR: Nvm sentinel field has non-zero length %d\n", f->len);
  if( f->extra !=0 ) Serial.printf("ERROR: Nvm sentinel field has non-zero extra %s\n", f->extra);
  // Setup the array that records the start positions of the fields
  _fieldstarts = new int[_fieldcount+1]; // Same length as _fields (which has a sentinel)
  _fieldstarts[0] = 0; // Offset of first field.
  for(int ix=0; ix<_fieldcount; ix++ ) {
    //Serial.printf("INFO: %s @ 0x%04x # %d\n",_fields[ix].name,_fieldstarts[ix],_fields[ix].len);
    _fieldstarts[ix+1] = _fieldstarts[ix] + 1+ fields[ix].len + 1 + 1; // Add 1 byte for len, len bytes for content, 1 for terminating zero, and 1 for checksum
  }
  // Connect to the EEPROM. Note storage size used is _fieldstarts[_fieldcount];
  EEPROM.begin(_fieldstarts[_fieldcount]);
  //Serial.printf("INFO: EEPROM size %d\n",_fieldstarts[_fieldcount]);
}


// Destructor
Nvm::~Nvm(void) {
  EEPROM.end();
  delete[] _fieldstarts; 
}


// Returns the number of fields.
int Nvm::count(void) {
  return _fieldcount;
}


// Returns field definition for field with index ix (or NULL if ix out of range).
NvmField* Nvm::field(int ix) {
  if( ix<0 || ix>=_fieldcount ) return 0;
  return &_fields[ix];
}


// Dumps the nvm (used part of EEPROM) to serial port.
void Nvm::dump(char * prefix) {
  if( prefix==0 ) prefix=(char*)"";
  int *start = _fieldstarts;
  int firstfree = _fieldstarts[_fieldcount]; 
  int address = 0;
  int  ix = 0;
  const char * name = 0;
  while( address<firstfree ) {
    Serial.printf("%s%04x ",prefix,address);
    int x=0; 
    while( x<16 && address<firstfree ) {
      char sep = ' ';
      if( *start==address ) { sep='|'; start++; name=_fields[ix].name; ix++; };
      Serial.printf("%c%02x", sep,EEPROM.read(address) );
      x++;
      address++;
    }
    while( x<16  ) {
      char sep = ' ';
      if( *start==address ) { sep='|'; };
      Serial.printf("%c--",sep);
      x++;
      address++;
    }
    if( name!=0 ) { Serial.printf(" %s",name); name=0; }
    Serial.printf("\n");
  }
}


// Looks up the field with name 'name' and return its index (returns -1 if name is not found).
int Nvm::find(const char * name) {
  for(int ix=0; ix<_fieldcount; ix++ ) {
    if( strcmp(_fields[ix].name,name)==0 ) return ix;
  }
  return -1;
}


// Reads field 'name' from EEPROM and stores that in 'val'. Note 'val' must be allocated by user (size NVM_MAX_LENZ). 
void Nvm::get(const char * name, char * val) {
  get( find(name), val );
}


// Reads field with index ix  from EEPROM and stores that in 'val'. Note 'val' must be allocated by user (size NVM_MAX_LENZ). 
// If EEPROM has invalid len, missing \0, or mismatching checksum, field(ix).dft is returned instead.
void Nvm::get(int ix,char*val) {
  // Check index
  if( ix<0 || ix>=_fieldcount ) {
    Serial.printf("ERROR: index (%d) out of range (was the passed name valid?)\n",ix);
    *val = '\0'; 
    return;
  }
  int address = _fieldstarts[ix];
  // Get string length
  uint8_t sum = NVM_SUMINIT;
  unsigned len = EEPROM.read(address++);
  if( len>_fields[ix].len ) { strcpy(val,_fields[ix].dft); return; }
  sum ^= len; // len is part of checksum
  // Get chars
  uint8_t* p= (uint8_t*)val;
  for(unsigned i=0; i<=len; i++) { // Read also terminating zero (<= instead of <)
    *p = EEPROM.read(address++);
    sum ^= *p;
    p++;
  }
  if( val[len]!='\0' ) { strcpy(val,_fields[ix].dft); return; }
  // Get and compare checksum
  uint8_t sum2 = EEPROM.read(address++);
  if( sum!=sum2 )  { strcpy(val,_fields[ix].dft); return; }
}


// Saves 'val' to field 'name' in EEPROM.
void Nvm::put(const char * name,const char*val) {
  put( find(name), val );
}


// Saves 'val' to EEPROM, in field with index ix (aborts if not 0<=ix<count() ).
// If needed, 'val' is truncated to field(ix).len.
// Checksum is also written.
void Nvm::put(int ix,const char*val) {
  // Check index
  if( ix<0 || ix>=_fieldcount ) {
    Serial.printf("ERROR: index (%d) out of range (was the passed name valid?)\n",ix);
    return;
  }
  int address = _fieldstarts[ix];
  //Serial.printf("Nvm('%s')=EEPROM(%d) <- ",name,address);
  // Determine string length
  uint8_t sum = NVM_SUMINIT;
  unsigned len = strlen(val);
  if( len>_fields[ix].len ) len=_fields[ix].len; // truncate
  EEPROM.write(address++, (uint8_t)len);
  sum ^= len; // len is part of checksum
  // Write all chars
  uint8_t * p= (uint8_t*)val;
  for(unsigned i=0; i<len; i++) {
    EEPROM.write(address++,*p);
    sum ^= *p;
    p++;
  }
  // Write terminating zero (at 'len')
  EEPROM.write(address++,'\0');
  sum ^= '\0';
  // Write checksum
  EEPROM.write(address++,sum);
  // Commit
  EEPROM.commit();
}


