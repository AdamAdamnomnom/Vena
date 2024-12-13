#include "Arduino.h"
#include <EEPROM.h>


const int EEPROM_SIZE = 1024;
const int MAX_TIMER = 10;
int numberOfTimers = 0;
class nzStero {
public:
  nzStero();
  void initialize();

 int check(int index, String impuls);
  void out(int index, bool val, int time);
  int check_and(int, int);
  int check_or(int, int);
  int check_nand(int, int);
  int check_nor(int, int);
  int check_xor(int, int);
  void showInOut(bool, bool);
  void setMarker(String name, int value);
  int getMarker(const char* name);
  void setCounter(String name, int value);
  void resetCounter(const char* name);
  void addToCounter(const char* name, int value);
  bool checkCounter(const char* name);
  long int getCounter(const char* name);
  void startTimer(String name, unsigned long duration);
bool checkTimer(String name, unsigned long duration);
  void stopFunction();

private:
 void setExtraVar(String name, const char* value);
  String getExtraVar(const char* name, int length);
  int findVariableAddress(const char* name);
  bool startsWith(const char* phrase, const char letter);
  static const char* errors[];

  int acAdres = 0;
  int countMax = 999;
  static nzStero* instance;
  struct Pin {
    int pinNumber;
    int value;
    int previousValue;
  };
  Pin input[4];
  Pin output[4];
  struct Timer {
    String name;
    unsigned long startTime;
    unsigned long duration;
    bool active;
  };
  static void handleInterrupt() {
    if (instance != NULL) {
      instance->stopFunction();
    }
  }
  void stop() {
    // Tutaj możesz umieścić kod, który ma być wykonany po wystąpieniu przerwania
    // Na przykład zatrzymanie programu lub inne działania
  }

  Timer timers[10];
};

nzStero* nzStero::instance = NULL;

nzStero::nzStero() {
  instance = this;
}
void nzStero::initialize() {
  const char* errors[] = {
    "Bez błędu",
    "Zły input ",
    "Zły output",
    "Zła warotść",
    "Nie utworzono markra",
    "Marker już istnije",
    "Nie zdefionowano wartośći ",
    "Bład karty SD",
  };

   stopFunction();
  input[0] = { D4, 0, 0 };
  input[1] = { D2, 0, 0 };
  input[2] = { D1, 0, 0 };
  input[3] = { D5, 0, 0 };

  output[0] = { D7, 0, 0 };
  output[1] = { D6, 0, 0 };
  output[2] = { D5, 0, 0 };
  output[3] = { D3, 0, 0 };

  for (int i = 0; i < 4; i++) {
    pinMode(input[i].pinNumber, INPUT);
    pinMode(output[i].pinNumber, OUTPUT);
  }

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0); // Zmiana NULL na 0
  }
}

void nzStero::stopFunction() {

  attachInterrupt(digitalPinToInterrupt(D2), handleInterrupt, FALLING);
}

int nzStero::check(int index, String impuls = ""){
  if (index >= 0 && index < 4) {
    int actualValueIn = digitalRead(input[index].pinNumber);
    int lastValueIn = input[index].previousValue;
    unsigned long debounceDelay = 0;

    if (impuls.equals("impuls")) {
      if (actualValueIn != lastValueIn) {
        delay(debounceDelay);
        actualValueIn = digitalRead(input[index].pinNumber);

        if (actualValueIn == HIGH && lastValueIn == LOW) {
          input[index].previousValue = HIGH;
          Serial.println("New impulse detected");
          return HIGH;
        } else if (actualValueIn == LOW && lastValueIn == HIGH) {
          input[index].previousValue = LOW;
          return LOW;
        }
      } else if (actualValueIn == LOW && lastValueIn == LOW) {
        return LOW;
      } else if (actualValueIn == HIGH && lastValueIn == HIGH) {
        return LOW;
      }
    } else {
      return actualValueIn;
    }
  }
  return -1;
}

