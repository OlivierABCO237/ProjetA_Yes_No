/*
  IMU Classifier
  This example uses the on-board IMU to start reading acceleration and gyroscope
  data from on-board IMU, once enough samples are read, it then uses a
  TensorFlow Lite (Micro) model to try to classify the movement as a known gesture.
  Note: The direct use of C/C++ pointers, namespaces, and dynamic memory is generally
        discouraged in Arduino examples, and in the future the TensorFlowLite library
        might change to make the sketch simpler.
  The circuit:
  - Arduino Nano 33 BLE or Arduino Nano 33 BLE Sense board.
  Created by Don Coleman, Sandeep Mistry
  Modified by Dominic Pajak, Sandeep Mistry
  This example code is in the public domain.
*/

#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"

const float accelerationThreshold = 2.5; // threshold of significant in G's
const int numSamples = 119;

// Trois jeux de données pour trois questions differentes

int samplesRead = numSamples;
int samplesRead1 = numSamples;
int samplesRead2 = numSamples;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map gesture index to a name
const char* GESTURES[] = {
  "Yes",
  "No"
};

#define NUM_GESTURES (sizeof(GESTURES) / sizeof(GESTURES[0]))

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // initialize the IMU
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print out the samples rates of the IMUs
  /*Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");*/

  Serial.println();

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
}

