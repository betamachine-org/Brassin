//#include  "EventsManager.h"
#include <OneWire.h>

// Version evenementielle de l'exemple de la lib standard OneDrive
// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// https://github.com/milesburton/Arduino-Temperature-Control-Library


typedef enum { evxDsNext, evxDsRead }  tevxDs;



class evHandlerDS18x20 : private eventHandler_t, OneWire  {
  public:
    evHandlerDS18x20(const uint8_t aPinNumber);
    virtual void begin()  override;
    virtual void handle()  override;
    //    bool isOn()  {
    //      return ledOn;
    //    };

  private:
    uint8_t current;
    uint8_t error;
    uint8_t addr[8];
    uint8_t type_s;
    float  celsius;
};


evHandlerDS18x20::evHandlerDS18x20(const uint8_t aPinNumber) :
  OneWire(aPinNumber)
{
  //  pinMode(pinNumber, OUTPUT);
  //  setFrequence(frequence);
};

void evHandlerDS18x20::begin() {
  reset_search();
  //    delay(250);
  current = 0;
  Events.delayedPush(250, evDs18x20, evxDsNext); // next read in 250ms
};

void evHandlerDS18x20::handle() {
  if (Events.code != evDs18x20) return;
  if (Events.ext == evxDsNext) {

    if ( !search(addr)) {
      Serial.println("No more addresses.");
      //    Serial.println();
      begin();
      return;
    }
    Serial.print("ROM =");
    for ( uint8_t i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      error = 1;
      return;
    }
    Serial.println();
    current++;

    // the first ROM byte indicates which chip
    switch (addr[0]) {
      case 0x10:
        Serial.println("  Chip = DS18S20");  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
      case 0x22:
        Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
      default:
        Serial.println("Device is not a DS18x20 family device.");
        return;
    }
    reset();
    select(addr);
    write(0x44, 1);        // start conversion, with parasite power on at the end

    Events.delayedPush(1000, evDs18x20, evxDsRead); // get coverted value in 1000ms ( > 750ms)
  }
  if (Events.ext == evxDsNext) {

    uint8_t present = reset();
    select(addr);
    write(0xBE);         // Read Scratchpad
    byte data[12];
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for ( uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = read();
      Serial.print(data[i], HEX);
      Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s) {
      raw = raw << 3; // 9 bit resolution default
      if (data[7] == 0x10) {
        // "count remain" gives full 12 bit resolution
        raw = (raw & 0xFFF0) + 12 - data[6];
      }
    } else {
      byte cfg = (data[4] & 0x60);
      // at lower res, the low bits are undefined, so let's zero them
      if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
      else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
      else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
 //   fahrenheit = celsius * 1.8 + 32.0;
    Serial.print("  Temperature = ");
    Serial.print(celsius);
    Serial.println(" Celsius, ");
//    Serial.print(fahrenheit);
//    Serial.println(" Fahrenheit");

  }
  Events.delayedPush(0, evDs18x20, evxDsNext); // get coverted value in 1000ms ( > 750ms)
}