void nzStero::out(int index, bool val, int time) {
  if (val == HIGH || val == LOW) {
    if (index >= 0 && index < 4) {
      analogWrite(output[index].pinNumber, val ? 255 : 0);
      output[index].value = val;

      if (time > 0) {
        output[index].previousValue = !val;

        unsigned long startTime = millis();

        while (millis() - startTime < time) {
          // Czekamy, aż upłynie określony czas
        }

        analogWrite(output[index].pinNumber, output[index].previousValue ? 255 : 0);
        output[index].value = output[index].previousValue;
      }
    } else {
      Serial.println("Error: Invalid output index");
    }
  } else {
    Serial.println("Error: Invalid value");
  }
}

int nzStero::check_and(int x, int y) {
  if (check(x) == HIGH && check(y) == HIGH) {
    return HIGH;
  }
  return LOW;
}

int nzStero::check_or(int x, int y) {
  if (check(x) == HIGH || check(y) == HIGH) {
    return HIGH;
  }
  return LOW;
}

int nzStero::check_nand(int x, int y) {
  if (check(x) == HIGH && check(y) == HIGH) {
    return LOW;
  }
  return HIGH;
}

int nzStero::check_nor(int x, int y) {
  if (check(x) == LOW || check(y) == LOW) {
    return HIGH;
  }
  return LOW;
}

int nzStero::check_xor(int x, int y) {
  if (check(x) == HIGH && check(y) == HIGH) {
    return LOW;
  }
  if (check(x) == HIGH || check(y) == HIGH) {
    return HIGH;
  }
  return LOW;
}

bool nzStero::startsWith(const char* phrase, const char letter) {
  return (*phrase == letter);
}


void nzStero::setMarker(String name, int value) {
  if (name.length() != 4 || name[0] != 'M' || !isdigit(name[1]) || !isdigit(name[2]) || !isdigit(name[3])) {
    Serial.println("Błędna nazwa zmiennej. Oczekiwano formatu MXYZ, gdzie XYZ to trzy cyfry.");
    return;
  }
  if (value != 1 && value != 0) {
    Serial.println("Błędna wartość. Oczekiwana 0/1");
    return;
  }
  String stringValue = String(value);

  setExtraVar(name, stringValue.c_str());
}

int nzStero::getMarker(const char* name) {
  int iMarker = getExtraVar(name, 1).toInt();
  return iMarker;
}

void nzStero::setCounter(String name, int value) {
  if (name.length() != 4 || name[0] != 'C' || !isdigit(name[1]) || !isdigit(name[2]) || !isdigit(name[3])) {
    Serial.println("Błędna nazwa zmiennej. Oczekiwano formatu CXYZ, gdzie XYZ to trzy cyfry.");
    return;
  }
  if (value < 0 || value > countMax) {
    Serial.print("Błędna wartość. Oczekiwany zakres 0-");
    Serial.println(countMax);
    return;
  }

  String countFormat = String(value);

  int countLength = countFormat.length();

  if (countLength < 3) {
    countFormat = String("000").substring(0, 3 - countLength) + countFormat;
  }

  countFormat += "000";

  setExtraVar(name, countFormat.c_str());
}

long int nzStero::getCounter(const char* name) {
  String sCounter = getExtraVar(name, 6);
  String gCounter = sCounter.substring(6 - 3);


  long int lCounter = gCounter.toInt();  // Konwersja na long int

  return lCounter;
}

void nzStero::addToCounter(const char* name, int value) {
  String sCounter = getExtraVar(name, 6);
  String fCounter = sCounter.substring(0, 3);
  String valueCounter = sCounter.substring(3);

  //long int lfCounter = fCounter.toInt();
  long int lvalueCounter = valueCounter.toInt();

  lvalueCounter = lvalueCounter + value;

  String lvalueCounterString = String(lvalueCounter);
  String countFormat = String("000").substring(0, 3 - lvalueCounterString.length()) + lvalueCounterString;

  String finalCounter = fCounter + countFormat;

  const char* charFinalCounter = finalCounter.c_str();
  Serial.println(charFinalCounter);

  setExtraVar(name, charFinalCounter);
}

