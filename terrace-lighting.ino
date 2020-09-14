#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define LED_PIN     21
#define CLOCK_PIN     19
#define NUM_LEDS    10
#define BRIGHTNESS  64
#define LED_TYPE    APA102
#define COLOR_ORDER GRB // maybe if we change this to BGR we can use CRGB directly again (another branch for testing)
CRGB leds[NUM_LEDS];

int currentProgram = -1;
int newProgram = 0;

WebServer server(80);
StaticJsonDocument<1024> doc;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  String hostname = String("terrace-light-") + getMac(1);
  Serial.println(hostname);
  WiFi.setHostname(hostname.c_str());
  WiFi.setSleep(false);
  WiFi.begin("express", "lets1get2wireless3");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handle_Request);
  server.on("/", HTTP_POST, handle_RequestPost);
  server.begin();

  delay(3000); // power-up safety delay
  FastLED.addLeds<LED_TYPE, LED_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  setupPacifica();


}


void loop()

{
  // checkClients();
  server.handleClient();

  if (currentProgram != newProgram)
  {
    currentProgram = newProgram;
    // switch(currentProgram) {
    //   case 0:
    //     setupPacifica();
    //     break;
    //   case 1:
    //     setupColorPalette();
    //     break;
    //   }
  }

  // switch(currentProgram) {
  //   case 0:
  //     loopPacifica();
  //     break;
  //   case 1:
  //     loopColorPalette();
  //     break;
  //   }
  FastLED.show();
}

void handle_Request() {

  String message = "";

  for (int i = 0; i < server.args(); i++)
  {

    message += "Arg nº" + (String)i + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
  }
  Serial.println(message);
  server.send(204);
}

void handle_RequestPost() {

  String message = "";

  for (int i = 0; i < server.args(); i++)
  {

    message += "Arg nº" + String(i) + " –> ";
    message += server.argName(i) + ": ";
    message += server.arg(i) + "\n";
  }
  Serial.println(message);

  DeserializationError error = deserializeJson(doc, server.arg(0));

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  float speed = doc["speed"];
  Serial.println(speed);
  JsonArray array = doc["colors"].as<JsonArray>();
  Serial.println(array.size());

  int index = 0;

  for (JsonVariant v : array)
  {
    String colorValues = v.as<String>();
    int color = webcolorToInt(colorValues);
    Serial.println(color);
    leds[index++] = convertRGBtoGBR(color);
  }
  server.send(200);
}

int webcolorToInt(String webcolor)
{
    return strtol(webcolor.c_str(), 0, 16);
}

/**
 * cleaning up the function a little (even if its a bit slower)
 * we should be able to replace this with fastLeds CRGB, but I need hardware
 * to test that :)
 */
int convertRGBtoGBR (int input) 
{
    int r = (input >> 16) & 0xFF;
    int g = (input >>  8) & 0xFF;
    int b = (input >>  0) & 0xFF;
    return b<<16 | g<<8 | r;
}

String byteToHex(byte b) {
  char output[3];

  output[0] = (b >> 4) + 0x30;
  if (output[0] > 0x39) output[0] += 7;

  output[1] = (b & 0x0f) + 0x30;
  if (output[1] > 0x39) output[1] += 7;

  output[2] = 0;
  return String(output);
}

String getMac(int length) {
  byte mac[6];
  WiFi.macAddress(mac);
  String s = "";
  
  for(int i=0; i<length; i++) {
    s = byteToHex(mac[5-i]) + s;
  }
  return s;
}
