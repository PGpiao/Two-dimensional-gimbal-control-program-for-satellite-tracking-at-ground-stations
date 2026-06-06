#include "easycomm.h"

class EasyCommProxy : public Stream {
private:
    Stream* _realSerial;
    float* _az;
    float* _el;
    String outBuffer;
    String rxLine;

public:
    EasyCommProxy(Stream* realSerial, float* az, float* el) 
        : _realSerial(realSerial), _az(az), _el(el), outBuffer(""), rxLine("") {}

    int available() override {
        if (outBuffer.length() > 0) return outBuffer.length();

        while (_realSerial->available() > 0) {
            char c = _realSerial->read();
            rxLine += c;

            if (c == '\n') {
                String cleanLine = rxLine;
                cleanLine.trim(); 

                if (cleanLine == "AZ") {
                    _realSerial->print("AZ");
                    _realSerial->print(*_az, 1);
                    _realSerial->print("\n");
                    rxLine = ""; 
                } 
                else if (cleanLine == "EL") {
                    _realSerial->print("EL");
                    _realSerial->print(*_el, 1);
                    _realSerial->print("\n");
                    rxLine = ""; 
                } 
                else {
                    outBuffer += rxLine;
                    rxLine = "";
                    return outBuffer.length();
                }
            }
        }
        return outBuffer.length();
    }

    int read() override {
        if (outBuffer.length() > 0) {
            char c = outBuffer.charAt(0);
            outBuffer.remove(0, 1);
            return c;
        }
        return -1;
    }

    int peek() override {
        if (outBuffer.length() > 0) return outBuffer.charAt(0);
        return -1;
    }

    void flush() override { _realSerial->flush(); }
    size_t write(uint8_t b) override { return _realSerial->write(b); }
};

#define EN1   3
#define DIR1  4
#define ST1   5
#define ENC1_PWM A2 

#define EN2   8
#define DIR2  7
#define ST2   6
#define ENC2_PWM A1 

float currentAz = 0, currentEl = 0; 
float targetAz = 0,  targetEl = 0;  

unsigned long lastSensorCheck = 0;
bool toggleAxis = false; 

EasyCommProxy myProxy(&Serial, &currentAz, &currentEl);
easycomm *easycom;

float readAngle(int pin, float lastAngle) {
    unsigned long highT = pulseIn(pin, HIGH, 10000);
    unsigned long lowT  = pulseIn(pin, LOW, 10000);   
    if (highT == 0 || lowT == 0) return lastAngle; 
    return ((float)highT / (highT + lowT)) * 360.0;
}

void setup()
{
    pinMode(EN1, OUTPUT); pinMode(DIR1, OUTPUT); pinMode(ST1, OUTPUT);
    pinMode(EN2, OUTPUT); pinMode(DIR2, OUTPUT); pinMode(ST2, OUTPUT);

    digitalWrite(EN1, HIGH);  digitalWrite(EN2, HIGH);
    digitalWrite(DIR1, HIGH); digitalWrite(DIR2, HIGH); 
    digitalWrite(ST1, LOW);   digitalWrite(ST2, LOW);

    Serial.begin(9600);
    Serial.setTimeout(50);

    easycom = new easycomm(&myProxy);

    currentAz = readAngle(ENC1_PWM, 0.0);
    currentEl = readAngle(ENC2_PWM, 0.0);
    targetAz = currentAz;
    targetEl = currentEl;
}

void loop()
{
    if (millis() - lastSensorCheck > 50) {
        lastSensorCheck = millis();
        if (toggleAxis) {
            currentAz = readAngle(ENC1_PWM, currentAz);
        } else {
            currentEl = readAngle(ENC2_PWM, currentEl);
        }
        toggleAxis = !toggleAxis;
    }

    easycom->parse(currentAz, currentEl);

    targetAz = easycom->ComAzim;
    targetEl = easycom->ComElev;

    float errorAz = targetAz - currentAz;
    if (errorAz > 180.0)  errorAz -= 360.0;
    if (errorAz < -180.0) errorAz += 360.0;

    float errorEl = targetEl - currentEl;
    if (errorEl > 180.0)  errorEl -= 360.0;
    if (errorEl < -180.0) errorEl += 360.0;

    bool needStepAz = (abs(errorAz) > 4.0);
    bool needStepEl = (abs(errorEl) > 4.0);

    if (needStepAz || needStepEl) {
        if (needStepAz) digitalWrite(ST1, HIGH);
        if (needStepEl) digitalWrite(ST2, HIGH);
        delayMicroseconds(800); 
        if (needStepAz) digitalWrite(ST1, LOW);
        if (needStepEl) digitalWrite(ST2, LOW);
        delayMicroseconds(800); 
    }
}