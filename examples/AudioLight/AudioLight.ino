
#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <ESPForm.h>

//For HTML content
#include "html.h"

//Your WiFi SSID and Password
#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

const int pwm_freq = 5000;
const int resolution = 10;
#if defined(ESP32)
const int led_channel = 0;
const int led_pin = 5;
#elif defined(ESP8266)
const int led_pin = 2;
#endif

unsigned long serverTimeout = 2 * 60 * 1000;

void formElementEventCallback(ESPFormClass::HTMLElementItem element)
{
    if (element.id == "audio1")
#if defined(ESP32)
        ledcWrite(led_channel, 1024 - element.value.toInt());
#elif defined(ESP8266)
        analogWrite(led_pin, 1024 - element.value.toInt());
#endif
}
void serverTimeoutCallback()
{

    //If server timeout (no client connected within specific time)
    Serial.println("***********************************");
    Serial.println("Server Timeout");
    Serial.println("***********************************");
    Serial.println();
}

void setup()
{

    Serial.begin(115200);
    Serial.println();

    Serial.printf("ESPForm v%s\n\n", ESPFORM_VERSION);

#if defined(ESP32)
    ledcSetup(led_channel, pwm_freq, resolution);
    ledcAttachPin(led_pin, led_channel);
#elif defined(ESP8266)
    pinMode(led_pin, OUTPUT);
    analogWriteRange(1023);
    analogWriteResolution(resolution);
    analogWriteFreq(pwm_freq);
#endif

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.persistent(false);

    //Set WiFi mode to STA (create ESP own network)
    WiFi.mode(WIFI_STA);

    //For STA only or AP + STA mode
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    //Add the html contents (in html.h) for the web page rendering

    //flash uint8_t array, file name, size of array, gzip compression
    ESPForm.addFileData(index_html_gz, "index.html", sizeof(index_html_gz), true);
    ESPForm.addFileData(main_js_gz, "main.js", sizeof(main_js_gz), true);

    //Start ESPForm's Webserver
    ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, false);

    ESPForm.startServer();

    Serial.println("***********************************");
    Serial.println("Use web browser and navigate to " + WiFi.localIP().toString());
    Serial.println("***********************************");
    Serial.println();
}

void loop()
{
}