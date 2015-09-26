#include "nokia5110.h"
#include "HX711.h"

// Define digital pins

#define VERSION "0.2"

// HX711.DOUT  - pin #A1
// HX711.PD_SCK - pin #A0
//tare to D11

#define buttonPin 11
#define ledPin  13
#define relayPin 9
#define jsSWpin A5  // 0 - 511 - 1023
#define jsXpin A4
#define jsYpin A3

#define weightDefault 12.0
#define weightMax 15.0
#define weightMin 3.0

HX711 scale(A1, A0);

void setup(void)
{
  LcdInitialise();
  LcdClear();
  gotoXY(0, 0);
  LcdString("Auto weight grinder");
  gotoXY(0, 3);
  LcdString("by cnwang");
  gotoXY(0, 4);
  LcdString("Ver. ");
  LcdString(VERSION);
  delay(1000);
  LcdClear();
  gotoXY(0, 0);
  LcdString("Setting.....");

  scale.set_scale(5268.91);
  scale.set_offset(27);
  scale.tare();
  pinMode(buttonPin, INPUT);
  delay(1000);

  pinMode(relayPin, OUTPUT);
  
  LcdString("done");
  delay(1000);


}

char buf[20];
int buttonState = 0;

float jsX;
float jsY;
float inc;
int jsSW;
float coffeeWeight = 12.0;
float currentWeight = 0.0;
bool isGo = false;
int stage = 1;

void(* resetFunc) (void) = 0;//declare reset function at address 0


void loop(void)
{

  if (stage == 1) {
    LcdClear();
    gotoXY(0, 0);

    LcdString ("Set weight");
    gotoXY(0, 5);
    LcdString("RESET     Go");


    while (stage == 1) {
      jsX = (analogRead(jsXpin) / 100) - 5; // -5 - 0 - 5
      jsY = (analogRead(jsYpin) / 100) - 5;
      jsSW = analogRead(jsSW);

      if (jsX < 0) {
        inc = (jsX == -5.0) ? -1.0 : -0.1;
      }

      if (jsX > 0) {
        inc = (jsX == 5.0) ? 1.0 : 0.1;
      }
      if (jsX == 0) inc = 0;



      coffeeWeight += inc;
      coffeeWeight = (coffeeWeight < weightMin) ? weightMin : coffeeWeight;
      coffeeWeight = (coffeeWeight > weightMax) ? weightMax : coffeeWeight;

      dtostrf(coffeeWeight, 3, 2, buf);
      gotoXY(0, 2);
      LcdString( "        g");
      gotoXY(0, 2);
      LcdString (buf);
      gotoXY(0, 3);
      //      dtostrf(jsY, 4, 0, buf);
      //
      //      LcdString (buf);

      if (jsY == 5.0) {
        gotoXY(0, 3);
        LcdString("Go ...");
        delay(1000);
        isGo = true;
        stage = 2;
      }
      if (jsY == -5.0) {
        delay(1000);
        resetFunc();

      }
      delay(100);
    }
  }

  if (stage == 2) {
    LcdClear();
    gotoXY(0, 0);
    LcdString ("Target Wt");
    gotoXY(0, 2);
    dtostrf(coffeeWeight, 4, 1, buf);
    LcdString(buf);
    LcdString (" g");
    gotoXY(0, 5);
    LcdString("Back   Grind");
    while (stage == 2) {
      jsY = (analogRead(jsYpin) / 100) - 5.0;
      jsSW = analogRead(jsSW);
      if (jsY == -5.0) {
        stage = 1;
        delay(1000);
      }
      if (jsY == 5.0) {
        stage = 3;
        delay(1000);
      }
    }
  }
  if (stage == 3) {

    LcdClear();
    gotoXY(0, 0);
    LcdString("GRINDING");
    gotoXY(0, 2);
    LcdString("Set: ");
    dtostrf(coffeeWeight, 4, 1, buf);

    LcdString(buf);

    scale.tare();
    gotoXY(0, 5);
    LcdString("ABORT       ");
    gotoXY(0, 3);
    LcdString("Act: ");

    //LcdString("weight :");

    while (stage == 3) {
      digitalWrite(relayPin, LOW);


      //   jsX = (analogRead(jsXpin)/100)-5; // -5 - 0 - 5
      jsY = (analogRead(jsYpin) / 100) - 5.0;
      jsSW = analogRead(jsSW);


      //buttonState = digitalRead(buttonPin);
      //      if (buttonState==HIGH) {
      //        digitalWrite(ledPin,HIGH);
      //        gotoXY(0,3);
      //        LcdString("Tare...");
      //        //delay(1000);
      //        digitalWrite(ledPin,LOW);
      //      }
      currentWeight = scale.get_units(5);
      if (currentWeight >= coffeeWeight) {
        digitalWrite(relayPin, HIGH);

        stage = 5;

        delay(1000);
        LcdClear();
      }
      dtostrf(currentWeight, 4, 1, buf);
      //gotoXY(5, 3);
      //LcdString("       ");
      gotoXY(5, 4);
      LcdString(buf);
      LcdString (" g");

      //delay(500);
      if (jsY == -5.0) {
        digitalWrite(relayPin, HIGH);

        gotoXY(0, 5);
        LcdString("ABORT ...   ");
        stage = 4;
        delay(1000);



      }

    }
  }

  if (stage == 4) { //abort
    gotoXY(0, 5);
    LcdString("CONT     FIN");
    while (stage == 4) {
      jsY = (analogRead(jsYpin) / 100) - 5;
      jsSW = analogRead(jsSW);
      if (jsY == -5.0) {
        gotoXY(0, 5);
        LcdString("          ");
        stage = 3;
      }
      if (jsY == 5.0) {
        stage = 5;
        LcdClear();
      }


    }
  }

  if (stage == 5) { //finished
    LcdClear();
    gotoXY(2, 0);
    LcdString("Finished");
    gotoXY(0, 1);
    dtostrf(currentWeight, 4, 1, buf);
    LcdString("Act: ");
    gotoXY(2, 2);
    LcdString(buf);
    gotoXY(0, 3);
    dtostrf(coffeeWeight, 4, 1, buf);
    LcdString("Set: ");
    gotoXY(2, 4);
    LcdString(buf);
    dtostrf(currentWeight / coffeeWeight * 100 - 100, 3, 1, buf);
    LcdString (" (");
    LcdString(buf);
    LcdString("%)");
    gotoXY(0, 5);
    delay(5000);
    LcdString("AGAIN");

    while (stage == 5) {
      jsY = (analogRead(jsYpin) / 100) - 5.0;
      jsSW = analogRead(jsSW);
      if (jsY == -5) {
        stage = 1;
      }
    }

  }

}
