#include <LiquidCrystal_I2C.h>

const unsigned int TRIG_PIN = 13;
const unsigned int ECHO_PIN = 12;
const unsigned int BAUD_RATE = 9600;

LiquidCrystal_I2C lcd(0x27, 16, 4);

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.begin(BAUD_RATE);
  lcd.init();
  lcd.backlight();
}

void loop() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

 const unsigned long duration = pulseIn(ECHO_PIN, HIGH);
 unsigned long distance = (duration * 34) / 200;

 lcd.setCursor(0, 0);
 
 if(duration == 0){
    Serial.println("Warning: no pulse from sensor");
    lcd.print("Invalid value");
 } else {
    Serial.print("distance to nearest object: ");
    Serial.print(distance);
    Serial.println(" mm");

    lcd.print(distance);
    lcd.print(" mm         ");
 }
 
 delay(100);
}