void loop() {

  // Début du quiz
  
      Serial.println("****Bonjour, et bienvenu dans ce quiz.*****\n Vous répondrez par --yes-- ou par --no-- au questions pausées en faisant le geste associé.\n");
    Serial.println("Question 1 : Pensez vous etre une bonne personne ?");

   
  // Création des variables pour chaque geste après chaque question 

    
  float aX, aY, aZ, gX, gY, gZ;
  float aX1, aY1, aZ1, gX1, gY1, gZ1;
    float aX2, aY2, aZ2, gX2, gY2, gZ2;

    // Initialisation d'un compteur pour chaque question
  int n=0;

  // wait for significant motion
  
  while (samplesRead == numSamples) {
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX, aY, aZ);

      // sum up the absolutes
      float aSum = fabs(aX) + fabs(aY) + fabs(aZ);

      // check if it's above the threshold
      if (aSum >= accelerationThreshold) {
        // reset the sample read count
        samplesRead=0;
        break;
      }
    }
  }

  // check if the all the required samples have been read since
  // the last time the significant motion was detected
  while (samplesRead < numSamples) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX, aY, aZ);
      IMU.readGyroscope(gX, gY, gZ);

      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      tflInputTensor->data.f[samplesRead * 6 + 0] = (aX + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 1] = (aY + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 2] = (aZ + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead * 6 + 3] = (gX + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 4] = (gY + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead * 6 + 5] = (gZ + 2000.0) / 4000.0;

      samplesRead++;

      if (samplesRead == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 6);
          delay(200);
          
        }

        // Geste 1 / question 1
        // Comparaison des données recceuillies, pour savoir si l'utilisateur à mis "yes" ou "no"
        
          if ( tflOutputTensor->data.f[1]<tflOutputTensor->data.f[2]){
          Serial.println("Mauvaise réponse. Vous avec choisi Yes, Pourtant vous etes une mauvaise personne\n");}
        else if (tflOutputTensor->data.f[1]>tflOutputTensor->data.f[2]) {
         Serial.println("Vous avez choisi no. Au moins vous le savez... Vous etes effectivement une mauvaise personne.\n");}

         Serial.println ("Deuxième question: Aimez vous le football ?");

      }
    }
  }
  
  delay(300);
  // Changement de la valeur du compteur
  
  n=1;
  while (samplesRead1 == numSamples && n==1) { // Utilisation de la valeur du compteur pour entrer dans une autre boucle de récupération de données
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX1, aY1, aZ1);

      // sum up the absolutes
      float aSum1 = fabs(aX1) + fabs(aY1) + fabs(aZ1);

      // check if it's above the threshold
      if (aSum1 >= accelerationThreshold) {
        // reset the sample read count
        samplesRead1=0;
        break;
      }
    }
  }
  
    while (samplesRead1 < numSamples && n==1) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX1, aY1, aZ1);
      IMU.readGyroscope(gX1, gY1, gZ1);

      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      // Utilisation des données de la deuxième récupération
      tflInputTensor->data.f[samplesRead1 * 6 + 0] = (aX1 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead1 * 6 + 1] = (aY1 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead1 * 6 + 2] = (aZ1 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead1 * 6 + 3] = (gX1 + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead1 * 6 + 4] = (gY1 + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead1 * 6 + 5] = (gZ1 + 2000.0) / 4000.0;

      samplesRead1++;

      if (samplesRead1 == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 6);
          delay(200);
          
        }
        // Geste 2 / question 2
        // Comparaison des données recceuillies, pour savoir si l'utilisateur à mis "yes" ou "no"
        
          if ( tflOutputTensor->data.f[1]<tflOutputTensor->data.f[2]){
          Serial.println("Bonne réponse. Vous avec choisi Yes, vous etes un peu meilleure qu'avant\n");}
        else if (tflOutputTensor->data.f[1]>tflOutputTensor->data.f[2]) {
         Serial.println("Vous avez choisi non. Vous etes pire que vous le croyez\n");}

         Serial.print("Troisième question: Messi méritait t'il son ballon d'or ??\n");
         
         Serial.println();
}
}
    } 

  delay(300);
  n=2;
  // Nouvelle valeur du compteur

  
  while (samplesRead2 == numSamples && n==2) { // utilisation de cette valeur pour entrer dans la troisième boucle de régulation
    
    if (IMU.accelerationAvailable()) {
      // read the acceleration data
      IMU.readAcceleration(aX2, aY2, aZ2);

      // sum up the absolutes
      float aSum2 = fabs(aX2) + fabs(aY2) + fabs(aZ2);

      // check if it's above the threshold
      if (aSum2 >= accelerationThreshold) {
        // reset the sample read count
        samplesRead2=0;
        break;
      }
    }
  }
  
    while (samplesRead2 < numSamples && n==2) {
    // check if new acceleration AND gyroscope data is available
    if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable()) {
      // read the acceleration and gyroscope data
      IMU.readAcceleration(aX2, aY2, aZ2);
      IMU.readGyroscope(gX2, gY2, gZ2);

      // normalize the IMU data between 0 to 1 and store in the model's
      // input tensor
      tflInputTensor->data.f[samplesRead2 * 6 + 0] = (aX2 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead2 * 6 + 1] = (aY2 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead2 * 6 + 2] = (aZ2 + 4.0) / 8.0;
      tflInputTensor->data.f[samplesRead2 * 6 + 3] = (gX2 + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead2 * 6 + 4] = (gY2 + 2000.0) / 4000.0;
      tflInputTensor->data.f[samplesRead2 * 6 + 5] = (gZ2 + 2000.0) / 4000.0;

      samplesRead2++;

      if (samplesRead2 == numSamples) {
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        if (invokeStatus != kTfLiteOk) {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        // Loop through the output tensor values from the model
        for (int i = 0; i < NUM_GESTURES; i++) {
          Serial.print(GESTURES[i]);
          Serial.print(": ");
          Serial.println(tflOutputTensor->data.f[i], 6);
          delay(200);
          
        }
        // Geste 3 / question 3
        // Comparaison des données recceuillies, pour savoir si l'utilisateur à mis "yes" ou "no"
        
          if ( tflOutputTensor->data.f[1]<0.5){
          Serial.print("Bonne réponse. Vous avec choisi Yes, Soyez béni\n");}
        else if (tflOutputTensor->data.f[1]>0.5) {
         Serial.print("Vous avez choisi non. Vous croupirez en enfer\n");}
         
         Serial.println();
    }
    }
}
  Serial.println("++++++  Merci d'avoir participé à ce quiz.++++++  ");

  // Fin du quiz

}