bool nzStero::checkCounter(const char* name) {
  String sCounter = getExtraVar(name, 6);
  String fCounter = sCounter.substring(0, 3);
  String valueCounter = sCounter.substring(3);

  long int lfCounter = fCounter.toInt();
  long int lvalueCounter = valueCounter.toInt();

  if (lfCounter == lvalueCounter) {
    return true;
  } else {
    return false;
  }
}

void nzStero::resetCounter(const char* name) {
  String sCounter = getExtraVar(name, 6);
  String fCounter = sCounter.substring(0, 3);


  String lvalueCounterString = "000";
  String countFormat = String("000").substring(0, 3 - lvalueCounterString.length()) + lvalueCounterString;

  String finalCounter = fCounter + countFormat;

  const char* charFinalCounter = finalCounter.c_str();
  Serial.println(charFinalCounter);

  setExtraVar(name, charFinalCounter);
}


void nzStero::startTimer(String name, unsigned long duration) {
  if (numberOfTimers < MAX_TIMER) {
    timers[numberOfTimers].name = name;
    timers[numberOfTimers].startTime = millis();
    timers[numberOfTimers].duration = duration;
    timers[numberOfTimers].active = true;
    numberOfTimers++;
  } else {
    Serial.println("Osiągnięto maksymalną liczbę timerów. Nie można dodać kolejnego.");
  }
}

bool nzStero::checkTimer(String name, unsigned long duration = 0) {
  for (int i = 0; i < numberOfTimers; ++i) {
    if (timers[i].name == name) {
      if (duration == 0) {
        if (timers[i].active && (millis() - timers[i].startTime >= timers[i].duration)) {
          timers[i].active = false;
          return true;
        } else {
          return false;
        }
      } else {
        if (timers[i].active && (millis() - timers[i].startTime >= duration)) {
          timers[i].active = false;
          return true;
        } else {
          return false;
        }
      }
    }
  }
  Serial.println("Nie znaleziono timera o nazwie: " + name);
  return false;
}




void nzStero::setExtraVar(String name, const char* value) {
  int address = findVariableAddress(name.c_str());
  if (address != -1) {
    for (int i = 0; i < strlen(value); i++) {
      EEPROM.write(address + i, value[i]);
    }
    EEPROM.write(address + strlen(value), '\0');  // Zakończ znakiem null
  } else {
    String fullName = name + String(value);
    int length = fullName.length();
    for (int i = 0; i < length; i++) {
      EEPROM.write(acAdres + i, fullName.charAt(i));
    }
    EEPROM.write(acAdres + length, '\0');  // Zakończ znakiem null
    acAdres += length + 1;                 // Aktualizuj wskaźnik
  }
}

String nzStero::getExtraVar(const char* name, int length) {
  int address = findVariableAddress(name);
  String finalValue = "";
  if (address != -1) {
    for (int i = 0; i < length; i++) {
      char charValue = EEPROM.read(address + i);
      // Serial.print("Odczytano znak na adresie ");
      // Serial.print(address + i);
      // Serial.print(": ");
      // Serial.println(charValue);
      finalValue += String(charValue);
    }
    // Serial.print("Odczytana wartość: ");
    // Serial.println(finalValue);
    return finalValue;
  }
  // Serial.println("Zmienna nie znaleziona");
  return "-1";
}

int nzStero::findVariableAddress(const char* name) {
  for (int address = 0; address < EEPROM_SIZE; address++) {
    int length = strlen(name);
    bool found = true;
    for (int i = 0; i < length; i++) {
      if (EEPROM.read(address + i) != name[i]) {
        found = false;
        break;
      }
    }
    if (found) {
      return address + length;
    }
  }
  return -1;
}

void nzStero::showInOut(bool inputs, bool outputs) {
  if (inputs == 1) {
    Serial.print("Inputs: ");
    for (int i = 0; i < 4; i++) {
      input[i].value = check(i);
      Serial.print(this->input[i].value);
      Serial.print(" ");
    }
  }
  Serial.println(" ");
  if (outputs == 1) {
    Serial.print("Outputs: ");
    for (int j = 0; j < 4; j++) {
      Serial.print(this->output[j].value);
      Serial.print(" ");
    }
    Serial.println(" ");
  }
  Serial.println(" ");
}
