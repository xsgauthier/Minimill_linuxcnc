#include <Nintendo.h>
#include <LiquidCrystal.h>

CGamecubeController GamecubeController(4);
LiquidCrystal lcd(12, 13, 10, 11, 9, 8, 7);
unsigned long gcPollTime = 0;
unsigned long lcdUpdateTime = 0;

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
}

uint8_t adc=0;
uint8_t firstbyte=0;
uint8_t dac = 0;

uint8_t pinmap[2] = {2, 6};
uint8_t dacpinmap[2] = {3, 5};

uint8_t lcd_val[2] = {0, 0};

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
      if(address < 2) {
          analogWrite(dacpinmap[address], dac);
          digitalWrite(pinmap[address], out);
          pinMode(pinmap[address], dir);
      } else if (address == 2) {
        // digitalWrite(pinmap[address], out);
        // pinMode(pinmap[address], dir);
        lcd_val[0] = dac;
      } else if (address == 3) {
      }
    }
    firstbyte = byte;
  }

  if (millis() > lcdUpdateTime) {
    dac = lcd_val[0];
    uint8_t sRow = 0;
    lcd.setCursor(sRow/* row */, 0/* col */);
    lcd.print("HD:   C");
    if (dac >= 100)
      lcd.setCursor(sRow+3/* row */, 0/* col */);
    else if (dac >= 10)
      lcd.setCursor(sRow+4/* row */, 0/* col */);
    else
      lcd.setCursor(sRow+5/* row */, 0/* col */);
    lcd.print(dac, DEC);
    
    lcdUpdateTime = millis() + 500;
  }

  if (millis() > gcPollTime)
  {
    uint16_t v;
    if (GamecubeController.read())
    {
      // Print Controller information
      auto status = GamecubeController.getStatus();
      auto report = GamecubeController.getReport();
      
      adc = 0;
      
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
       adc = 4;
    }

    v = analogRead(0) | (adc << 11);
    if(digitalRead(pinmap[0])) v |= (1<<10);
    Serial.write((v >> 7) | 0x80);
    Serial.write(v & 0x7f);
    adc = (adc + 1);

    v = analogRead(1) | (adc << 11);
    if(digitalRead(pinmap[1])) v |= (1<<10);
    Serial.write((v >> 7) | 0x80);
    Serial.write(v & 0x7f);
    adc = (adc + 1);
    
  }  
}
