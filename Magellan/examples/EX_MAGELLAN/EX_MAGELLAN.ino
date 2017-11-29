#include "Magellan.h"
Magellan magel;
char auth[]="your key"; //Token Key you can get from magellan platform

String payload;

void setup() {
  Serial.begin(9600);
  magel.begin(auth);           //init Magellan LIB
}

void loop() {
  String Temp=String(random(0,100));
  String EC=String(random(0,100));

  payload="{\"Temp\":"+Temp+",\"EC\":"+EC+"}";       //please payload with json format

  magel.post(payload);                            //post data payload to Magellan platform
  
}