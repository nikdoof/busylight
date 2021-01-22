// Busylight - A web accessible busylight
// (c) 2021 Andrew Williams <andy@tensixtyone.com>
// This code is licensed under MIT license (see LICENSE.txt for details)

#include <config.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#ifdef WEBSERVER
#include <ESP8266WebServer.h>
#endif
#ifdef MQTT
#include <PubSubClient.h>
#endif

#define PWM_MAX_VALUE 1024

struct status
{
  char name[20];
  int r;
  int g;
  int b;
  int blink;
};

struct status statuses[] = {
  {"busy", 255, 0, 0, 0},
  {"available", 0, 255, 0, 0},
  {"away", 150, 255, 0, 0},
  {"ooo", 255, 0, 255, 0},
  {"offline", 0, 0, 0, 0},
  {"blue", 0, 0, 255, 10}
};
#define STATUSES_COUNT 6

//#####################################

WiFiClient wifi;
#ifdef WEBSERVER
ESP8266WebServer server;
#endif
#ifdef MQTT
PubSubClient pubsub(wifi);
#endif
char current_state[20];

int RGB_calc(int value) {
  #if LED_TYPE == 0
  return PWM_MAX_VALUE - ((value / 255.f) * PWM_MAX_VALUE);
  #else
  return (value / 255.f) * PWM_MAX_VALUE;
  #endif
}

void RGB_color(int r, int g, int b)
{
  analogWrite(RED_LED_PIN, RGB_calc(r));
  analogWrite(GREEN_LED_PIN, RGB_calc(g));
  analogWrite(BLUE_LED_PIN, RGB_calc(b));
}

void Switch_Status(struct status status) {
  if (status.blink > 0) {
    bool bstate = false;
    for(int j=0; j<status.blink; j++) {
      if (bstate) {
        RGB_color(status.r, status.g, status.b);
      } else {
        RGB_color(0, 0, 0);
      }
      bstate = !bstate;
      delay(500);
    }
  } else {
    RGB_color(status.r, status.g, status.b);
  }
  strcpy(current_state, status.name);
}

bool Switch_Status(String name) {
  Serial.println("Switching to " + name);
  for(int j=0; j<STATUSES_COUNT; j++) {
    if (String(statuses[j].name) == name) {
      Switch_Status(statuses[j]); 
      return true;
    }
  }
  return false;
}

#ifdef WEBSERVER
void IndexPage() {
  char buffer[1024] = "<html><head><title>Busylight</title></head><body>";
  for(int j=0; j<STATUSES_COUNT; j++) {
    strcat(buffer, "<a href=/");
    strcat(buffer, statuses[j].name);
    strcat(buffer, ">");
    strcat(buffer, statuses[j].name); 
    strcat(buffer, "</a><br/>");
  }
  strcat(buffer, "</body></html>");
  server.send(200, "text/html", buffer);
}

void StatusLookup() {
  if (Switch_Status(server.uri().substring(1))) { 
    server.send(204, "text/plain", "OK");
    return;
  }
  server.send(404, "text/plain", "Missing");
}
#endif

#ifdef MQTT
void Pubsub_Callback(char* topic, byte* payload, unsigned int length) {
}
#endif

void setup()
{
  Serial.begin(9600);
  Serial.println("");
  Serial.println("---");
  
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);

  Serial.println("Config - Pins: (" + String(RED_LED_PIN) + "," + String(GREEN_LED_PIN) + "," + String(BLUE_LED_PIN) + "), PWM: " + String(PWM_MAX_VALUE));
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to " + String(WIFI_SSID));
  while(WiFi.status()!= WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  digitalWrite(LED_BUILTIN, LOW);
  Switch_Status("offline");

#ifdef MQTT
  pubsub.setServer(MQTT_SERVER, MQTT_PORT);
  pubsub.setCallback(Pubsub_Callback);
#endif
#ifdef WEBSERVER
  server.on("/",IndexPage);
  server.on("/health", [](){server.send(200,"text/plain","OK");});
  server.on("/state", [](){server.send(200,"text/plain", current_state);});
  server.onNotFound(StatusLookup);
  server.begin();
#endif
}

void loop()
{
#ifdef MQTT
  Serial.print("Connecting to MQTT ");
  while (!pubsub.connected()) {
    Serial.print(".");
    if (pubsub.connect("Client")) {
      Serial.println(" done.");
    }
    delay(500);
  }
  pubsub.loop();
#endif
#ifdef WEBSERVER
  server.handleClient();
#endif
}
