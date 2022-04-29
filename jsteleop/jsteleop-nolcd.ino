#include <Nintendo.h>

CGamecubeController GamecubeController(4);
unsigned long gcPollTime = 0;

void setup() {
  Serial.begin(9600);
}

uint8_t adc=0;
uint8_t firstbyte=0;
uint8_t dac = 0;

void loop() {
  while(Serial.available()) 
  {
    uint8_t byte = Serial.read();
    if(((firstbyte & 0x80) == 0x80) && ((byte & 0x80) == 0)) {
      // got a packet
      uint16_t payload = (firstbyte << 7) | byte;
      uint8_t address = (firstbyte >> 4) & 7;
      dac = payload & 0xff;
      uint8_t dir = (payload & 0x100) == 0x100;
      uint8_t out = (payload & 0x200) == 0x200;
      if(address < 6) {
          //analogWrite(dacpinmap[address], dac);
          //digitalWrite(pinmap[address], out);
          //pinMode(pinmap[address], dir);
      }
    }
    firstbyte = byte;
  }

  if (millis() > gcPollTime)
  {
    if (GamecubeController.read())
    {
      // Print Controller information
      auto status = GamecubeController.getStatus();
      auto report = GamecubeController.getReport();
      
      adc = 0;
      uint16_t v;
      
      v = report.xAxis | (adc << 11);
      Serial.write((v >> 7) | 0x80);
      Serial.write(v & 0x7f);
      adc = (adc + 1);
  
      v = report.yAxis | (adc << 11);
      Serial.write((v >> 7) | 0x80);
      Serial.write(v & 0x7f);
      adc = (adc + 1);
  
      v = report.cyAxis | (adc << 11);
      Serial.write((v >> 7) | 0x80);
      Serial.write(v & 0x7f);
      adc = (adc + 1);
  
      v = (report.a << 0)
        | (report.b << 1)
        | (report.x << 2)
        | (report.y << 3)
        | (report.start << 4)
        | (report.z << 5)
        | (report.dleft << 6)
        | (report.dright << 7)
        | (report.ddown << 8)
        | (report.dup << 9)
        | (adc << 11);
      Serial.write((v >> 7) | 0x80);
      Serial.write(v & 0x7f);
      adc = (adc + 1);
          
      gcPollTime = millis() + 200;
    }
    else
    {
      // TODO: Indicate error (error led)
       gcPollTime = millis() + 500;
    }
  }  
}
