#include "Arduino.h"
#include <SdFat.h>

// const int MAX_EXTRA_VARS = 256; // Maksymalna liczba dodatkowych zmiennych
// const int VAR_NAME_LENGTH = 10; // Maksymalna długość nazwy zmiennej
const char* filename = "data.txt";
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
  void addExtraVar(const char* name, int value);
  void setExtraVar(const char* name, int value);
  int getExtraVar(const char* name);
private:
  int input[4];
  int output[4];
  int outputVal[4];
  SdFat SD;
  bool checkVarInSD(const char* name);
  bool startsWith(const char* phrase, const char* letter);
  static const char* errors[];
  //void sErrors()
};

nzStero::nzStero() {
  // Konstruktor
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
    // Dodaj inne komunikaty błędów w razie potrzeby
  };


  input[0] = 17;  // 6 zmiana kacpra wszystkie piny input;
  input[1] = 16;  // 3;
  input[2] = 15;  // 7;
  input[3] = 18;  // 5;
                  //

  output[0] = 7;
  output[1] = 6;
  output[2] = 5;
  output[3] = 3;  //zmiana kacpra

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

  // pinMode(7, OUTPUT);
  // pinMode(8, OUTPUT);
  // pinMode(9, OUTPUT);

  File file = SD.open("data.txt", FILE_WRITE);
  // if (!SD.begin(10)) {
  // Serial.println("SD initialization failed!");
  //  while (1)
  //  ;
}


int nzStero::check(int index) {
  if (index >= 0 && index < 4) {
    if(analogRead(input[index] )> 900){
      return 1;
    }
    else{
      return 0;
    }
    // return digitalRead(input[index]);
  }
  return -1;
  //sErrors()
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

bool nzStero::checkVarInSD(const char* sName) {
  File file = SD.open(filename, FILE_READ);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line.indexOf(sName) != -1) {
        file.close();
        return true;  // Znaleziono frazę
      }
    }
    file.close();
  }
  return false;  // Nie znaleziono frazy
}


void nzStero::addExtraVar(const char* name, int value = 0) {
  if (checkVarInSD(name) == false) {
    File file = SD.open(filename, FILE_WRITE);  // Otwarcie pliku w trybie zapisu

    if (file) {
      char line[50];
      sprintf(line, "%s=%d", name, value);  // Sformatowanie linii tekstu
      file.println(line);                   // Dopisanie nowej linii do pliku
      file.close();                         // Zamknięcie pliku
    }
  }
}
void nzStero::setExtraVar(const char* name, int value) {
  if (checkVarInSD(name)) {
    File originalFile = SD.open(filename, FILE_READ);
    File tempFile = SD.open("temp.txt", FILE_WRITE);

    if (originalFile && tempFile) {
      while (originalFile.available()) {
        String line = originalFile.readStringUntil('\n');
        if (!line.startsWith(name)) {
          tempFile.println(line);
        } else {
          char newline[50];
          sprintf(newline, "%s=%d", name, value);
          tempFile.println(newline);
        }
      }

      originalFile.close();
      tempFile.close();

      SD.remove(filename);
      SD.rename("temp.txt", filename);
    }
  }
}

int nzStero::getExtraVar(const char* name) {
  File file = SD.open(filename, FILE_READ);
  int value = -1;  // Wartość domyślna, jeśli zmienna nie zostanie znaleziona

  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      if (line.startsWith(name)) {
        int equalsPos = line.indexOf('=');
        if (equalsPos != -1) {
          value = line.substring(equalsPos + 1).toInt();
        }
        break;  // Przerywamy pętlę, bo znaleźliśmy wartość
      }
    }
    file.close();
  }

  return value;
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