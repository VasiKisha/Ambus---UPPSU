//Copyright (c) 2020 VasiKisha
//All rights reserved.

//This source code is licensed under the MIT-style license found in the
//LICENSE file in the root directory of this source tree. 

//Device description
#define DEVICE "UPPSU"
//Firmware version
#define FIRMWARE "1.0"

//Commands
//DEV?
//FW?
//SWITCH?
//MODE?
//VOLT?
//CURR?
//SETV(?)
//SETC(?)
//SWREM(?)
//OUTPUT?
//CALVG(?)
//CALVO(?)
//CALIG(?)
//CALIO(?)
//CALS
//CALR

#include "ambus.h"
#include "EEPROM.h"

#define SWITCH 13   //HW switch
#define LED 12      //LED indicator
#define CCMODE 9    //CC mode = true
#define CVREF 11    //voltage control
#define CCREF 10    //current control
#define VREAD A0    //voltage measure
#define CREAD A1    //current measure

#define MAXVREAD    24.58333    //(Rh = 4k7, Rl = 1k2)
#define MAXCREAD    2.08333     //(Rs = 50mR, A = 48)
#define MAXVSET     19.16666    //(Rh = 12k, Rl = 1k8)
#define MAXCSET     9.09090     //(Rs = 50mR, A = 11)

struct Read
{
    bool switchHW;
    bool switchRem;
    bool ccMode;
    int voltage;
    int current;
};

struct Set
{
    bool led;
    bool output;
    int ccRef;
    int cvRef;
};

struct Timer
{
    int t10;
    int t1000;
    int t300;
};

struct Calibration
{
    float vGain;
    int vOffset;
    float iGain;
    int iOffset;
};

struct Uppsu
{
    Set set;
    Read read;
    Timer timer;
    Calibration calibration;
}uppsu;

unsigned long previousMillis = 0;

AMBUS ambus("UPPSU");

void setup()
{
    pinMode(CCMODE, INPUT);
    pinMode(SWITCH, INPUT);
    pinMode(LED, OUTPUT);
    pinMode(CVREF, OUTPUT);
    pinMode(CCREF, OUTPUT);
    analogReference(DEFAULT);
    
    //uppsu.calibration.iGain = 1.0;
    //uppsu.calibration.iOffset = 0.0;
    //uppsu.calibration.vGain = 1.0;
    //uppsu.calibration.vOffset = 0.0;
    EEPROM.get(0, uppsu.calibration);
    uppsu.read.switchRem = true;
    Measure();

    Serial.begin(115200);

    uppsu.timer.t10 = 10;
    uppsu.timer.t1000 = 1000;
    uppsu.timer.t300 = 300;
}

