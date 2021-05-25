
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

//The AP
String apSSID = "ESPForm";
String apPSW = "12345678";

unsigned long prevMillis = 0;
unsigned long serverTimeout = 2 * 60 * 1000;
float timezone = 3;

void formElementEventCallback(ESPFormClass::HTMLElementItem element)
{
  Serial.println();
  Serial.println("***********************************");
  Serial.println("id: " + element.id);
  Serial.println("value: " + element.value);
  Serial.println("type: " + element.type);
  Serial.println("event: " + ESPForm.getElementEventString(element.event));
  Serial.println("Free heap: " + String(ESP.getFreeHeap()));
  Serial.println("***********************************");
  Serial.println();

  //If first knob value changed
  if (element.id == "clear-btn")
  {
    ESPForm.runScript("clearData()");
  }
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

  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);
  WiFi.persistent(false);

  //Set WiFi mode to STA (create ESP own network) or AP + STA (create ESP own network and join the WiFi network)
  WiFi.mode(WIFI_AP_STA);

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

  //generate seed numbers
  randomSeed(analogRead(0));

  //Set the system time via NTP server
  Serial.println("***********************************");
  Serial.println("Set up time...");
  ESPForm.setClock(timezone);
  Serial.println("***********************************");
  Serial.println();

  //FLASH_FS is defined in ESPFormFS.h
#if defined(ESP32)
  FLASH_FS.begin(true);
#elif defined(ESP8266)
  FLASH_FS.begin();
#endif

  //Element Event Config existed?
  if (!FLASH_FS.exists("/chart-test.json"))
  {

    //Add html element event listener, id "knob1" for onchange event
    ESPForm.addElementEventListener("clear-btn", ESPFormClass::EVENT_ON_CLICK);

    /*
    If the id of html elements changed, please update the ElementEventListener config
    */

    //Save notification config
    ESPForm.saveElementEventConfig("/chart-test.json", esp_form_storage_flash);
  }
  else
  {
    //Load notification config
    ESPForm.loadElementEventConfig("/chart-test.json", esp_form_storage_flash);
  }

  //Add the html contents (in html.h) for the web page rendering

  //flash uint8_t array, file name, size of array, gzip compression
  ESPForm.addFileData(index_html_gz, "index.html", sizeof(index_html_gz), true);
  ESPForm.addFileData(main_js_gz, "main.js", sizeof(main_js_gz), true);
  ESPForm.addFileData(highcharts_js_gz, "highcharts.js", sizeof(highcharts_js_gz), true);

  //If AP only or AP + STA mode, set the AP's SSID and Password
  ESPForm.setAP(apSSID.c_str(), apPSW.c_str());

  //Start ESPForm's Webserver
  ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, true);

  ESPForm.startServer();

  Serial.println();
  Serial.println("***********************************");
  Serial.println("Use web browser and navigate to " + WiFi.localIP().toString());

  Serial.println("Or join the AP " + apSSID + " and password " + apPSW);
  Serial.println("then navigate to " + WiFi.softAPIP().toString());
  Serial.println("***********************************");
  Serial.println();
}

void loop()
{
  //If a client existed
  if (ESPForm.getClientCount() > 0)
  {

    if (millis() - prevMillis > 1000)
    {
      prevMillis = millis();

      float minf = 20.0f;
      float maxf = 40.0f;

      float value = minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);
      time_t now = time(nullptr);

      ESPForm.runScript("addData(" + String(now) + "," + String(value) + ")");
    }
  }
}
