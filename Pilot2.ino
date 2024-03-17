#include <SoftwareSerial.h>
#include "venaPilot2.h"

Vena vena;

String msg = "";
void setup() {
  vena.initialize();
  vena.setMarker("M001",1);
}

void loop() {

  msg = vena.readMsg();


  switch (msg.toInt()) {
    case 1:
      vena.out(2, HIGH);  //y3 wsuniety
      break;
    case 2:
      vena.out(2, LOW);  // y3 wysów
      break;
    case 3:
      vena.out(3, HIGH);  //Led swieci ciagle
      vena.setMarker("M001", 0);
      break;
    case 4:
      vena.setMarker("M001", 1);  // led miga
      break;
    case 5:
      vena.out(1, LOW);   // zabezpieczenie przed dwoma naraz
      vena.out(0, HIGH);  // y1 wysów
      break;
    case 6:
      vena.out(0, LOW);   // zabezpieczenie przed dwoma naraz
      vena.out(1, HIGH);  //y2 wsów
      break;
    case 7:
      vena.out(1, LOW);  //y2 koniec wsow
      break;
    case 8:
      vena.out(0, LOW);  // koniec wsów y1   
      break;
    case 9:
      vena.out(2, LOW);  
      vena.out(0, LOW); 
      vena.out(1, HIGH, 200);  
      vena.out(3, HIGH,100);
      vena.out(3, HIGH,100);
      vena.out(3, HIGH,100);
      vena.setMarker("M001",1);
      break;

  }

  if (vena.getMarker("M001") == 1) {
    vena.out(3, vena.generateImpulse("G1", 300, 300, true));
  }
}
