#include <Arduino_APDS9960.h>

//number of measurements per condition
const int numSamples = 50;
int iterate = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);

  // check if the APDS sensor is working
  if (!APDS.begin()) {
    Serial.println("Error initializing APDS9960 sensor!");
    while(1);
  }

  // print the header of the csv file
  Serial.println("proximity,shade,brightness");
}

void loop() {
  // put your main code here, to run repeatedly
  // float proximity, shade, brightness;
    // proximity: range = [0, 255]
    if(APDS.proximityAvailable()) {
      // add the measurement id
      // read the proximity value
      int proximity = APDS.readProximity();
      Serial.print(proximity);
      Serial.print(',');
    }
      
    // shade: range ... & brightness: range = [0, 255]
    if (APDS.colorAvailable()){
      int r,g,b,a;
      APDS.readColor(r, g, b, a);
      int rgb = 65536*r + 256*g + b;
      Serial.print(rgb);
      Serial.print(',');
      Serial.println(a);

      // increase the measurement id
      iterate++;
      if (iterate > numSamples){
        iterate = 0;
        Serial.println();
        delay(12000);
      }
    }

   
    delay(140);
  
  
}
