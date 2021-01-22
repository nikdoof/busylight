// Busylight - A web accessible busylight
// (c) 2021 Andrew Williams <andy@tensixtyone.com>
// This code is licensed under MIT license (see LICENSE for details)

#include <config.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <statuses.h>

WiFiClient wifi;
WiFiClientSecure wifi_secure;
char current_state[20];

#ifdef ENABLE_WEBSERVER
#include <ESP8266WebServer.h>
ESP8266WebServer server;
#endif

#ifdef ENABLE_MQTT
#include <PubSubClient.h>
PubSubClient pubsub(wifi);
#endif

#ifdef ENABLE_TELEGRAM
#include <UniversalTelegramBot.h>

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, wifi_secure);
const unsigned long BOT_MTBS = 1000;
unsigned long telegram_bot_last_time;
#endif

// Calculates the PWM value for the RGB LED, taking into account Anode/Cathode
int RGB_calc(int value) {
  #if LED_TYPE == 0
  return PWM_MAX_VALUE - ((value / 255.f) * PWM_MAX_VALUE);
  #else
  return (value / 255.f) * PWM_MAX_VALUE;
  #endif
}

// Writes out the RBG colours to the pins
void RGB_color(int r, int g, int b)
{
  analogWrite(RED_LED_PIN, RGB_calc(r));
  analogWrite(GREEN_LED_PIN, RGB_calc(g));
  analogWrite(BLUE_LED_PIN, RGB_calc(b));
}

// Switches status based on a status struct provided
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

// Switches status based on a name of a status struct
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

#ifdef ENABLE_WEBSERVER
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

#ifdef ENABLE_MQTT
void Pubsub_Callback(char* topic, byte* payload, unsigned int length) {
}
#endif

#ifdef ENABLE_TELEGRAM
void Telegram_Message(int msg_count) {

  // Iterate through the waiting messages
  for (int i = 0; i < msg_count; i++) {
    String text = bot.messages[i].text;
    String chat_id = bot.messages[i].chat_id;

    if (text == "/start") {
      String buffer = "Hi, i'm Busylight, you can control me with one of the following commands\n\n";
      for(int j=0; j<STATUSES_COUNT; j++) {
        buffer += "/";
        buffer += statuses[j].name;
        buffer += "\n";
      }
      bot.sendMessage(chat_id, buffer);
    } else if (!text.charAt(0) == '/') {
      bot.sendMessage(chat_id, "Hi, i'm Busylight, to see my commands try /start");
    } else {
      bool res = Switch_Status(text.substring(1));
      if (!res) {
        bot.sendMessage(chat_id, "Sorry, " + text.substring(1) + " is a invalid status");
      }
    }
  }
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

#ifdef ENABLE_MQTT
  pubsub.setServer(MQTT_SERVER, MQTT_PORT);
  pubsub.setCallback(Pubsub_Callback);
#endif

#ifdef ENABLE_WEBSERVER
  server.on("/",IndexPage);
  server.on("/health", [](){server.send(200,"text/plain","OK");});
  server.on("/state", [](){server.send(200,"text/plain", current_state);});
  server.onNotFound(StatusLookup);
  server.begin();
#endif

#ifdef ENABLE_TELEGRAM
// Add api.telegram.org root cert
wifi_secure.setTrustAnchors(&cert);
wifi_secure.setInsecure();
#endif
}

void loop()
{
#ifdef ENABLE_MQTT
  if (!pubsub.connected()) {
    Serial.print("Connecting to MQTT ");
    while (!pubsub.connected()) {
      Serial.print(".");
      if (pubsub.connect("Client")) {
        Serial.println(" done.");
      }
      delay(500);
    }
  }
  pubsub.loop();
#endif

#ifdef ENABLE_TELEGRAM
  if (millis() - telegram_bot_last_time > BOT_MTBS) {
    int msg_count = bot.getUpdates(bot.last_message_received + 1);
    while (msg_count) {
      Serial.println("Recevied Telegram Message");
      Telegram_Message(msg_count);
      msg_count = bot.getUpdates(bot.last_message_received + 1);
    }
    telegram_bot_last_time = millis();
  }
#endif

#ifdef ENABLE_WEBSERVER
  server.handleClient();
#endif
}
