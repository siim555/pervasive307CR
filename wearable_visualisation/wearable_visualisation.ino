#include <Adafruit_NeoPixel.h>
     
#define ledPIN  6
#define N_LEDS 16

int lightPin = A3;
int tempPin = A0;
int buttonPin = 3;
int lastButtonValue = 1;
// 0: Light Sensor, 1: Temperature Sensor
int visualMode = 1;
int currentLightValue;
float averageLightValue = -1;
// 0: Off, 1: Dim, 2: Mid-Bright, 3: Full
int lightMode = 0;
// Every 20 values = 4 seconds w/ 200 delay. Need to avoid using delay (check timestamps?)
int lightValues[20];
int lightValueIndex = 0;
float averageTempValue = -100.00;
// 0: Blue (Cold), 1: Green (Good), 2: Red (Hot)
int tempMode = -1;
// Every 120 Values = 30 seconds w/ 200 delay.
float tempValues[120];
int tempValueIndex = 0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, ledPIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  pinMode(lightPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  strip.begin();
  chasePermanent(strip.Color(0,0,0));
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  currentLightValue = analogRead(lightPin);
  int buttonValue = digitalRead(buttonPin);
  buttonCheck(buttonValue);
  lightValues[lightValueIndex] = currentLightValue;
  lightValueIndex = lightValueIndex + 1;
  tempValues[tempValueIndex] = calculateTemperature();
  tempValueIndex = tempValueIndex + 1;
  updateBrightness();
  updateTemperature();
  if(averageLightValue == -1 && visualMode == 0){
    chaseLoad(strip.Color(10,10,10));
  }
  if(averageTempValue == -100.00 && visualMode == 1){
    chaseLoadTemperature(strip.Color(1,3,5),strip.Color(0,5,0),strip.Color(5,0,0));
  }
  delay(200);
}

void buttonCheck(int value){
  if(value > lastButtonValue){
    visualMode++;
    if(visualMode > 1){
      // Update back to the last known brightness
      visualMode = 0;
      if(lightMode == 2){
        chasePermanent(strip.Color(20, 20, 20));
      } else if(lightMode == 1){
        chasePermanent(strip.Color(10, 10, 10));
      } else {
        chasePermanent(strip.Color(0, 0, 0));
      }
    } else {
      // Update back to the last temperature
      if(tempMode == 2){
        chasePermanent(strip.Color(1, 3, 5));
      } else if(tempMode == 1){
        chasePermanent(strip.Color(0, 5, 0));
      } else {
        chasePermanent(strip.Color(5, 0, 0));
      }
    }
  }
  lastButtonValue = value;
}

// Function for calculating temperature based on voltage
float calculateTemperature(){
  // Variable to store raw temperature
  long rawTemp;
  // Variable to store voltage calculation
  float voltage;
  // Variable to store Fahrenheit value
  float fahrenheit;
  // Variable to store Celsius value
  float celsius;
  // Read the raw 0-1023 value of temperature into a variable.
  rawTemp = analogRead(tempPin);
  // Calculate the voltage, based on that value.
  // Multiply by maximum voltage (3.3V) and divide by maximum ADC value (1023).
  // If you plan on using this with a LilyPad Simple Arduino on USB power, change to 4.2
  voltage = rawTemp * (4.9 / 1023.0);
  // Calculate the celsius temperature, based on that voltage..
  celsius = (voltage - 0.5) * 100;
  return celsius;
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
    // qsort(values, valuesLength, sizeof(int), sort_desc_int);
    bubbleSortInt(values, 20);
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
    for(int i = 0; i<20; i++){
      Serial.println(values[i]);
      if(values[i] > (median-stdDeviation) && values[i] < (median+stdDeviation)){
        totalLight = totalLight + values[i];
        valuesAdded++;
      }
    }
    toReturn = totalLight / valuesAdded;
  }
  return toReturn;
}

