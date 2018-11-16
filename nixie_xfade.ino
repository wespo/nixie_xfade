// Create an IntervalTimer object 
IntervalTimer nixieTimer;

#define ANODE0 14
#define ANODE1 15
#define ANODE2 16
#define ANODE3 17

#define DPL 11
#define DPR 10

#define BCD0 23
#define BCD1 22
#define BCD2 21
#define BCD3 20

#define MICROS_PER_DIGIT 400
#define MIN_MICROS 10.0

float digitMicros[8] = {0,0,0,0,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1}; //array to store current number of "on" microseconds (out of MICROS_PER_DIGIT)
float digitTargets[8] = {0,0,0,0,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1,MICROS_PER_DIGIT/1.1}; //array to store target number of "on" microseconds (out of MICROS_PER_DIGIT)
float digitDeltas[8] = {0}; //array to store number of "on" microseconds to change by (out of MICROS_PER_DIGIT)
int digitValues[8] = {10,10,10,10,1,2,3,4}; //array to store the value of each digit, and decimal point.
int DPLValues[8] = {0}; //array to store the value of each digit, and decimal point.
int DPRValues[8] = {0}; //array to store the value of each digit, and decimal point. 


const int anodes[4] = {ANODE0, ANODE1, ANODE2, ANODE3}; //array of anode pins
volatile int anodeIndex = 49; //current anode index
volatile bool ticToc = true;
void setupPins() //set up GPIO pins
{
  pinMode(ANODE0, OUTPUT);
  pinMode(ANODE1, OUTPUT);
  pinMode(ANODE2, OUTPUT);
  pinMode(ANODE3, OUTPUT);
  pinMode(DPL, OUTPUT);
  pinMode(DPR, OUTPUT);
  
  pinMode(BCD0, OUTPUT);
  pinMode(BCD1, OUTPUT);
  pinMode(BCD2, OUTPUT);
  pinMode(BCD3, OUTPUT);
}

void writeValue(int value, int DPLVal, int DPRVal)
{
    digitalWrite(BCD0, value & 0x01);
    digitalWrite(BCD1, value & 0x02);
    digitalWrite(BCD2, value & 0x04);
    digitalWrite(BCD3, value & 0x08);
    digitalWrite(DPL, DPLVal);
    digitalWrite(DPR, DPRVal);
}
void writeDigit()
{
  if(ticToc)
  {
    //write value to this digit
    writeValue(digitValues[anodeIndex],DPLValues[anodeIndex],DPRValues[anodeIndex]);

    //compute necessary time
    if(digitDeltas[anodeIndex] != 0.0)
    {
      digitMicros[anodeIndex] += digitDeltas[anodeIndex]; //fade
      if(anodeIndex < 4) //fade down check
      {
        if(digitMicros[anodeIndex] <= MIN_MICROS)
        {
          digitMicros[anodeIndex] = 0.0;
          digitDeltas[anodeIndex] = 0.0;
        }
      }
      else //fade up check
      {
        if(digitMicros[anodeIndex] >= digitTargets[anodeIndex])
        {
          digitMicros[anodeIndex] = digitTargets[anodeIndex];
          digitDeltas[anodeIndex] = 0.0;
        }
      }
    }
    
    delayMicroseconds(20);
    if((digitMicros[anodeIndex] > MIN_MICROS) && (digitValues[anodeIndex] < 10))
    {
      digitalWrite(anodes[anodeIndex%4], HIGH);
    }
    //stay on for necessary time.
    nixieTimer.begin(writeDigit,int(digitMicros[anodeIndex])); 
    ticToc = false;
  }
  else
  {
    //write "off" to this digit
    //writeValue(anodeIndex%4,10,0,0);
    digitalWrite(anodes[anodeIndex%4], LOW);
    
    //stay off for remaining time
    nixieTimer.begin(writeDigit,MICROS_PER_DIGIT-int(digitMicros[anodeIndex])); 

    //update pointer to next value.
    anodeIndex++;
    anodeIndex%=8;
    ticToc = true;
  } 
  //digitalWrite(13, ticToc);
}

