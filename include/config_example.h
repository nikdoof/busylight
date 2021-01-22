// Type of RGB LED, adjusts the brightness calculations.
#define LED_TYPE 0 // 0 = Anode, 1 = Cathode

// GPIO pins for each colour
#define RED_LED_PIN 4
#define GREEN_LED_PIN 0
#define BLUE_LED_PIN 2

// Defines how many gradients a PWM pin has on this device, for ESP devices
// its 1024, on other systems its 255. Adjust as needed if you don't get
// accurate colour representation.
#define PWM_MAX_VALUE 1024

// WIFI connection details
#define WIFI_SSID "ssid"
#define WIFI_PASS "s3cur3"

// Enables the webserver
#define WEBSERVER

// Enable MQTT connection (not complete)
//#define ENABLE_MQTT
#define MQTT_SERVER "mqtt.local"
#define MQTT_PORT 1883

// Enable Telegram connection
//#define ENABLE_TELEGRAM
#define TELEGRAM_BOT_TOKEN "xxx:xxx"


