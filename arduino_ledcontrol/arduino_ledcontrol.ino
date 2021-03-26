const int ledPin =  4;
int ledState = LOW;
unsigned long previousMillis = 0;
long interval = 100;     // ************ removed const

char state;
boolean newData = false;
boolean ledBlink = false;
boolean runLed = false;

void setup()
{
   Serial.begin(9600);
   Serial.println("LED_READY");
   pinMode(ledPin, OUTPUT);
}

void loop()
{
   recvOneChar();
   if (newData == true)
   {
      if (state == 2) //run led
      {
         runLed = true;
         newData = false;
         ledState = HIGH;  // turn led on
         ledBlink = false;
      }
      else if (state == 3) //run blink
      {
         runLed = true;
         newData = false;
         ledBlink = true;
      }
      else if (state == 1) // turn off led
      {
         runLed = false;
         ledState = LOW;  // turn led off
         newData = false;
         ledBlink = false;
      }
   }
   unsigned long currentMillis = millis();
   if (currentMillis - previousMillis >= interval && runLed == true && ledBlink == true)
   {
      previousMillis = currentMillis;
      if (ledState == LOW)
      {
         ledState = HIGH;
         interval = 100;   // ********** sets on time
      }
      else
      {
         ledState = LOW;
         interval = 100; // ********** sets off time
      }
   } //end if
   digitalWrite(ledPin, ledState);
}

void recvOneChar()
{
   if (Serial.available() > 0)
   {
      state = Serial.parseInt();
      newData = true;
   }
}
