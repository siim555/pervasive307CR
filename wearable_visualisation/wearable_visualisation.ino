#include <Adafruit_NeoPixel.h>
     
#define ledPIN  6
#define N_LEDS 16

int sensorPin = A3;
int buttonPin = 3;
int lastButtonValue = 1;
// 0: Light Sensor, 1: Temperature Sensor
int visualMode = 0;
int currentLightValue;
float averageLightValue = -1;
// 0: Off, 1: Dim, 2: Mid-Bright, 3: Full
int lightMode = 0;
// Every 20 values = 5 seconds w/ 500 delay. Need to avoid using delay (check timestamps?)
int lightValues[20];
int lightValueIndex = 0;
// Conductive thread is significantly resistant and increases the voltage, so we have to consider
// this when reading the voltage values. Do not take the offset off; instead add it to the 
// boundaries
int resistanceOffset = 16;
int ledOffset = 0;
     
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, ledPIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(sensorPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  strip.begin();
  chasePermanent(strip.Color(0,0,0));
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  currentLightValue = analogRead(sensorPin);
  int buttonValue = digitalRead(buttonPin);
  buttonCheck(buttonValue);
  lightValues[lightValueIndex] = currentLightValue;
  lightValueIndex = lightValueIndex + 1;
  updateBrightness();
  if(averageLightValue == -1){
      chaseLoad(strip.Color(10,10,10));
  }
  if(visualMode == 1){
    ledOffset = 0;
    chaseLoad(strip.Color(0,0,5));
  }
  delay(200);
}

void buttonCheck(int value){
  if(value > lastButtonValue){
    visualMode++;
    if(visualMode > 1){
      visualMode = 0;
    }
  }
  lastButtonValue = value;
}

float averageLight(int values[]){
  int totalLight = 0;
  float toReturn = 0;
  if(averageLightValue == -1){
    for(int i = 0; i<10; i++){
      totalLight = totalLight + values[i];
    }
    toReturn = totalLight / 10;
  } else {
    // We're presuming the values are on a bell curve, so drop the bottom and top 25% (outliers)
    int valuesLength = sizeof(values) / sizeof(values[0]);
    float initialMean = 0;
    // qsort - last parameter is a function pointer to the sort function
    qsort(values, valuesLength, sizeof(values[0]), sort_desc);
    // Get initial mean
    for(int i = 0; i<20; i++){
      initialMean = initialMean + values[i];
    }
    initialMean = initialMean / 20;
    // (x - mean)^2
    float varianceVals[20];
    for(int i = 0; i<20; i++){
      varianceVals[i] = pow((values[i] - initialMean),2);
    }
    // Get Standard Deviation
    float stdDeviation = 0;
    for(int i = 0; i<20; i++){
      stdDeviation = stdDeviation + varianceVals[i];
    }
    stdDeviation = pow((stdDeviation / 20), 0.5);
    // Only add values for mean that are < 1 deviation from the median
    float median = values[9];
    int valuesAdded = 0;
    float totalLightWithout = 0;
    float totalLightBellCurve = 0;
    for(int i = 0; i<20; i++){
      if(values[i] > (median-stdDeviation) && values[i] < (median+stdDeviation)){
        totalLight = totalLight + values[i];
        valuesAdded++;
      }
      if(i>4 && i<16){
        totalLightBellCurve = totalLightBellCurve + values[i];
      }
      totalLightWithout = totalLightWithout + values[i];
    }
    toReturn = totalLight / valuesAdded;
    totalLightWithout = totalLightWithout / 20;
    totalLightBellCurve = totalLightBellCurve / 10;
    toReturn = totalLightBellCurve;
  }
  return toReturn;
}

void updateBrightness(){
  if(lightValueIndex == 20 or (lightValueIndex == 10 and averageLightValue == -1)){
    averageLightValue = averageLight(lightValues);
    Serial.println("0," + String(averageLightValue));
    lightValueIndex = 0;
    // This statement will eventually call the Neopixel Fade to fade between brightnesses
    if(visualMode == 0){
      if(averageLightValue < (10+resistanceOffset+ledOffset)){
        // Pitch Black, ask for full bright
        if(lightMode != 2){
          lightMode = 2;
          ledOffset = 70;
          chasePermanent(strip.Color(20, 20, 20));
        }
      } else if(averageLightValue < (20+resistanceOffset+ledOffset)){
        if(lightMode != 1){
          lightMode = 1;
          ledOffset = 35;
          chasePermanent(strip.Color(10, 10, 10));
        }
      } else {
        if(lightMode != 0){
          lightMode = 0;
          ledOffset = 0;
          chasePermanent(strip.Color(0, 0, 0));
        }
      }
    }
  }
}

int sort_desc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

static void chasePermanent(uint32_t c) {
      for(uint16_t i=0; i<strip.numPixels()+4; i++) {
          strip.setPixelColor(i  , c); // Draw new pixel
          strip.show();
          delay(25);
      }
    }

static void chaseLoad(uint32_t c) {
      for(uint16_t i=0; i<strip.numPixels()+4; i++) {
          strip.setPixelColor(i  , c);
          strip.setPixelColor(i-4, 0); // Draw new pixel
          strip.show();
          delay(25);
      }
    }

   