float averageTemp(float values[]){
  float totalTemp = 0;
  float toReturn = 0;
  if(averageTempValue == -100.00){
    for(int i = 0; i<10; i++){
      totalTemp = totalTemp + values[i];
    }
    toReturn = totalTemp / 10;
  } else {
    for(int i = 0; i<120; i++){
      totalTemp = totalTemp + values[i];
    }
    toReturn = totalTemp / 120;
  }
  return toReturn;
}

void updateBrightness(){
  if(lightValueIndex == 20 or (lightValueIndex == 10 and averageLightValue == -1)){
    averageLightValue = averageLight(lightValues);
    lightValueIndex = 0;
    if(averageTempValue != -100.00){
      Serial.println(String(averageTempValue) + "," + String(averageLightValue));
    }
  }
  // This statement will eventually call the Neopixel Fade to fade between brightnesses
  if(averageLightValue != -1){
    // Workout the approximate offset we need; this is to prevent the LEDs being constantly turned on and off
    // Only call the brightness change if there is a change in brightness AND we're displaying light readings
    if(averageLightValue < 10){
      // Pitch Black, ask for full bright
      if(lightMode != 2 and visualMode == 0){
        chasePermanent(strip.Color(20, 20, 20));
      }
      lightMode = 2;
    } else if(averageLightValue < 20){
      if(lightMode != 1 and visualMode == 0){
        chasePermanent(strip.Color(10, 10, 10));
      }
      lightMode = 1;
    } else {
      if(lightMode != 0 and visualMode == 0){
        chasePermanent(strip.Color(0, 0, 0));
      }
      lightMode = 0;
    }
  }
}

void updateTemperature(){
  if(tempValueIndex == 120 or (tempValueIndex == 10 and averageTempValue == -100.00)){
    averageTempValue = averageTemp(tempValues);
    tempValueIndex = 0;
  }
  // This statement will eventually call the Neopixel Fade to fade between brightnesses
  if(averageTempValue != -100.00){
    // Only call the color change if there's a change in color AND we're set to show temperature
    if(averageTempValue < 16.00){
      // Too cold, display blue
      if(tempMode != 2 and visualMode == 1){
        chasePermanent(strip.Color(1, 3, 5));
      }
      tempMode = 2;
    } else if(averageTempValue < 24.00){
      // Just right, display green
      if(tempMode != 1 and visualMode == 1){
        chasePermanent(strip.Color(0, 5, 0));
      }
      tempMode = 1;
    } else {
      // Too hot, display red
      if(tempMode != 0 and visualMode == 1){
        chasePermanent(strip.Color(5, 0, 0));
      }
      tempMode = 0;
    }
  }
}

int sort_desc_int(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

float sort_desc_float(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((float *)cmp1);
  int b = *((float *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

void swapInt(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 

void swapFloat(float *xp, float *yp) 
{ 
    float temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 

// A function to implement bubble sort 
void bubbleSortInt(int arr[], int n) 
{ 
   int i, j; 
   for (i = 0; i < n-1; i++)       
       // Last i elements are already in place    
       for (j = 0; j < n-i-1; j++)  
           if (arr[j] > arr[j+1]) 
              swapInt(&arr[j], &arr[j+1]); 
}

void bubbleSortFloat(float arr[], int n) 
{ 
   int i, j; 
   for (i = 0; i < n-1; i++)       
       // Last i elements are already in place    
       for (j = 0; j < n-i-1; j++)  
           if (arr[j] > arr[j+1]) 
              swapFloat(&arr[j], &arr[j+1]); 
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

static void chaseLoadTemperature(uint32_t c, uint32_t b, uint32_t a) {
      for(uint16_t i=0; i<strip.numPixels()+9; i++) {
          strip.setPixelColor(i  , c);
          strip.setPixelColor(i-3, b);
          strip.setPixelColor(i-6, a);
          strip.setPixelColor(i-9, 0); // Draw new pixel
          strip.show();
          delay(25);
      }
    }

   
