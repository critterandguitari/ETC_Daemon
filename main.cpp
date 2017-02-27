
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>

#include "OSC/OSCMessage.h"
#include "OSC/SimpleWriter.h"
#include "Serial.h"
#include "UdpSocket.h"
#include "SLIPEncodedSerial.h"
#include "Timer.h"

Serial serial;
SLIPEncodedSerial slip;
SimpleWriter dump;

// for communicating with Pd or other program
UdpSocket udpSock(4001);

// exit flag
int quit = 0;

/** OSC messages received internally (from PD or other program) **/
void setLED(OSCMessage &msg);
void reload(OSCMessage &msg);
void sendReady(OSCMessage &msg);
void sendShutdown(OSCMessage &msg);
void quitMother(OSCMessage &msg);
void checkShutdown(OSCMessage &msg);
/* end internal OSC messages received */

/* helpers */
void sendGetKnobs(void);

// for shutdown
int shutdownEnabled = 0;
Timer shutdownTimer;

int main(int argc, char* argv[]) {
      
    uint32_t seconds = 0;
    char udpPacketIn[256];
    uint8_t i = 0;
    int len = 0;
    int page = 0;


    Timer knobPollTimer, pingTimer, upTime;

    knobPollTimer.reset();
    pingTimer.reset();
    upTime.reset();
    shutdownTimer.reset();

    udpSock.setDestination(4000, "localhost");
    OSCMessage msgIn;

    char cmd[256];

    uint8_t blobby[5];
    blobby[0] = 0;
    blobby[1] = 1;
    blobby[2] = 2;
    blobby[3] = 3;
    blobby[4] = 4;

    OSCMessage blobTest("/blobby");
    blobTest.add(blobby, 5);

    // send ready to wake up MCU
    // MCU is ignoring stuff over serial port until this message comes through
    // don't empty the message because it gets sent out periodically incase MCU resets
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    // send it a few times just in case
    for(i = 0; i<4; i++) {
       slip.sendMessage(dump.buffer, dump.length, serial);
       usleep(20000); // wait 20 ms
    }
    
    quit = 0;

    // full udp -> serial -> serial -> udp
    shutdownTimer.reset();
    for (;;){

        // check if we are dealing with a shutdown
        if (shutdownEnabled){
            if (shutdownTimer.getElapsed() > 1000.f){
                shutdownEnabled = 0;
                printf("That's it! shutting down!\n");
                sprintf(cmd, "/root/ETC_Sys/scripts/shutdown.sh &");
                system(cmd);
            }
        }

        // receive udp, send to serial
        len = udpSock.readBuffer(udpPacketIn, 256, 0);
        if (len > 0){
            msgIn.empty();
            for (i = 0; i < len; i++){
                msgIn.fill(udpPacketIn[i]);
            }    
            if(!msgIn.hasError()){
                msgIn.send(dump);
                slip.sendMessage(dump.buffer, dump.length, serial);
           //     msgIn.dispatch("/key", checkShutdown, 0);
            }
            else {
                printf("bad message\n");
            }
            msgIn.empty();
        }   

        // receive serial, send udp
        if(slip.recvMessage(serial)) {
            // send to listening UDP
            udpSock.writeBuffer(slip.decodedBuf, slip.decodedLength);
            
            // check if we need to do shutdown
            msgIn.fill(slip.decodedBuf, slip.decodedLength);
            if(!msgIn.hasError()){
                msgIn.dispatch("/key", checkShutdown, 0);
            }
            else {
                printf("bad message\n");
            }
            
            msgIn.empty();
        }

        // sleep for .5ms
        usleep(750);
        
        // every 1 second send a ping in case MCU resets
        if (pingTimer.getElapsed() > 1000.f){
          //  printf("pinged the MCU at %f ms.\n", upTime.getElapsed());
            pingTimer.reset();
            rdyMsg.send(dump);
            slip.sendMessage(dump.buffer, dump.length, serial);
   
            //blobTest.send(dump);
            //udpSock.writeBuffer(dump.buffer, dump.length);

         }

        // poll for knobs
 /*       if (knobPollTimer.getElapsed() > 40.f){
            knobPollTimer.reset();
            sendGetKnobs();
        }
   */     
        // check exit flag
        if (quit) {
            printf("quitting\n");
            return 0;
        }
    } // for;;
}


void checkShutdown(OSCMessage &msg){

	if (msg.isInt(0)) {
        // shutdown key (0) pressed, enable shutdown, start timer
        if (msg.getInt(0) == 0){
            if (msg.getInt(1) > 0) {
    	        printf("got shutdown key, setting shutdown timer...\n");
                shutdownEnabled = 1;
                shutdownTimer.reset();
            }
            // released before timer ends, disable shutdown
            else {
                shutdownEnabled = 0;
            }
        }
	}
}

void quitMother(OSCMessage &msg){
    quit = 1;
}

void setLED(OSCMessage &msg){
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}

void reload(OSCMessage &msg){
    printf("received reload msg\n");
}

void sendReady(OSCMessage &msg){   
    printf("sending ready...\n");
    OSCMessage rdyMsg("/ready");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}

void sendShutdown(OSCMessage &msg){
    printf("sending shutdown...\n");
    OSCMessage rdyMsg("/shutdown");
    rdyMsg.add(1);
    rdyMsg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
    rdyMsg.empty();
}
/* end internal OSC messages received */

void sendGetKnobs(void){
    OSCMessage msg("/getknobs");
    msg.add(1);
    msg.send(dump);
    slip.sendMessage(dump.buffer, dump.length, serial);
}



