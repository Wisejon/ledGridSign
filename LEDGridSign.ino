#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>
// this file will include settings that should not be committed
#include "customSettings.h"


ESP8266WebServer server(80);   //Web server object. Will be listening in port 80 (default for HTTP)

#define NUMPIXELS 100 // Popular NeoPixel ring size
int steps = 5; // set number of steps between colors
int stepDelay = 1000; // delay between steps

int brightness = 64; //set brighhtness

const int minC = 0; //minimum color
const int maxC = 255; // maximum color
const int np = NUMPIXELS; //number of pixels in array
const int columns = 13; // number of values stored in each pixel array.

int valueArray[np][columns];

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        5

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int displayType = 0; //set displayType
// 0: No Colors
// 1: Random - Red Only
// 2: Random - Green Only
// 3: Random - Blue Only
// 3: Random - RBG

String INDEX_HTML = "";

void setup() {
  Serial.begin(115200);
  // wifi settings should be set in your customSettings.h file
  WiFi.begin(wifiName, wifiPassword); //Connect to the WiFi network
  
  while (WiFi.status() != WL_CONNECTED) { //Wait for connection
    delay(500);
    Serial.println("Waiting to connect…");
  }
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //Print the local IP to access the server
  Serial.println(WiFi.macAddress());  //Print the mac address

  //Associate the client handler paths
  server.on("/", handleRoot); 

  // Start the server
  server.begin();                                       
  Serial.println("Server listening"); 

  //Generate Base Pixels for Display
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  // set the pixels to 0
  clearScreen();

  delay(1000);

  // set brightness
  pixels.setBrightness(brightness);

  //update status page html
  updateStatusPage(displayType,brightness,steps,stepDelay);

  for(int i = 0; i < np; i++){
    // starting pixel colors
    int rgb[3];
    generateColors(rgb);
    valueArray[i][0] = rgb[0]; //red 1 value
    valueArray[i][1] = rgb[1]; //green 1 value
    valueArray[i][2] = rgb[2]; //blue 1 value
    
    // destination pixel colors
    generateColors(rgb);
    valueArray[i][3] = rgb[0]; //red 2 value
    valueArray[i][4] = rgb[1]; //green 2 value
    valueArray[i][5] = rgb[2]; //blue 2 value 

    // current pixel colors
    valueArray[i][6] = valueArray[i][0]; //red 3 value
    valueArray[i][7] = valueArray[i][1]; //green 3 value
    valueArray[i][8] = valueArray[i][2]; //blue 3 value 

    // delta per step
    valueArray[i][9] = calcDelta(valueArray[i][0],valueArray[i][3],steps);
    valueArray[i][10] = calcDelta(valueArray[i][1],valueArray[i][4],steps);;
    valueArray[i][11] = calcDelta(valueArray[i][2],valueArray[i][5],steps);;

    // Steps between starting and destination pixels
    valueArray[i][12] = steps;

    // //set color for every pixel (3rd set of pixel colors)
    pixels.setPixelColor(i, pixels.Color(valueArray[i][6], valueArray[i][7], valueArray[i][8]));
  }
  // //turn on pixels
  pixels.show();   // Send the updated pixel colors to the hardware.
  delay(stepDelay);

}

void loop(){
  server.handleClient();
  adjustColors(valueArray);
} 

