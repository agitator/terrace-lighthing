#include <FastLED.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define LED_PIN     21
#define CLOCK_PIN     19
#define NUM_LEDS    10
#define BRIGHTNESS  64
#define LED_TYPE    APA102
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

int currentProgram = -1;
int newProgram = 0;

WebServer server(80);
StaticJsonDocument<1024> doc;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println(String("terrace-light-") + getMac());
  WiFi.setHostname((String("terrace-light-") + getMac()).c_str());
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
    int c = strtol(colorValues.c_str(), 0, 16);
    Serial.println(c);
    int r = (c & 0xff0000) >> 16;
    int g = (c & 0x00ff00) << 8 ;
    int b = (c & 0x0000ff) << 8;

    leds[index++] = b|g|r;
  }
  server.send(200);
}

// void checkClients() {
//   String header = "";
//   WiFiClient client = server.available();

//   if (client) {
//     Serial.println("Client connected");
//     String currentLine = "";

//     while (client.connected()) {
//       if (client.available()) {
//         char c = client.read();
//         Serial.write(c);
//         header += c;
//         if (c == '\n') {
//           if (currentLine.length() == 0) {


//             if (header.indexOf("POST /api?program=") >= 0) {
//               int valueStart = header.indexOf('=');
//               int valueEnd = header.indexOf("HTTP");
//               String value = header.substring(valueStart + 1, valueEnd - 1);
//               int _program = atoi(value.c_str());
//               newProgram = _program;
//               Serial.println(_program);

//             }

//             sendHtmlHeader(client);

//             client.println();
//             break;

//           } else {
//             currentLine = "";
//           }

//         } else if (c != '\r') {
//           currentLine += c;
//         }
//       }
//     }
//     client.stop();
//     Serial.println("Client disconnected");
//   }
// }


// void sendHtmlHeader(WiFiClient &client) {
//   client.println("HTTP/1.1 200 OK");
//   client.println("Content-type:text/html");
//   client.println("Connection: close");
//   client.println();
// }

String byteToHex(byte b) {
  char output[3];

  output[0] = (b >> 4) + 0x30;
  if (output[0] > 0x39) output[0] += 7;

  output[1] = (b & 0x0f) + 0x30;
  if (output[1] > 0x39) output[1] += 7;

  output[2] = 0;
  return String(output);
}

String getMac() {
  byte mac[6];
  WiFi.macAddress(mac);
  // return byteToHex(mac[0]) +
  //        byteToHex(mac[1]) +
  //        byteToHex(mac[2]) +
  //        byteToHex(mac[3]) +
  //        byteToHex(mac[4]) +
  //        byteToHex(mac[5]);
  return byteToHex(mac[5]);
}