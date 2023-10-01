#include "Arduino.h"
#include <EEPROM.h>

const int EEPROM_SIZE = 1024;

class nzStero {
public:
  nzStero();
  void initialize();
  int check(int);
  void out(int, int);
  int check_and(int, int);
  int check_or(int, int);
  int check_nand(int, int);
  int check_nor(int, int);
  int check_xor(int, int);
  void showInOut(bool, bool);  // inputs , outputs
  void setMarker(String name, int value);
  int getMarker(const char* name);
  void setCounter(String name, int value);
  void addToCounter(const char* name, int value);
  bool checkCounter(const char* name);
  long int getCounter(const char* name);

private:
  void setExtraVar(String name, const char* value);
  String getExtraVar(const char* name, int lenght);
  int findVariableAddress(const char* name);
  int input[4];
  int output[4];
  int outputVal[4];

  bool startsWith(const char* phrase, const char* letter);
  static const char* errors[];
  int acAdres = 0;
  int countMax = 999;
};

nzStero::nzStero() {
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

  input[0] = 17;
  input[1] = 16;
  input[2] = 15;
  input[3] = 20;

  output[0] = 7;
  output[1] = 6;
  output[2] = 5;
  output[3] = 3;

  outputVal[0] = 0;
  outputVal[1] = 0;
  outputVal[2] = 0;
  outputVal[3] = 0;

  pinMode(input[0], INPUT);
  pinMode(input[1], INPUT);
  pinMode(input[2], INPUT);
  pinMode(input[3], INPUT);

  pinMode(output[0], OUTPUT);
  pinMode(output[1], OUTPUT);
  pinMode(output[2], OUTPUT);
  pinMode(output[3], OUTPUT);

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, NULL);
  }
}

int nzStero::check(int index) {
  if (index >= 0 && index < 4) {
    if (analogRead(input[index]) > 900) {
      return 1;
    } else {
      return 0;
    }
  }
  return -1;
}

void nzStero::out(int index, int val) {
  if (val == HIGH || val == LOW) {
    if (index >= 0 && index < 4) {
      if (val == 1) {
        analogWrite(output[index], 255);
      } else {
        analogWrite(output[index], 0);
      }
      outputVal[index] = val;
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

bool nzStero::startsWith(const char* phrase, const char* letter) {
  return (phrase[0] == letter);
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

void nzStero::showInOut(bool input, bool output) {
  if (input == 1) {
    Serial.print("Inputs: ");
    Serial.print(check(0));
    Serial.print(" ");
    Serial.print(check(1));
    Serial.print(" ");
    Serial.print(check(2));
    Serial.print(" ");
    Serial.print(check(3));
    Serial.print(" ");
  }
  if (output == 1) {
    Serial.print("Outputs: ");
    Serial.print(outputVal[0]);
    Serial.print(" ");
    Serial.print(outputVal[1]);
    Serial.print(" ");
    Serial.print(outputVal[2]);
    Serial.print(" ");
    Serial.print(outputVal[3]);
    Serial.print(" ");
  }
  Serial.println(" ");
}