void updateStatusPage(int dt, int b, int s, int sd){

  String d = "<option value=\"0\">No Display</option>";
  if(dt == 0){
    d = "<option value=\"0\" selected>No Display</option>"
    "<option value=\"1\">Red Only</option>" 
    "<option value=\"2\">Green Only</option>" 
    "<option value=\"3\">Blue Only</option>" 
    "<option value=\"4\">Random Colors</option>";
  } else if (dt == 1){
    d = "<option value=\"0\" >No Display</option>"
    "<option value=\"1\" selected>Red Only</option>" 
    "<option value=\"2\">Green Only</option>" 
    "<option value=\"3\">Blue Only</option>" 
    "<option value=\"4\">Random Colors</option>";
  } else if (dt == 2){
    d = "<option value=\"0\" >No Display</option>"
    "<option value=\"1\" >Red Only</option>" 
    "<option value=\"2\" selected>Green Only</option>" 
    "<option value=\"3\">Blue Only</option>" 
    "<option value=\"4\">Random Colors</option>";
  } else if (dt == 3){
    d = "<option value=\"0\" >No Display</option>"
    "<option value=\"1\" >Red Only</option>" 
    "<option value=\"2\">Green Only</option>" 
    "<option value=\"3\" selected>Blue Only</option>" 
    "<option value=\"4\">Random Colors</option>";
  } else if (dt == 4){
    d = "<option value=\"0\" >No Display</option>"
    "<option value=\"1\" >Red Only</option>" 
    "<option value=\"2\">Green Only</option>" 
    "<option value=\"3\">Blue Only</option>" 
    "<option value=\"4\" selected>Random Colors</option>";
  }
  
  INDEX_HTML = "<!DOCTYPE HTML>" 
  "<html>" 
  "<head>" 
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">" 
  "<title>Neopixel Sign</title>"
  "<!-- CSS only -->"
"<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta1/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-giJF6kkoqNQ00vy+HMDP7azOuL0xtbfIcaT9wjKHr8RbDVddVHyTfAAsrekwKmP1\" crossorigin=\"anonymous\">"
"<!-- JavaScript Bundle with Popper -->"
"<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.0.0-beta1/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-ygbV9kiqUc6oa4msXn9868pTtWMgiQaeYH7/t7LECLbyPA2x65Kgf80OJFdroafW\" crossorigin=\"anonymous\"></script>"
 
//  "<style>" 
//  "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"" 
//  "</style>" 
  "</head>" 
  "<body>"
  "<div class=\"container\">" 
    "<h1>NeoPixel Sign</h1>" 
    "<FORM action=\"/\" method=\"post\">" 
    "<P>"
    "<label for=\"routine\">Display Type</label>" 
    "<select name=\"routine\" class=\"form-select\">" 
    + d +
    "</select class=\"form-select\"><BR>"
    //set brightness  
    "<label for=\"brightness\" class=\"form-label\">Brightness</label> <input class=\"form-range\" type=\"range\" name=\"brightness\" min=\"0\" max=\"255\" value=\"" + String(b) + "\">"
    "<BR>" 
    //stepDelay
    "<label for=\"steps\" class=\"form-label\">Steps</label> <input class=\"form-range\" type=\"range\" name=\"steps\" min=\"0\" max=\"20\" value=\"" + String(s) + "\">"
    //stepDelay
    "<label for=\"stepDelay\" class=\"form-label\">Step Delay (milliseconds)</label> <input class=\"form-range\" type=\"range\" name=\"stepDelay\" min=\"50\" max=\"1000\" steps=\"50\" value=\"" + String(sd) + "\">"
    "<button type=\"submit\" class=\"btn btn-primary\">Submit</button>"
    "&nbsp;<button type=\"reset\" class=\"btn btn-warning\">Reset</button>"
    "</P>" 
    "</FORM>"
  "</div>"
  "</body>" 
  "</html>";
  
}


void handleRoot()
{
  if (server.args() > 0) {
    Serial.println("hit handler");
    Serial.println( server.args() );
    // grab the arguments and parse them
    String message = "Number of args received:";
    message += server.args();            //Get number of parameters
    message += "\n";                            //Add a new line
  
    for (int i = 0; i < server.args(); i++) {
      message += "Arg #" + (String)i + " --> ";   //Include the current iteration value
      message += server.argName(i) + ": ";     //Get the name of the parameter
      message += server.arg(i) + "\n";              //Get the value of the parameter
    } 
      
    Serial.println(message);
    
    handleSubmit();
  }
  else {
    Serial.println("updated brightness is: " + String(brightness));
    //update status page html
    updateStatusPage(displayType,brightness,steps,stepDelay);
    server.send(200, "text/html", INDEX_HTML);
  }
  
  
}

void returnFail(String msg)
{
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
}

