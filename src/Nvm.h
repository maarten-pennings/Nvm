/*
  Nvm.h - Library for saving named strings in EEPROM (non-volatile memory)
  Created by Maarten Pennings 2017 April 17, Updated comments 2017 Oct 29
*/
#ifndef __NVM_H_
#define __NVM_H_


/*
  Define an nvm layout (a list of field names, field (default) values and field length) as follows:
    static NvmField fields[] = {
      {"ssid"    , "The ssid of the wifi AP"     , 32, 0},
      {"password", "The password of the wifi AP" , 32, 0},
      {0         , 0                             ,  0, 0}, // Mandatory sentinel
    };
  Then call the constructor
    Nvm * nvm = new Nvm(fields);
  or have it static
    Nvm nvm(fields);
  Note: Methods in this class print error conditions to Serial. Most notably, 
  the constructor does a consistency check on the 'fields' and prints
  any errors to Serial. This makes a static constructor less suitable, since 
  the Serial port is not yet open.

  To store a value
    nvm->put("ssid","Something");
  To retrieve a value    
    char val[NVM_MAX_LENZ];
    #define name "ssid"
    nvm->get(name,val);
    Serial.printf("'%s' -> '%s'\n",name,val);

  To inspect (hex dump) the EEPROM, call
    nvm->dump();
    
  Note, if a string is retrieved before it is stored, the default is returned
  (dft as specified in the layout). This is based on a simple checksum, which
  is also stored in the EEPROM.

  All 'name' and 'val' strings have a maximum len of NVM_MAX_LENZ-1
  (so buffers of len NVM_MAX_LENZ can store them with the terminating zero).
  If a string is 'put', it is truncated to its 'len'.
  
  Memory footprint is as follows: for each field, the EEPROM stores
  1 byte (for len), len (for data), 1 byte for the termination zero, and 1 byte a checksum.
*/


// Max buffer size (strlen+1) for values
#define NVM_MAX_LENZ 65 


// An array of NvmField's defines the nvm layout; one NvmField defines a single field; it records:
// the name of the field, its default value, and the max length of the value (i.e. the reserved space in the nvm)
// A field has 'extra' which is not used by the Nvm module).
class NvmField { 
  public:
    NvmField( const char * _name, const char * _dft, unsigned int _len, const char* _extra ): name(_name),dft(_dft),len(_len),extra(_extra) {}; 
    const char *       name; // The name of the field.
    const char *       dft;  // Default value of the field (when not yet stored).
    const unsigned int len;  // Max strlen of value stored for name .
    const char *       extra;// Extra data (not used by Nvm module)
};


// Wrapper class around the EEPROM to put and get strings into the EEPROM by name.
// For each string the length, terminating zero and a checksum is also stored.
// If a string is retrieved whose checksum is incorrect (e.g. when it was not 'put') the default value is returned.
class Nvm {
  public: // main API functions
    Nvm(NvmField*fields);                           // Constructor, passing the NVM layout.
    ~Nvm(void);
    void     get(const char * name,char*val);       // Reads field 'name' from EEPROM and stores that in 'val'. Note 'val' must be allocated by user (size NVM_MAX_LENZ). 
    void     put(const char * name,const char*val); // Saves 'val' to field 'name' in EEPROM.
  public: // helpers function
    void     dump(char * prefix=(char*)"  ");       // Dumps the nvm (used part of EEPROM) to serial port (each line is prefixed with 'prefix)'.
    int      count(void);                           // Returns the number of fields.
    NvmField*field(int ix);                         // Returns field definition for field with index ix (or NULL if ix out of range).
    int      find(const char * name);               // Looks up the field with name 'name' and return its index (returns -1 if name is not found).
    void     get(int ix,char*val);                  // Reads field with index ix  from EEPROM and stores that in 'val'. Note 'val' must be allocated by user (size NVM_MAX_LENZ). 
    void     put(int ix,const char*val);            // Saves 'val' to EEPROM, in field with index ix.
  private: // internal functions
    NvmField*_fields;                               // The layout (the list of field definitions).
    int*     _fieldstarts;                          // For each field, stores the offset into the EEPROM.
    int      _fieldcount;                           // Number of fields (i.e. the length of the _fields array, excluding its terminator)
};


#endif

