#include "Arduino.h"
#include "nzStero.h"


nzStero myStero;

void setup() {
    Serial.begin(9600);
    //myStero.addExtraVar(int M4, int 0)
    myStero.initialize(); // Inicjalizacja biblioteki
     myStero.addExtraVar("M1", 1);
     myStero.addExtraVar("M2", 0);
     myStero.addExtraVar("M3", 1);
     myStero.addExtraVar("M4", 0);
     myStero.addExtraVar("M5", 1);
     myStero.addExtraVar("M6", 0);
     Serial.println("incijalizeded udanet");
    // myStero.addExtraVar("M3", 1);
}

void loop() {
    // Odczyt stanów wejść
    bool input0 = myStero.check(0);
    bool input1 = myStero.check(1);
    bool input2 = myStero.check(2);
    bool input3 = myStero.check(3);
    //myStero.setExtraVar(int M4, 1 )
    // Sprawdzenie AND dwóch wejść
    //int andResult = myStero.check_and(0, 1);
    
    // Ustawienie stanów wyjść na podstawie wejść
    myStero.out(0, input0);
    myStero.out(1, input1);
    myStero.out(2, input2);
    myStero.out(3, input3);

    // Wyświetlenie wyników na monitorze szeregowym
    //myStero.showInOut(1,1);
  //  // Serial.print(" AND Result: ");
    Serial.print("marker 1 ");
    Serial.println(myStero.getExtraVar("M1"));
    Serial.print("marker 2 ");
    Serial.println(myStero.getExtraVar("M2"));
    Serial.print("marker 3 ");
    Serial.println(myStero.getExtraVar("M3"));
    Serial.print("marker 4 ");
    Serial.println(myStero.getExtraVar("M4"));
    Serial.print("marker 5 ");
    Serial.println(myStero.getExtraVar("M5"));
    Serial.print("marker 6 ");
    Serial.println(myStero.getExtraVar("M6"));
// Serial.println("Pierwsze 20 adresy i dane w pamięci EEPROM:");
//       for (int i = 0; i < 20; i++) {
//         int odczytanaWartosc = EEPROM.read(i);
//         Serial.print("Adres ");
//         Serial.print(i);
//         Serial.print(": ");
//         Serial.println(odczytanaWartosc);
//       }
    delay(400); // Odstęp czasowy
    while(true){

    };
}