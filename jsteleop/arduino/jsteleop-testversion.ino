
#include <Servo.h>
#include <LiquidCrystal.h>
#include <Nintendo.h>

Servo myservo;  // create servo object to control a servo
LiquidCrystal lcd(12, 13, 10, 11, 9, 8, 7);
CGamecubeController GamecubeController(4);
unsigned long gcPollTime = 0;

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin

volatile unsigned long pulseCount=0;
const unsigned long updateInterval=1000000; // in us
unsigned long startTime = 0;

void setup() {
  // myservo.attach(3);  // attaches the servo on pin 9 to the servo object
  Serial.begin(9600);
  // pinMode(2, INPUT);
  // attachInterrupt(0, N1, FALLING);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

uint8_t adc=0;
uint8_t firstbyte=0;
uint8_t dac = 0;

void loop() {
  if (startTime == 0) {
    startTime=micros() + updateInterval;
  }

  while(Serial.available()) {
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
    //print_gc_report(report, status);
    //delay(100);
    lcd.setCursor(0, 1);
    if (report.a)lcd.print("A"); else lcd.print(" ");
    if (report.b)lcd.print("B"); else lcd.print(" ");
    if (report.x)lcd.print("X"); else lcd.print(" ");
    if (report.y)lcd.print("Y"); else lcd.print(" ");
    
    //val = map(report.xAxis, 16, 235, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
    // myservo.write(val);                  // sets the servo position according to the scaled value
    //lcd.print(report.xAxis, DEC);
    //lcd.print(" ");
    //lcd.print(report.yAxis, DEC);

    // Rumble if button "A" was pressed
    if (report.a) {
      //GamecubeController.setRumble(true);
    }
    else {
      //GamecubeController.setRumble(false);
    }
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
    lcd.setCursor(0, 1);
    lcd.print(":(");
     gcPollTime = millis() + 300;
  }
  }

  if (micros() > startTime) {
    unsigned long uu = micros();
    lcd.setCursor(4, 0);
    lcd.print("     ");
    lcd.setCursor(4, 0);
    lcd.print(dac);

    startTime=uu + updateInterval;
  }

  
}

volatile unsigned long debounce = 0;
void N1() {
 if (micros() > debounce)
 {
    pulseCount++;
 }
    debounce = micros() + 2000;
}
