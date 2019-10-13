#include <RHReliableDatagram.h>
#include <RH_ASK.h>
#include <SPI.h>

#define TRANSMITTER_ADDRESS 1
#define RECEIVER_ADDRESS 2

RH_ASK driver;
RHReliableDatagram manager(driver, TRANSMITTER_ADDRESS);

const int GREEN = 5;
const int YELLOW = 6;
const int RED = 7;

const int maxSize = 10;

int pending = 0;
int writeTo = 0;
int readFrom = 0;
uint8_t data[10][32];

void setup()
{
  pinMode(GREEN, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(RED, OUTPUT);
  Serial.begin(115200);
  if (!manager.init())
    Serial.println("init failed");
}

void loop()
{
  recvBytesWithStartEndMarkers();
  sendIt();
  digitalWrite(GREEN, LOW);
  digitalWrite(YELLOW, LOW);
}

void recvBytesWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    byte startMarker = 0x3C;
    byte endMarker = 0x3E;
    byte rb;
   

    while (Serial.available() > 0) {
        rb = Serial.read();

        if (recvInProgress == true) {
            if (rb != endMarker) {
                data[writeTo][ndx] = rb;
                ndx++;
            }
            else {
                recvInProgress = false;
                ndx = 0;
                writeTo = next(writeTo);
                pending = pending + 1;
                digitalWrite(GREEN, HIGH);
            }
        }

        else if (rb == startMarker) {
            recvInProgress = true;
        }
    }
}

void receiveIt() {
  if (Serial.available() > 0)  {
    Serial.readBytes(data[writeTo], sizeof(data[writeTo]));
    writeTo = next(writeTo);
    pending = pending + 1;
    digitalWrite(GREEN, HIGH);
  }
}

void sendIt() {
  if (pending > 0) {
    if (!manager.sendtoWait(data[readFrom], sizeof(data[readFrom]), RECEIVER_ADDRESS)) {
      digitalWrite(RED, HIGH);
    }
    memset(data[readFrom], 0, 32);
    readFrom = next(readFrom);
    pending = pending - 1;
    digitalWrite(YELLOW, HIGH);
  }
}

int next(int current) {
  if (current < maxSize) {
    return current + 1;
  } else {
    return 0;
  }
}