void setup() {
  // put your setup code here, to run once:
  setupPins();
  pinMode(13, OUTPUT);
  Serial.begin(115200);
  Serial.setTimeout(-1);
  nixieTimer.begin(writeDigit,digitMicros[4]);  //set up timer
  Serial.println("Nixie Display (C) W. Esposito 2018. To update display, send via serial: X Y\\n as ASCII text where X is the tube number and Y is the value 0-10 (where 10 is off). To crossfade to the new value send X Y 1\\n.");
}

void updateDisplay(int tubeNum, int newValue, bool dpl, bool dpr, float brightnessVal, float fadeTime)
{
  tubeNum %=4;

  DPLValues[tubeNum+4] = dpl;
  DPRValues[tubeNum+4] = dpr;
  
  digitValues[tubeNum] = digitValues[tubeNum+4];
  digitValues[tubeNum+4] = newValue; //array to store the value of each digit, and decimal point.

  digitMicros[tubeNum] = digitMicros[tubeNum+4];
  digitMicros[tubeNum+4] = MIN_MICROS;

  digitTargets[tubeNum] = 0;
  digitTargets[tubeNum+4] = brightnessVal;
  
  digitDeltas[tubeNum] = -1.0 * digitMicros[tubeNum] * (8.0 * MICROS_PER_DIGIT) / (fadeTime * 1000000.0); //array to store number of "on" microseconds (out of MICROS_PER_DIGIT)
  digitDeltas[tubeNum+4] = digitTargets[tubeNum+4] * (8.0 * MICROS_PER_DIGIT) / (fadeTime * 1000000.0); //array to store number of "on" microseconds (out of MICROS_PER_DIGIT)
}

void newDigitFade(int tubeNum, int newValue, bool dpl, bool dpr)
{
  tubeNum %=4;
  float fadeTime = 0.4;
  float newBrightness = MICROS_PER_DIGIT/1.1;//random(MIN_MICROS, MICROS_PER_DIGIT-MIN_MICROS);
  updateDisplay(tubeNum, newValue, dpl, dpr, newBrightness, fadeTime);
}

void newDigit(int tubeNum, int newValue, bool dpl, bool dpr)
{
  tubeNum %=4;
  DPLValues[tubeNum+4] = dpl;
  DPRValues[tubeNum+4] = dpr;
  digitValues[tubeNum+4] = newValue;
}

//elapsedMillis changeTimer = 10000;
//int tubeCount = 0;
void loop() {
  String inMsg = Serial.readStringUntil('\n');
  int tubeVal = inMsg.substring(0, inMsg.indexOf(" ")).toInt();
  inMsg = inMsg.substring(inMsg.indexOf(" ")).trim();
  String valStr = inMsg.substring(0, inMsg.indexOf(" ")).trim();
  bool dpl = false;
  bool dpr = false;
  if(valStr[0] == '.')
  {
    dpl = true;
    valStr = valStr.substring(1);
  }
  if(valStr[valStr.length()-1] == '.')
  {
    dpr = true;
    valStr = valStr.substring(0, valStr.length()-1);
  }
  int newVal = valStr.toInt();
  inMsg = inMsg.substring(inMsg.indexOf(" ")).trim();
  int fadeFlag = inMsg.substring(0, inMsg.indexOf(" ")).toInt();
  inMsg = inMsg.substring(inMsg.indexOf(" ")).trim();

  if(fadeFlag)
  {
    newDigitFade(tubeVal, newVal, dpl, dpr);
  }
  else
  {
    newDigit(tubeVal, newVal, dpl, dpr);
  }
//  if(changeTimer > 1000)
//  {
//    changeTimer = 0;
//    
//    //mock update
//    
//    
//    int newValue = random(0,15); //array to store the value of each digit, and decimal point.
//    if(random(0,2))
//    {
//      newDigit(tubeCount, newValue);
//    }
//    else
//    {
//      newDigitFade(tubeCount, newValue);
//    }
//
//    tubeCount++;
//    tubeCount %= 4;
//  }
}
