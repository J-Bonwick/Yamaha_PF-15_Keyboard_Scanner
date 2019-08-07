#include <Arduino.h>
#include <digitalWriteFast.h>
#include <Wire.h>

//Clock pin for checking loop frequency
const int clockPin = 7;
//Define note pins
const int NOTE_CS = 48;
const int NOTE_D = 49;
const int NOTE_DS = 46;
const int NOTE_E = 47;
const int NOTE_F = 44;
const int NOTE_FS = 45;
const int NOTE_G = 42;
const int NOTE_GS = 43;
const int NOTE_A = 40;
const int NOTE_AS = 41;
const int NOTE_B = 38;
const int NOTE_C = 39;

//Define Break pins
const int MK0  = 36;
const int MK1  = 34;
const int MK2  = 32;
const int MK3  = 30;
const int MK4  = 28;
const int MK5  = 26;
const int MK6  = 24;
const int MK7  = 22;
//define Make pins
const int BR0 = 37;
const int BR1 = 35;
const int BR2 = 33;
const int BR3 = 31;
const int BR4 = 29;
const int BR5 = 27;
const int BR6 = 25;
const int BR7 = 23;

const int BRArray[8] = {BR0, BR1, BR2, BR3, BR4, BR5, BR6, BR7};
const int MKArray[8] = {MK0, MK1, MK2, MK3, MK4, MK5, MK6, MK7};
int BRValue[8];
int MKValue[8];

uint32_t keyTime[88];
int keyStatus[88]; //0 = TOP, 1 = Going down, 2 = bottom, 3 = going up

uint32_t deBounce[88];//Used for preventing false trigger on release

int totalKey;//Key postion between 0 and 88
int block;//Block number

void blockScan(int, bool);
void midiSend(int, int, int);

void setup() {
  Serial.begin(115200); //DEBUG
  Serial1.begin(31250); //Midi
  Serial2.begin(31250);

  Wire.begin();//Join I2C bus

  //Set pinModeFasts
  pinModeFast(NOTE_D, OUTPUT);
  pinModeFast(NOTE_CS, OUTPUT);
  pinModeFast(NOTE_E, OUTPUT);
  pinModeFast(NOTE_DS, OUTPUT);
  pinModeFast(NOTE_FS, OUTPUT);
  pinModeFast(NOTE_F, OUTPUT);
  pinModeFast(NOTE_GS, OUTPUT);
  pinModeFast(NOTE_G, OUTPUT);
  pinModeFast(NOTE_AS, OUTPUT);
  pinModeFast(NOTE_A, OUTPUT);
  pinModeFast(NOTE_C, OUTPUT);
  pinModeFast(NOTE_B, OUTPUT);

  digitalWriteFast(NOTE_D, HIGH);
  digitalWriteFast(NOTE_CS, HIGH);
  digitalWriteFast(NOTE_E, HIGH);
  digitalWriteFast(NOTE_DS, HIGH);
  digitalWriteFast(NOTE_FS, HIGH);
  digitalWriteFast(NOTE_F, HIGH);
  digitalWriteFast(NOTE_GS, HIGH);
  digitalWriteFast(NOTE_G, HIGH);
  digitalWriteFast(NOTE_AS, HIGH);
  digitalWriteFast(NOTE_A, HIGH);
  digitalWriteFast(NOTE_C, HIGH);
  digitalWriteFast(NOTE_B, HIGH);

  pinModeFast(BR0, INPUT);
  pinModeFast(BR1, INPUT);
  pinModeFast(BR2, INPUT);
  pinModeFast(BR3, INPUT);
  pinModeFast(BR4, INPUT);
  pinModeFast(BR5, INPUT);
  pinModeFast(BR6, INPUT);
  pinModeFast(BR7, INPUT);

  pinModeFast(MK0, INPUT);
  pinModeFast(MK1, INPUT);
  pinModeFast(MK2, INPUT);
  pinModeFast(MK3, INPUT);
  pinModeFast(MK4, INPUT);
  pinModeFast(MK5, INPUT);
  pinModeFast(MK6, INPUT);
  pinModeFast(MK7, INPUT);

  pinModeFast(clockPin, OUTPUT);
  //Fill array
  for(int i = 0; i < 88; i++){
    keyStatus[i] = 0;
  }
}

