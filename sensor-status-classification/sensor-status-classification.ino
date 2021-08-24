#include <Arduino_APDS9960.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"

//number of measurements per condition
const int numSamples = 50;
int iterate = 0;

// Decalre global variables used for TensorFlow Lite for microcontrollers
tflite::MicroErrorReporter tflErrorReporter;
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer which will experimentally be adjusted based on the used model
constexpr int tensorArenaSize = 3*1024;
byte tensorArena[tensorArenaSize]__attribute__((aligned(16)));

// Map the states
const char* STATES[] = {
  "empty",
  "picked",
  "stocked"
};

#define NUM_OF_STATES (sizeof(STATES) / sizeof(STATES[0]))

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  // check if the APDS sensor is working
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
    while(1);
  }

  // append the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // interpretor to run the model and allocate memory
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);
  tflInterpreter->AllocateTensors();

  // get pointers for the model's input and outputs
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  Serial.println("Model is ready to collect input!");
}

void loop() {
  // put your main code here, to run repeatedly
  // float proximity, shade, brightness;
    // proximity: range = [0, 255]
    if(APDS.proximityAvailable()) {
      // add the measurement id
      // read the proximity value
      int proximity = APDS.readProximity();
      // normalize proximity data and store it in the model's input tensor
      tflInputTensor->data.f[iterate*3 + 0] = proximity/255.0; 
    }
      
    // shade: range ... & brightness: range = [0, 255]
    if (APDS.colorAvailable()){
      int r,g,b,a;
      APDS.readColor(r, g, b, a);
      int rgb = 65536*r + 256*g + b;
      
      // normalize RGB, and A data and store it in the model's input tensor
      tflInputTensor->data.f[iterate*3 + 1] = rgb/269553921.0;
      tflInputTensor->data.f[iterate*3 + 2] = a/4097.0; 
      
      // increase the measurement index
      iterate++;
      if (iterate > numSamples){
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }
        for (int i = 0; i < NUM_OF_STATES; i++) {
          Serial.print(STATES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 3);
        }
        Serial.println();
        iterate = 0;
        delay(900);
      }
    }
    delay(10);
}
