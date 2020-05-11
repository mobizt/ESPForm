
#ifdef ESP32
#include <WiFi.h>
#include "FS.h"
#include <SPIFFS.h>
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

void formElementEventCallback(HTMLElementItem element)
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
  if (element.id == "knob1")
  {
    ESPForm.runScript("$('#knob2').val(" + element.value + ").trigger('change')");
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


#if defined(ESP32)
  SPIFFS.begin(true);
#elif defined(ESP8266)
  SPIFFS.begin();
#endif

  //Element Event Config existed?
  if (!SPIFFS.exists("/knob-test.json"))
  {

    //Add html element event listener, id "knob1" for onchange event
    ESPForm.addElementEventListener("knob1", EVENT_ON_CHANGE);

    /*
    If the id of html elements changed, please update the ElementEventListener config
    */

    //Save notification config
    ESPForm.saveElementEventConfig("/knob-test.json", ESPFormStorage_SPIFFS);
  }
  else
  {
    //Load notification config
    ESPForm.loadElementEventConfig("/knob-test.json", ESPFormStorage_SPIFFS);

    //Test set and read the config value
    HTMLElementItem element;
    
    //Test read
    element = ESPForm.getElementEventConfigItem("knob1");
    if (element.success)
    {
      Serial.println("***********************************");
      Serial.println("Test the Notification config value read for knob1");
      Serial.println("Event: " + ESPForm.getElementEventString(element.event));
      Serial.println("Value: " + element.value);
      Serial.println("***********************************");
    }

    element.id = "knob1";
    element.event = EVENT_ON_CHANGE;
    element.value = "37";

    //Test set the config value
    ESPForm.setElementEventConfigItem(element);


    //Save changes
    ESPForm.saveElementEventConfig("/knob-test.json", ESPFormStorage_SPIFFS);
  }

  //Add the html contents (in html.h) for the web page rendering

  //flash uint8_t array, file name, size of array, gzip compression
  ESPForm.addFileData(index_html_gz, "index.html", sizeof(index_html_gz), true);
  ESPForm.addFileData(main_js_gz, "main.js", sizeof(main_js_gz), true);
  ESPForm.addFileData(jquery_min_js_gz, "jquery.min.js", sizeof(jquery_min_js_gz), true);
  ESPForm.addFileData(jquery_knob_min_js_gz, "jquery.knob.min.js", sizeof(jquery_knob_min_js_gz), true);

  //If AP only or AP + STA mode, set the AP's SSID and Password
  ESPForm.setAP(apSSID.c_str(), apPSW.c_str());

  //Start ESPForm's Webserver
  ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, true);

  ESPForm.startServer();

  Serial.println("***********************************");
  Serial.println("Use web browser and navigate to " + WiFi.localIP().toString());
  Serial.println("Or join the AP " + WiFi.softAPSSID() + " and password " + WiFi.softAPPSK());
  Serial.println("then navigate to " + WiFi.softAPIP().toString());
  Serial.println("***********************************");
  Serial.println();
}

void loop()
{
  //If a client existed
  if (ESPForm.getClientCount() > 0)
  {

    if (millis() - prevMillis > 2 * 60 * 1000)
    {
      prevMillis = millis();

      //Test change the html element value
      //Set the knob value to 50
      ESPForm.setElementContent("knob1", "50");

      //Save config
      ESPForm.saveElementEventConfig("/knob-test.json", ESPFormStorage_SPIFFS);
    }
  }
}
