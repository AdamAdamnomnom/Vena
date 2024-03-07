#include <Arduino.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial a(3, 1);  // TX, RX

const int EEPROM_SIZE = 512;
const int MAX_TIMER = 10;
int numberOfTimers = 0;
const int MAX_GENERATORS = 10;
int acAdres = 0;
int countMax = 999;
class nzStero {
public:
  nzStero();
  void initialize();

  int check(int index, String impuls = "");
   void out(int index, bool val, unsigned long duration  = 0);
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
  bool checkTimer(String name, unsigned long duration = 0);
  
  bool generateImpulse(String name, unsigned long czas1, unsigned long czas2, bool warunek);

  static void setStopFunction(void (*stop)());
  String readMsg();
  void sendMsg(const char* message);

  void reset();


private:
  void setExtraVar(String name, const char* value);
  String getExtraVar(const char* name, int length);
  int findVariableAddress(const char* name);
  bool startsWith(const char* phrase, const char* letter);
  static const char* errors[];
  static void handleInterruptStatic();
  static void (*stopFunction)();
  void handleInterrupt();

struct Pin {
    int pinNumber;
    bool value;
    bool previousValue;
    unsigned long startTime;
    bool isRunning;
  };
  Pin input[4];
  Pin output[4];

  struct Timer {
    String name;
    unsigned long startTime;
    unsigned long duration;
    bool active;
  };
 Timer timers[MAX_TIMER];
  // Struktura przechowująca nazwę generatora impulsów i jego stan
  struct ImpulseGenerator {
    String name;
    unsigned long previousMillis = 0;
    bool impulseState = false;
  };

  // Tablica generatorów impulsów
  ImpulseGenerator generators[MAX_GENERATORS];
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


  input[0] = { D0, 0, 0 };
  input[1] = { D3, 0, 0 };
  input[2] = { D8, 0, 0 };
  input[3] = { D6, 0, 0 };

  output[0] = { D1, 0, 0 };
  output[1] = { D5, 0, 0 };
  output[2] = { D7, 0, 0 };
  output[3] = { D4, 0, 0 };

  for (int i = 0; i < 4; i++) {
    pinMode(input[i].pinNumber, INPUT);
    pinMode(output[i].pinNumber, OUTPUT);
    output[i].isRunning = 0;
    delay(100);
  }

  for (int i = 0; i < EEPROM.length(); i++) {
    EEPROM.write(i, 0);  // Zmiana NULL na 0
    EEPROM.commit();
    delay(0);
  }
  pinMode(4, INPUT);
  attachInterrupt(digitalPinToInterrupt(4), handleInterruptStatic, RISING);
  Serial.println("init");
   a.begin(4800);
}
void (*nzStero::stopFunction)() = NULL;
void nzStero::setStopFunction(void (*stop)()) {
  stopFunction = stop;
}
void IRAM_ATTR nzStero::handleInterruptStatic() {
  if (stopFunction != NULL) {
    stopFunction();
  }
}


int nzStero::check(int index, String impuls) {
  if (index >= 0 && index < 4) {
    int actualValueIn = digitalRead(input[index].pinNumber);
    int lastValueIn = input[index].previousValue;
    unsigned long debounceDelay = 0;  // Debounce delay time in milliseconds

    if (impuls.equals("impuls")) {
      if (actualValueIn != lastValueIn) {
        delay(debounceDelay);
        actualValueIn = digitalRead(input[index].pinNumber);

        if (actualValueIn == 1 && lastValueIn == 0) {
          input[index].previousValue = 1;
          Serial.println("New impulse detected");
          return 1;
        } else if (actualValueIn == 0 && lastValueIn == 1) {
          input[index].previousValue = 0;
          return 0;
        }
      } else if (actualValueIn == 0 && lastValueIn == 0) {  // Poprawione umiejscowienie warunku
        return 0;
      } else if (actualValueIn == 1 && lastValueIn == 1) {
        return 0;
      }
    } else {
      if (actualValueIn == 1) {
        return 1;
      } else {
        return 0;
      }
    }
  }
  return -1;  // Default return value indicating an error
}
void nzStero::out(int index, bool val, unsigned long duration) {
  if (duration == 0) {
    analogWrite(output[index].pinNumber, val ? 255 : 0);
  } else {
    unsigned long currentTime = millis();

    if (!output[index].isRunning) {
      // Jeśli pin nie jest aktualnie włączony, ustaw jego stan i czas rozpoczęcia
      output[index].isRunning = true;
      output[index].previousValue = val;
      output[index].startTime = currentTime;
      analogWrite(output[index].pinNumber, HIGH);
    } else {
      // Jeśli pin jest aktualnie włączony, sprawdź, czy upłynął czas trwania
      if (duration > 0 && currentTime - output[index].startTime >= duration) {
        // Jeśli upłynął czas trwania impulsu, wyłącz pin
        output[index].isRunning = false;
        analogWrite(output[index].pinNumber, LOW);
      
      }
    }
  }
}

bool nzStero::generateImpulse(String name, unsigned long czas1, unsigned long czas2, bool warunek) {
  ImpulseGenerator* generator = nullptr;

  // Sprawdzamy, czy generator o podanej nazwie istnieje
  for (int i = 0; i < MAX_GENERATORS; ++i) {
    if (generators[i].name == name) {
      generator = &generators[i];
      break;
    }
  }

  // Jeśli nie znaleziono generatora, sprawdzamy, czy jest miejsce na nowy generator
  if (generator == nullptr) {
    for (int i = 0; i < MAX_GENERATORS; ++i) {
      if (generators[i].name == "") {
        generator = &generators[i];
        generator->name = name;  // Ustawiamy nazwę nowego generatora
        break;
      }
    }
  }

  // Jeśli nie znaleziono wolnego miejsca na nowy generator, zwracamy false
  if (generator == nullptr) {
    return false;
  }

  unsigned long currentMillis = millis();

  if (warunek && !(generator->impulseState)) {
    generator->previousMillis = currentMillis;
    generator->impulseState = true;
  }

  if (generator->impulseState) {
    if (currentMillis - generator->previousMillis <= czas1) {
      return true;
    } else if (currentMillis - generator->previousMillis > czas1 && currentMillis - generator->previousMillis <= czas1 + czas2) {
      return false;
    } else {
      generator->impulseState = false;
    }
  }

  return false;
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
  return (*phrase == *letter);  // Modify condition
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

bool nzStero::checkTimer(String name, unsigned long duration) {
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
      EEPROM.commit();
    }
    EEPROM.write(address + strlen(value), '\0');
    EEPROM.commit();  // Zakończ znakiem null
  } else {
    String fullName = name + String(value);
    int length = fullName.length();
    for (int i = 0; i < length; i++) {
      EEPROM.write(acAdres + i, fullName.charAt(i));
      EEPROM.commit();
    }
    EEPROM.write(acAdres + length, '\0');
    EEPROM.commit();        // Zakończ znakiem null
    acAdres += length + 1;  // Aktualizuj wskaźnik
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

 void nzStero::sendMsg(const char* message) {
   a.write(message);  // nie moze byc @
   a.write('@');
 }
 String nzStero::readMsg() {
   String cd;
   cd = a.readStringUntil('@');
   return cd;
 }
void nzStero::reset() {

  ESP.reset();
}