void handleSubmit()
{
  int routine;
  Serial.println(server.arg("routine").toInt());
  // if (!server.hasArg("routine")) return returnFail("BAD ARGS");
  
  routine = server.arg("routine").toInt();
  // set routine
  if (routine == 0) {
    displayType = 0;
  } else if (routine == 1) {
    displayType = 1;
  } else if (routine == 2) {
    displayType = 2;
  } else if (routine == 3) {
    displayType = 3;
  } else if (routine == 4) {
    displayType = 4;
  }
  //set brightness
  if(server.hasArg("brightness")){
    brightness = server.arg("brightness").toInt();
    Serial.println("update brightness to: " + String(brightness));   
  }

  //set steps
  if(server.hasArg("steps")){
    steps = server.arg("steps").toInt();
    Serial.println("update steps to: " + String(steps));   
  }

  //set step delay
  if(server.hasArg("stepDelay")){
    stepDelay = server.arg("stepDelay").toInt();
    Serial.println("update step Delay to: " + String(stepDelay));   
  }
  
  
  clearScreen();
  adjustColors(valueArray);
  server.send(200, "text/html", "<h1>SUBMITTED DATA</h1><br><a href=\"/\">Return to Home Page</a>");
}

void clearScreen(){
  for(int i = 0; i < np; i++){
    pixels.setPixelColor(i,0,0,0);
  }
  pixels.show();   // Send the updated pixel colors to the hardware.
}

void adjustColors( int a[][columns] ){
  for(int i = 0; i < np; i++){

    // Adjust pixel colors and adjust to max if trying to go over
    int tRed = overLimitCheck(a[i][3], a[i][6], a[i][9]);
    int tGreen = overLimitCheck(a[i][4], a[i][7], a[i][10]);
    int tBlue = overLimitCheck(a[i][5], a[i][8], a[i][11]);

    valueArray[i][6] = tRed; // current Red Value
    valueArray[i][7] = tGreen; // current Green Value
    valueArray[i][8] = tBlue; // current Blue Value

    // //set color for every pixel (3rd set of pixel colors)
    pixels.setPixelColor(i, pixels.Color(valueArray[i][6], valueArray[i][7], valueArray[i][8]));

    // iterate step count down
    valueArray[i][12] = valueArray[i][12] - 1;

    //check if steps are zero and then
      if(valueArray[i][12] == 0){
        // generate new values and set steps back to 0
  
        // move dest to starting place 
        valueArray[i][0] = valueArray[i][3];
        valueArray[i][1] = valueArray[i][4];
        valueArray[i][2] = valueArray[i][5];
  
        // and generate new destination led points
        int rgb[3];
        generateColors(rgb);
        valueArray[i][3] = rgb[0];
        valueArray[i][4] = rgb[1];
        valueArray[i][5] = rgb[2];
  
        // generate new delta per step
        valueArray[i][9] = calcDelta(valueArray[i][0],valueArray[i][3],steps);
        valueArray[i][10] = calcDelta(valueArray[i][1],valueArray[i][4],steps);
        valueArray[i][11] = calcDelta(valueArray[i][2],valueArray[i][5],steps);
  
        valueArray[i][12] = steps;
      }
  }
  // turn on pixels
  pixels.setBrightness(brightness);
  pixels.show();   // Send the updated pixel colors to the hardware.
  delay(stepDelay);
}

int overLimitCheck(int dVal,int cVal, int delta){
  int tVal = cVal + delta;
  if(delta < 0){
      if (tVal < dVal){
        tVal = dVal;
      }
    } else if (delta > 0) {
      if (tVal > dVal){
        tVal = dVal;
    }
  }
  return tVal;
}

//generate the colors based on teh set display type
void generateColors(int colors[]){
  if( displayType == 0){
    colors[0] = 0;
    colors[1] = 0;
    colors[2] = 0;
  } else if(displayType == 1){ //red
    colors[0] = random(minC, maxC);
    colors[1] = 0;
    colors[2] = 0;
  } else if(displayType == 2){ //green
    colors[0] = 0;
    colors[1] = random(minC, maxC);
    colors[2] = 0;
  }else if(displayType == 3){ //blue
    colors[0] = 0;
    colors[1] = 0;
    colors[2] = random(minC, maxC);
  } else if(displayType == 4){ //random
    colors[0] = random(minC, maxC);
    colors[1] = random(minC, maxC);
    colors[2] = random(minC, maxC);
  } 
}

//calculate the delta for color changes
int calcDelta(int val1, int val2, int s){
    int delta = val2 - val1;
    int stepAmt = delta / s;
    int stepMod = delta % s;
    
    if(delta == 0){
        return delta;
    } else {
      if (delta > 0){
        if(stepMod){
          stepAmt = stepAmt + 1;
        }
        return stepAmt;
      } else if (delta < 0){
        if(stepMod){
          stepAmt = stepAmt - 1;
        }
        return stepAmt;
      }
    }
}