void loop()
{
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis)
    {
        previousMillis = currentMillis;

        if (!uppsu.timer.t300--)
        {
            uppsu.timer.t300 = 300;
            Measure();
            Control();
            
        }

        if (!uppsu.timer.t1000--)
        {
            //timer.t1000 = 1000;
            //if (telemetry.switchState)
            //{
            //    Serial.write("Switch ON, LED ON, CVREF 1/4, CCREF 1/4, ");
            //    control.led = true;
            //    control.cvRef = 64;
            //    control.ccRef = 64;
            //}
            //else
            //{
            //    Serial.write("Switch OFF, LED OFF, CVREF 0, CCREF 0, ");
            //    control.led = false;
            //    control.cvRef = 0;
            //    control.ccRef = 0;
            //}
            //if (telemetry.ccMode)
            //{
            //    Serial.write("CC MODE, ");
            //}
            //else
            //{
            //    Serial.write("CV MODE, ");
            //}
            //Serial.write("Voltage ");
            //Serial.print(telemetry.voltage, HEX);
            //Serial.write(", ");
            //Serial.write("Current ");
            //Serial.print(telemetry.current, HEX);
            //Serial.write("\n");
        }       
    }

    if (ambus.dataReceived())
    {
        if (ambus.getCommand() == "DEV?")
        {
            ambus.acknowledge(DEVICE);
        }
        else if (ambus.getCommand() == "FW?")
        {
            ambus.acknowledge(FIRMWARE);
        }
        else if (ambus.getCommand() == "SWITCH?")
        {
            if (uppsu.read.switchHW)
                ambus.acknowledge("ON");
            else
                ambus.acknowledge("OFF");
        }
        else if (ambus.getCommand() == "MODE?")
        {
            if (uppsu.read.ccMode)
                ambus.acknowledge("CC");
            else
                ambus.acknowledge("CV");
        }
        else if (ambus.getCommand() == "VOLT?")
        {
            ambus.acknowledge(String(MAXVREAD * uppsu.read.voltage / 1023.0, 2));
        }
        else if (ambus.getCommand() == "CURR?")
        {
            ambus.acknowledge(String(MAXCREAD * uppsu.read.current / 1023.0, 2));
        }
        else if (ambus.getCommand() == "SETV")
        {
            String temp = ambus.getData();
            uppsu.set.cvRef = 255 * temp.toDouble() / MAXVSET;
            if (uppsu.set.cvRef > 255) uppsu.set.cvRef = 255;
            if (uppsu.set.cvRef < 0) uppsu.set.cvRef = 0;
            ambus.acknowledge(String(MAXVSET * uppsu.set.cvRef / 255, 2));
        }
        else if (ambus.getCommand() == "SETV?")
        {
            ambus.acknowledge(String(MAXVSET * uppsu.set.cvRef / 255, 2));
        }
        else if (ambus.getCommand() == "SETC")
        {
            String temp = ambus.getData();
            uppsu.set.ccRef = 255 * temp.toDouble() / MAXCSET;
            if (uppsu.set.ccRef > 255) uppsu.set.ccRef = 255;
            if (uppsu.set.ccRef < 0) uppsu.set.ccRef = 0;
            ambus.acknowledge(String(MAXCSET * uppsu.set.ccRef / 255, 2));
        }
        else if (ambus.getCommand() == "SETC?")
        {
            ambus.acknowledge(String(MAXCSET * uppsu.set.ccRef / 255, 2));
        }
        else if (ambus.getCommand() == "SWREM")
        {
            String temp = ambus.getData();
            if (temp == "ON")
            {
                uppsu.read.switchRem = true;
                ambus.acknowledge(temp);
            }
            else if (temp == "OFF")
            {
                uppsu.read.switchRem = false;
                ambus.acknowledge(temp);
            }
        }
        else if (ambus.getCommand() == "SWREM?")
        {
            if (uppsu.read.switchRem)
                ambus.acknowledge("ON");
            else
                ambus.acknowledge("OFF");
        }
        else if (ambus.getCommand() == "OUTPUT?")
        {
            if (uppsu.set.output)
                ambus.acknowledge("ON");
            else
                ambus.acknowledge("OFF");
        }
        else if (ambus.getCommand() == "CALVG")
        {
            String temp = ambus.getData();
            uppsu.calibration.vGain = temp.toFloat();
            ambus.acknowledge(String(uppsu.calibration.vGain, 2));
        }
        else if (ambus.getCommand() == "CALVG?")
        {
            ambus.acknowledge(String(uppsu.calibration.vGain, 2));
        }
        else if (ambus.getCommand() == "CALVO")
        {
            String temp = ambus.getData();
            uppsu.calibration.vOffset = temp.toInt();
            ambus.acknowledge(String(uppsu.calibration.vOffset));
        }
        else if (ambus.getCommand() == "CALVO?")
        {
            ambus.acknowledge(String(uppsu.calibration.vOffset));
        }
        else if (ambus.getCommand() == "CALIG")
        {
            String temp = ambus.getData();
            uppsu.calibration.iGain = temp.toFloat();
            ambus.acknowledge(String(uppsu.calibration.iGain, 2));
        }
        else if (ambus.getCommand() == "CALIG?")
        {
            ambus.acknowledge(String(uppsu.calibration.iGain, 2));
        }
        else if (ambus.getCommand() == "CALIO")
        {
            String temp = ambus.getData();
            uppsu.calibration.iOffset = temp.toInt();
            ambus.acknowledge(String(uppsu.calibration.iOffset));
        }
        else if (ambus.getCommand() == "CALIO?")
        {
            ambus.acknowledge(String(uppsu.calibration.iOffset));
        }
        else if (ambus.getCommand() == "CALS")
        {
            EEPROM.put(0, uppsu.calibration);
            ambus.acknowledge("CALSaved");
        }
        else if (ambus.getCommand() == "CALR")
        {
            EEPROM.get(0, uppsu.calibration);
            ambus.acknowledge("CALRestored");
        }
    }
}

void Measure()
{
    uppsu.read.switchHW = digitalRead(SWITCH);
    uppsu.read.ccMode = digitalRead(CCMODE);
    uppsu.read.voltage = analogRead(VREAD);
    uppsu.read.current = analogRead(CREAD);
}

void Control()
{
    if (uppsu.read.switchHW && uppsu.read.switchRem)
    {
        uppsu.set.output = true;
        uppsu.set.led = true;
    }
    else
    {
        uppsu.set.output = false;
        uppsu.set.led = false;
    }

    //if(uppsu.set.output) digitalWrite(,HIGH);
    //else digitalwrite(,LOW);
    if (uppsu.set.led) digitalWrite(LED, HIGH);
    else digitalWrite(LED, LOW);

    int calValue = uppsu.calibration.vOffset + (int)(uppsu.set.cvRef * uppsu.calibration.vGain);
    if (calValue > 255) calValue = 255;
    analogWrite(CVREF, calValue );
    calValue = uppsu.calibration.iOffset + (int)(uppsu.set.ccRef * uppsu.calibration.iGain);
    if (calValue > 255) calValue = 255;
    analogWrite(CCREF, calValue);
}

void serialEvent()
{
    ambus.serialEventHandler();
}
