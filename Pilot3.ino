#include "venaPilot3.h"      // Włączamy bibliotekę venaPilot3.h (Wymagane do działa porgramu)

Vena vena;  // Inicjalizacja obiektu Vena (Wymagane do działa porgramu)

int x, y = 0;  // Inicjalizacja zmiennych x i y

void setup() {
  vena.initialize();  // Inicjalizacja kontrolera Vena (Wymagane do działa porgramu)

  // Ustawienie markerów i liczników
  vena.setMarker("M001", LOW);   // Ustawienie markeru M001 na niski poziom sygnału
  vena.setStopFunction(stop);    // Ustawienie funkcji zatrzymującej (Wymagane do działania przycisku stop)
  vena.setCounter("C001", 3);    // Ustawienie licznika C001 na wartość 3
  vena.setCounter("C002", 0);    // Ustawienie licznika C002 na wartość 0
}

void stop() {
  vena.sendMsg("9");  // Wysłanie wiadomości "9" (Resetowanie)
  vena.reset();       // Zresetowanie kontrolera Vena
}

void loop() {
  // Sprawdzenie warunków dla rozpoczęcia sekwencji
  if (vena.l_or(vena.check(0), vena.getCounter("C002") != 0) || vena.getCounter("C001") > 0) {
    vena.sendMsg("3");  // Wysłanie wiadomości "3" (LED świeci ciągle)

    // Sprawdzenie warunku dla zakończenia 3 sekwencji
    if (vena.getCounter("C001") == 3) {
      vena.sendMsg("4");    // Wysłanie wiadomości "4" (Miganie diody)
      vena.setMarker("M001", 0);  // Ustawienie markeru M001 na wysoki poziom sygnału
      vena.resetCounter("C001");  // Zresetowanie licznika C001
    } else { // W przeciwnym razie
      vena.addToCounter("C002", 1);  // Inkrementacja licznika C002 o 1

       // Faza 1: Wysów Y3 i rozpoaczecie timera 
      if (vena.getCounter("C002") == 1) {
        vena.sendMsg("1");  // Wysłanie wiadomości "1" (Wysuw Y3)
        if (vena.getCounter("C001") == 0) {
          vena.startTimer("T001", 5000);  // Rozpoczęcie timera T001 z czasem 5000 ms
        }
      }

      // Faza 2: Wysuwanie Y1,Y2 oraz Wysuwanie Y3 (jeśli warunek spełniony)
      if (vena.checkTimer("T001", 0) == 1 || x != 0) {
        x++;

        // Faza 3: Wysuwanie Y1
        if (x == 1) {
          vena.sendMsg("5");  // Wysłanie wiadomości "5" (Wysuw Y1)
        }

        // Faza 4: Wysuwanie Y2
        if (vena.check(2) || y != 0) {
          y++;
          if (y == 1) {
            vena.sendMsg("6");  // Wysłanie wiadomości "6" (Wysuw Y2)
          }

          // Faza 5 : Wysuwanie Y3
          vena.sendMsg("2");  // Wysłanie wiadomości "2" (Wysuw Y3)
          
          // Faza 6: Wykrywanie impulsu na pinie D1 i zakończenie sekwencji
          if (vena.check(1)) {
            vena.sendMsg("7");  // Wysłanie wiadomości "7" 
            vena.addToCounter("C001", 1);  
            x = 0;  
            vena.addToCounter("C002", 1);  
          }
        }
      }
    }
  }
}
