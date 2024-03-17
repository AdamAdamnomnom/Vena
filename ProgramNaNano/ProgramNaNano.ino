#include <SoftwareSerial.h>
#include "venaPilot2.h"
//to było nano działało
//nano 0,1
// SoftwareSerial a(0,1);
Vena vena;

String msg = "";
void setup() {
  vena.initialize();
  Serial.begin(9600);
  // a.begin(4800);
  vena.setMarker("M001",1);
}

void loop() {

  msg = vena.readMsg();
  Serial.println(msg);


  switch (msg.toInt()) {
    case 1:
      vena.out(2, HIGH);  //y3 wsuniety
      Serial.println("msg 1");
      break;
    case 2:
      vena.out(2, LOW);  // y3 wysów
      Serial.println("msg 2");
      break;
    case 3:
      vena.out(3, HIGH);  //Led swieci ciagle
      vena.setMarker("M001", 0);
      Serial.println("msg 3");
      break;
    case 4:
      vena.setMarker("M001", 1);  // led miga
      Serial.println("msg 4");
      break;
    case 5:
      vena.out(1, LOW);   // zabezpieczenie przed dwoma naraz
      vena.out(0, HIGH);  // y1 wysów
      Serial.println("msg 5");
      break;
    case 6:
      vena.out(0, LOW);   // zabezpieczenie przed dwoma naraz
      vena.out(1, HIGH);  //y2 wsów
      Serial.println("msg6");
      break;
    case 7:
      Serial.println("msg 7");
      vena.out(1, LOW);  //y2 koniec wsow
      break;
    case 8:
      vena.out(0, LOW);  // koniec wsów y1
      Serial.println("msg 8");
      break;
    case 9:
    
      vena.out(2, LOW);  
      vena.out(0, LOW); 
      vena.out(1, HIGH);  
      
      delay(500);
      
      vena.out(1, LOW);
       
      
      vena.out(3, HIGH);
      delay(100);
      vena.out(3, LOW);
      delay(100);
      vena.out(3, HIGH);
      delay(100);
      vena.out(3, LOW);
      delay(100);
      vena.out(3, HIGH);
      delay(100);
      vena.out(3, LOW);
      delay(100);
      vena.setMarker("M001",1);
      break;

  }

  if (vena.getMarker("M001") == 1) {
    vena.out(3, vena.generateImpulse("G1", 300, 300, true));
  }
}
