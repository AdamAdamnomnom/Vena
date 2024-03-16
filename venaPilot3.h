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
class Vena {
public:
    Vena();
    void initialize();

    int check(int index, String impuls = "");
    void out(int index, bool val, unsigned long duration = 0);
    bool l_and(bool, bool);
    bool l_or(bool, bool);
    bool l_nand(bool, bool);
    bool l_nor(bool,bool);
    bool l_xor(bool,bool);
    bool l_xnor(bool,bool);
    bool l_not(bool);

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
 
    struct ImpulseGenerator {
        String name;
        unsigned long previousMillis = 0;
        bool impulseState = false;
    };
    ImpulseGenerator generators[MAX_GENERATORS];
};

Vena::Vena() {
}

void Vena::initialize() {
    for (int i = 0; i < 4; i++) {
        pinMode(input[i].pinNumber, INPUT);
        pinMode(output[i].pinNumber, OUTPUT);
        output[i].isRunning = false;
        delay(100);
    }

    for (int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0); 
        EEPROM.commit();
        delay(0);
    }
    pinMode(4, INPUT);
    attachInterrupt(digitalPinToInterrupt(4), handleInterruptStatic, RISING);
}

void (*Vena::stopFunction)() = NULL;

void Vena::setStopFunction(void (*stop)()) {
    stopFunction = stop;
}

void IRAM_ATTR Vena::handleInterruptStatic() {
    if (stopFunction != NULL) {
        stopFunction();
    }
}

int Vena::check(int index, String impuls) {
    if (index >= 0 && index < 4) {
        int actualValueIn = digitalRead(input[index].pinNumber);
        int lastValueIn = input[index].previousValue;
        unsigned long debounceDelay = 0; 

        if (impuls.equals("impuls")) {
            if (actualValueIn != lastValueIn) {
                delay(debounceDelay);
                actualValueIn = digitalRead(input[index].pinNumber);

                if (actualValueIn == 1 && lastValueIn == 0) {
                    input[index].previousValue = 1;
                    return 1;
                } else if (actualValueIn == 0 && lastValueIn == 1) {
                    input[index].previousValue = 0;
                    return 0;
                }
            } else if (actualValueIn == 0 && lastValueIn == 0) {  
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
    return -1;  
}

void Vena::out(int index, bool val, unsigned long duration) {
    if (duration == 0) {
        analogWrite(output[index].pinNumber, val ? 255 : 0);
    } else {
        unsigned long currentTime = millis();

        if (!output[index].isRunning) {
            output[index].isRunning = true;
            output[index].previousValue = val;
            output[index].startTime = currentTime;
            analogWrite(output[index].pinNumber, HIGH);
        } else {
            if (duration > 0 && currentTime - output[index].startTime >= duration) {
                output[index].isRunning = false;
                analogWrite(output[index].pinNumber, LOW);
            }
        }
    }
}

bool Vena::generateImpulse(String name, unsigned long czas1, unsigned long czas2, bool warunek) {
    ImpulseGenerator* generator = nullptr;

    for (int i = 0; i < MAX_GENERATORS; ++i) {
        if (generators[i].name == name) {
            generator = &generators[i];
            break;
        }
    }

    if (generator == nullptr) {
        for (int i = 0; i < MAX_GENERATORS; ++i) {
            if (generators[i].name == "") {
                generator = &generators[i];
                generator->name = name; 
                break;
            }
        }
    }
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

bool Vena::l_and(bool x, bool y) {
    return x && y;
}

bool Vena::l_or(bool x, bool y) {
    return x || y;
}

bool Vena::l_nand(bool x, bool y) {
    return !(x && y);
}

bool Vena::l_nor(bool x, bool y) {
    return !(x || y);
}

bool Vena::l_xor(bool x, bool y) {
    return x != y;
}

bool Vena::l_xnor(bool x, bool y) {
    return x == y;
}

bool Vena::l_not(bool x) {
    return !x;
}

bool Vena::startsWith(const char* phrase, const char* letter) {
    return (*phrase == *letter);  
}

void Vena::setMarker(String name, int value) {
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

int Vena::getMarker(const char* name) {
    int iMarker = getExtraVar(name, 1).toInt();
    return iMarker;
}

void Vena::setCounter(String name, int value) {
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

long int Vena::getCounter(const char* name) {
    String sCounter = getExtraVar(name, 6);
    String gCounter = sCounter.substring(6 - 3);

    long int lCounter = gCounter.toInt(); 

    return lCounter;
}

void Vena::addToCounter(const char* name, int value) {
    String sCounter = getExtraVar(name, 6);
    String fCounter = sCounter.substring(0, 3);
    String valueCounter = sCounter.substring(3);
    long int lvalueCounter = valueCounter.toInt();
    lvalueCounter = lvalueCounter + value;
    String lvalueCounterString = String(lvalueCounter);
    String countFormat = String("000").substring(0, 3 - lvalueCounterString.length()) + lvalueCounterString;
    String finalCounter = fCounter + countFormat;
    const char* charFinalCounter = finalCounter.c_str();
    setExtraVar(name, charFinalCounter);
}

bool Vena::checkCounter(const char* name) {
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

void Vena::resetCounter(const char* name) {
    String sCounter = getExtraVar(name, 6);
    String fCounter = sCounter.substring(0, 3);

    String lvalueCounterString = "000";
    String countFormat = String("000").substring(0, 3 - lvalueCounterString.length()) + lvalueCounterString;

    String finalCounter = fCounter + countFormat;

    const char* charFinalCounter = finalCounter.c_str();

    setExtraVar(name, charFinalCounter);
}

void Vena::startTimer(String name, unsigned long duration) {
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

bool Vena::checkTimer(String name, unsigned long duration) {
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

void Vena::setExtraVar(String name, const char* value) {
    int address = findVariableAddress(name.c_str());
    if (address != -1) {
        for (int i = 0; i < strlen(value); i++) {
            EEPROM.write(address + i, value[i]);
            EEPROM.commit();
        }
        EEPROM.write(address + strlen(value), '\0');
        EEPROM.commit();  
    } else {
        String fullName = name + String(value);
        int length = fullName.length();
        for (int i = 0; i < length; i++) {
            EEPROM.write(acAdres + i, fullName.charAt(i));
            EEPROM.commit();
        }
        EEPROM.write(acAdres + length, '\0');
        EEPROM.commit();        
        acAdres += length + 1;  
    }
}

String Vena::getExtraVar(const char* name, int length) {
    int address = findVariableAddress(name);
    String finalValue = "";
    if (address != -1) {
        for (int i = 0; i < length; i++) {
            char charValue = EEPROM.read(address + i);
            finalValue += String(charValue);
        }
        return finalValue;
    }
    return "-1";
}

int Vena::findVariableAddress(const char* name) {
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

void Vena::showInOut(bool inputs, bool outputs) {
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

 void Vena::sendMsg(const char* message) {
   a.write(message);  // nie moze byc @
   a.write('@');
 }
 String Vena::readMsg() {
   String cd;
   cd = a.readStringUntil('@');
   return cd;
 }
void Vena::reset() {
    ESP.reset();
}
