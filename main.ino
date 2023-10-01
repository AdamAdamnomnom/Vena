#include "Arduino.h"
#include "nzStero.h"


nzStero myStero;


void setup() {
  Serial.begin(9600);
  myStero.initialize();  // Inicjalizacja biblioteki
  Serial.println("inicjalizacja");
  myStero.setCounter("C001", 10);
}

void loop() {

  if (myStero.check(0)) {
    myStero.addToCounter("C001", 1);
    Serial.println(myStero.getCounter("C001"));
    if (myStero.checkCounter("C001")) {
      Serial.println("gratulacje doliczyłeś do 10");
      while (true) {
      }
    }
  }
}
// nie dziłą gdy jest 0 z porzodu countera 