void loop() {
  //Scan block 0 which consists of four keys
  digitalWriteFast(NOTE_A, LOW);
  blockScan(0, true);
  digitalWriteFast(NOTE_A, HIGH);
  digitalWriteFast(NOTE_AS, LOW);
  blockScan(1, true);
  digitalWriteFast(NOTE_AS, HIGH);
  digitalWriteFast(NOTE_B, LOW);
  blockScan(2, true);
  digitalWriteFast(NOTE_B, HIGH);
  digitalWriteFast(NOTE_C, LOW);
  blockScan(3, true);
  digitalWriteFast(NOTE_C, HIGH);
  digitalWriteFast(NOTE_CS, LOW);
  blockScan(4, false);
  digitalWriteFast(NOTE_CS, HIGH);
  digitalWriteFast(NOTE_D, LOW);
  blockScan(5, false);
  digitalWriteFast(NOTE_D, HIGH);
  digitalWriteFast(NOTE_DS, LOW);
  blockScan(6, false);
  digitalWriteFast(NOTE_DS, HIGH);
  digitalWriteFast(NOTE_E, LOW);
  blockScan(7, false);
  digitalWriteFast(NOTE_E, HIGH);
  digitalWriteFast(NOTE_F, LOW);
  blockScan(8, false);
  digitalWriteFast(NOTE_F, HIGH);
  digitalWriteFast(NOTE_FS, LOW);
  blockScan(9, false);
  digitalWriteFast(NOTE_FS, HIGH);
  digitalWriteFast(NOTE_G, LOW);
  blockScan(10, false);
  digitalWriteFast(NOTE_G, HIGH);
  digitalWriteFast(NOTE_GS, LOW);
  blockScan(11, false);
  digitalWriteFast(NOTE_GS, HIGH);

  digitalWriteFast(clockPin, HIGH);
  digitalWriteFast(clockPin,LOW);
}

void blockScan(int key, bool firstBlock){
  //Read IO into array
  for(int i = 0; i < 8; i++){
    BRValue[i] = digitalReadFast(BRArray[i]);
    MKValue[i] = digitalReadFast(MKArray[i]);
  }
  if(firstBlock){
    block = 0;
  }
  else{
    block = 1;
  }
  for(; block < 8; block++){
    if(firstBlock){
      totalKey = key + block * 12;
    }
    else{
      totalKey = key + (block - 1) * 12;
    }
    //LOGIC STUFF HERE
    if((keyStatus[totalKey] == 0) && BRValue[block] && MKValue[block]){
      keyTime[totalKey] = micros();
      keyStatus[totalKey] = 1;
    }
    if((keyStatus[totalKey] == 1) && !BRValue[block]){
      keyStatus[totalKey] = 0;
    }
    if((keyStatus[totalKey] == 1) && !MKValue[block]){
      uint32_t time = micros() - keyTime[totalKey];
      if(time > 200000){
        midiSend(0x90, totalKey, 0);
      }
      else{
        midiSend(0x90, totalKey, map(time, 3500, 200000, 127, 1));
      }
      keyStatus[totalKey] = 2;
    }
    if((keyStatus[totalKey] == 2) && BRValue[block] && MKValue[block]){
      deBounce[totalKey] = micros();
      keyStatus[totalKey] =3;
    }
    if((keyStatus[totalKey] == 3) && !MKValue[block]){
      keyStatus[totalKey] = 2;
    }
    if((keyStatus[totalKey] == 3) && ((micros() - deBounce[totalKey]) > 10000)){
      midiSend(0x90, totalKey, 0x00);
      keyStatus[totalKey] = 4;
    }
    if((keyStatus[totalKey] == 4) && !BRValue[block]){
      keyStatus[totalKey] = 0;
    }
  }
}

void midiSend(int cmd, int pitch, int velocity){
  Serial1.write(cmd);
  Serial1.write(pitch + 21);
  Serial1.write(velocity);

  Wire.beginTransmission(8);
  Wire.write(pitch +21);
  Wire.write(velocity);
  Wire.endTransmission();
}