//Copyright (c) 2020 VasiKisha
//All rights reserved.

//This source code is licensed under the MIT-style license found in the
//LICENSE file in the root directory of this source tree. 

/*
 * ambus.h
 *
 * Created: 02.07.2016 9:15:07
 *  Author: VasiKisha
 */ 

#ifndef AMBUS_H_
#define AMBUS_H_

#define DIRECTIONPIN    5

class AMBUS
{
    public:    
        AMBUS(String myAddress);
        
        void serialEventHandler();                //use in serialEvent() - handles byte receiving, catching SOP and EOP, message analysis, CRC check and Address check
        boolean dataReceived();                    //True = new message is ready to pickup, False = no new message
        String getCommand();                    //returns command from received message
        String getData();                        //returns data from received message
        void notacknowledge();                    //do not acknoledge message
        void acknowledge(String answer = "");    //message is acknowledged, optionally data can be included
        
    private:
        boolean receiving;                        //receinving flag
        boolean dataReady;                        //data ready flag
        
        String address;
        String command;
        String data;
        String crc;
        String deviceAddress;
        String dataPacket;
        
        void stringParser();                    //analyzuje dataPacket
        char checksum(String data, unsigned int count);
};

#endif /* AMBUS_H_ */
