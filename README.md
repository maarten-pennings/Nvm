#Nvm
A library for saving named strings into the EEPROM (non-volatile memory) of the ESP8266.

##Introduction
The ESP8266 has an integrated non volatile memory (NVM).
This library allows one to save and retrieve named strings into the NVM.

The following steps are required
- Define an nvm layout.
  This is a list of fields, and for each field the name, default value and length needs to be specified.
- Create and instance of the Nvm class.
  Pass the constructed layout.
- Save and retrieve data.
  Use the `put` and `get`  methods.
  
Note, if a string is retrieved before it is stored, the default is returned. 
This is based on a simple checksum, which is also stored in the EEPROM.

Memory footprint is as follows: for each field, the EEPROM stores
1 byte (for the actual length), len bytes (for data, len as secified in the layout), 1 byte for the termination zero, and 1 byte a checksum.


##Details
See [Nvm.h](src/Nvm.h) for details.
