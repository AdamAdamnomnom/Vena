// #include <SoftwareSerial.h>
// #include <Arduino.h>
// #include "venaPilot3.h"

// Vena vena;

// SoftwareSerial a(3, 1);  // TX, RX

// int x = 0, y = 0, z = 0, q = 0;

// void setup() {
 
//   vena.initialize();
//   Serial.begin(9600);
//   a.begin(4800);

//   // Inicjalizacja markerów i licznika
//   vena.setMarker("M001", LOW);
//   vena.setMarker("M002", LOW);
//   vena.setCounter("C001", 0);
// }

// void loop() {
//   // Sprawdź warunki, czy należy wykonać kolejną iterację
//   if (vena.l_or(vena.check(0), z != 0) || q != 0) {
//     send("3");  // LED świeci ciągle

//     // Warunek dla pierwszej fazy
//     if (z == 0 && q == 0) {
//       send("1");  // Wysłanie Y3
//       vena.startTimer("T001", 5000);
//       z = 1; // Oznaczamy, że pierwsza faza jest w trakcie
//     }

//     // Sprawdź czy minął czas pierwszej fazy lub wystąpił impuls na pinie D8
//     if (vena.checkTimer("T001", 5000) || x != 0) {
//       x++;

//       if (x == 1) {
//         send("5");  // Wysłanie Y1
//       }

//       // Warunek dla drugiej fazy
//       if (vena.check(2) || y != 0) {
//         y++;

//         if (y == 1) {
//           send("6");  // Wysłanie Y2
//         }
//         send("2");  // wysów y3
//         // Warunek dla trzeciej fazy - zakończenie sekwencji
            // q=0 x>1 y>1 z>1  
//         if (digitalRead(D0)) {
//           send("7"); // Wysłanie sygnału potwierdzenia
//           delay(200);
//           send("7");
//           delay(200);
//           send("7");

//           // Inkrementuj licznik C001
//           vena.addToCounter("C001", 1);

//           // Zresetuj zmienne sterujące
//           x = 0;
//           y = 0;
//           z = 0;
//           q++; // Inkrementuj q, aby oznaczyć zakończenie sekwencji
//         }
//       }
//     }
//   }

//   // Sprawdź warunek zakończenia sekwencji
//   if (vena.getCounter("C001") == 3) {
//     send("4");  // Miganie diody
//     vena.setMarker("M001", 0); // Wyłączenie diody
//     vena.resetCounter("C001"); // Zresetowanie licznika
//     q = 0; // Zresetowanie zmiennej q
//   }
// }

// void send(const char* msg) {
//   delay(123);
//   a.write(msg);
//   a.write('@');
// }















#include <SoftwareSerial.h>
#include <Arduino.h>
#include "venaPilot3.h"
//to było esp działało
//esp 3,1
Vena vena;

SoftwareSerial a(3, 1);  // TX, RX

int x, y, z , q, b = 0;
void setup() {
  vena.initialize();
  Serial.begin(9600);
  a.begin(4800);
  // miga
  vena.setMarker("M001", LOW);
//  vena.setMarker("M002", LOW);
//  Vena::setStopFunction(stop);
  //reset();
  // attachInterrupt(digitalPinToInterrupt(4), stop, RISING);
  vena.setCounter("C001", 3);
}
void stop(){
  send("9");
  ESP.restart();

}
void loop() {

if(digitalRead(4)==HIGH){
  stop();
}

  if (vena.l_or(vena.check(0), z != 0)|| q !=0 ) {


    send("3");  // led swieci ciagle
    // teraz counter s
    if (b==3) {
      send("4");  //magenie diody
      vena.setMarker("M001", 0);
      // vena.resetCounter("C001");
      b=0;
      q=0;
    } else {
      z++;

      if (z == 1) {
        send("1");  //wswów y3
        if(q==0){
        vena.startTimer("T001", 5000);
        vena.resetTimer("T001");
        }else{
           vena.resetTimer("T001");
        }
        
      }


      if (vena.checkTimer("T001", 5000) == 1 || x != 0) {
        x++;

        if (x == 1) {
          send("5");  // wysów y1
        }
        if (vena.check(2) || y != 0) {
          y++;
           if (y == 1) {
            send("6");  // wsów y2
            }
          send("2");  // wysów y3
          

    

          if ( digitalRead(D8)) {
            send("7");
            delay(200);
            send("7");
            send("7");
            delay(200);
            send("7");
            send("7");
            b++;
            x = 0;
            z = 0;
            y= 0;
            q++;// do zastapienia przrz spwadzanie czy counter jest wiekszy od zera
          }
        }
      }
    }
  }
}

void send(const char* msg) {
  delay(123);
  a.write(msg);  // nie moze byc @
  a.write('@');
}