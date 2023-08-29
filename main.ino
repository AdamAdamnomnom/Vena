#include "Arduino.h"
#include "nzStero.h"

nzStero myStero;

void setup() {
    Serial.begin(9600);
    //myStero.addExtraVar(int M4, int 0)
    myStero.initialize(); // Inicjalizacja biblioteki
    myStero.addExtraVar("M1", 0);
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
    myStero.showInOut(1,1);
   // Serial.print(" AND Result: ");
    //Serial.println(andResult);
    myStero.setExtraVar("M1", 1);
    Serial.println(myStero.getExtraVar("M1"));
    delay(400); // Odstęp czasowy
}