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



#if defined(ESP32)
WiFiMulti wifiMulti;
#elif defined(ESP8266)
ESP8266WiFiMulti wifiMulti;
#endif

//The config file contains HTML elements and events
String configFileName = "/wifi.json";

//The AP
String apSSID = "ESPForm";
String apPSW = "12345678";

//Timeout for WiFi and AP connection
unsigned long wifiTimeout = 60 * 1000;
unsigned long serverTimeout = 2 * 60 * 1000;

//Function constructors
bool startWiFi();
bool loadConfig();
void deleteConfigFile(const String &filename);
bool loadWiFiConfig(const String &filename);
void setupESPForm();
void addNewFormElementEvents();
bool addStaticIP();
bool addAP();
void scanWiFi();
void formElementEventCallback(HTMLElementItem element);
void serverTimeoutCallback();

void setup()
{

    Serial.begin(115200);
    Serial.println();

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.persistent(false);

    setupESPForm();

    if (loadConfig())
    {
        if (!startWiFi())
        {
            Serial.println("MAIN:  Start server");
            ESPForm.startServer();
        }
    }
    else
    {
        Serial.println("MAIN:  Start server");
        ESPForm.startServer();
    }
}

void loop()
{
}

bool startWiFi()
{

    //WiFi data is ready then start connction
    WiFi.mode(WIFI_STA);
    Serial.print("MAIN:  Connecting to Wi-Fi..");

#if defined(ESP32)
    wifiMulti.run();
#elif defined(ESP8266)
    bool timedout = false;
    unsigned long pm = millis();
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
        timedout = millis() - pm > wifiTimeout;
        if (timedout)
            break;
    }
#endif

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println();
        Serial.print("MAIN:  Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();
    }
    else
    {
        Serial.println("MAIN:  WiFi connection failed!");
        return false;
    }

    return true;
}

bool loadConfig()
{

    ESPForm.stopServer();

    //Reset the webserver
    Serial.println("MAIN:  Load configuration");

    //Load config from file. If it does not existed,
    //then create the new one.
    if (!loadWiFiConfig(configFileName))
        return false;

    //Get WiFi data (SSID and PSW) from config
    if (!addAP())
        return false;

    //Get the static IP from config
    addStaticIP();

    return true;
}

void deleteConfigFile(const String &filename)
{
    Serial.println("MAIN:  Delete config file...");

#if defined(ESP32)
    SPIFFS.begin(true);
#elif defined(ESP8266)
    SPIFFS.begin();
#endif

    if (SPIFFS.exists(filename))
        SPIFFS.remove(filename);

    Serial.println("MAIN:  Success");
    Serial.println();
}

bool loadWiFiConfig(const String &filename)
{

    Serial.print("MAIN:  Load WiFi config...");

#if defined(ESP32)
    SPIFFS.begin(true);
#elif defined(ESP8266)
    SPIFFS.begin();
#endif

    if (!SPIFFS.exists(filename))
    {
        Serial.println(" file is not existed");
        //Add new config.
        addNewFormElementEvents();
        return false;
    }

    //If config existed, load the HTML elements and events
    ESPForm.loadElementEventConfig(filename, ESPFormStorage_SPIFFS);
    Serial.println(" success");
    return true;
}

void setupESPForm()
{
    Serial.println("MAIN:  Setup ESPForm");

    ESPForm.setAP(apSSID.c_str(), apPSW.c_str());

    //Prepare html contents (in html.h) for the web page rendering (only once)

    //SPIFFS's char array, file name
    ESPForm.addFileData(index_html, "index.html");
    ESPForm.addFileData(main_js, "main.js");

    //SPIFFS's uint8_t array, file name, size of array, gzip compression
    ESPForm.addFileData(bootstrap_css, "bootstrap.min.css", sizeof(bootstrap_css), true);
    ESPForm.addFileData(signs_png, "signs.png", sizeof(signs_png), false);
    ESPForm.addFileData(shield_png, "shield.png", sizeof(shield_png), false);
    ESPForm.addFileData(wifi0_png, "wifi0.png", sizeof(wifi0_png), false);
    ESPForm.addFileData(wifi25_png, "wifi25.png", sizeof(wifi25_png), false);
    ESPForm.addFileData(wifi50_png, "wifi50.png", sizeof(wifi50_png), false);
    ESPForm.addFileData(wifi75_png, "wifi75.png", sizeof(wifi75_png), false);

    //Can also add Flash or SD file
    //ESPForm.addFile("reload.png", "/reload.png", ESPFormStorage_SPIFFS);

    ESPForm.begin(formElementEventCallback, serverTimeoutCallback, serverTimeout, true);
}

void addNewFormElementEvents()
{

    Serial.println("MAIN:  Add new HTML Form Element Event Listener");

    //Add these html form elements (in index.html)
    ESPForm.addElementEventListener("ssid1", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("psw1", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("ssid2", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("psw2", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("ssid3", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("psw3", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("sta-static", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("sta-ip", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("sta-subnet", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("sta-gateway", EVENT_ON_CHANGE);
    ESPForm.addElementEventListener("scan-wifi", EVENT_ON_CLICK);
    ESPForm.addElementEventListener("save-btn", EVENT_ON_CLICK);
    ESPForm.addElementEventListener("boot-btn", EVENT_ON_CLICK);

    ESPForm.saveElementEventConfig("/wifi.json", ESPFormStorage_SPIFFS);
}

bool addStaticIP()
{

    IPAddress ip, subnet, gateway;

    HTMLElementItem element = ESPForm.getElementEventConfigItem("sta-static");

    if (!element.success || element.value != "true")
        return false;

    element = ESPForm.getElementEventConfigItem("sta-ip");
    if (!element.success)
        return false;

    if (!ip.fromString(element.value))
        return false;

    element = ESPForm.getElementEventConfigItem("sta-subnet");
    if (!element.success)
        return false;

    if (!subnet.fromString(element.value))
        return false;

    element = ESPForm.getElementEventConfigItem("sta-gateway");
    if (!element.success)
        return false;

    if (!gateway.fromString(element.value))
        return false;

    WiFi.config(ip, gateway, subnet);
    return true;
}

bool addAP()
{

    Serial.print("MAIN:  Add AP...");

    String ssid, psw;
    bool ap1, ap2, ap3;

    HTMLElementItem element = ESPForm.getElementEventConfigItem("ssid1");
    element.value.trim();
    ap1 = element.success && element.value != "null" && element.value.length() > 0;
    ssid = element.value;

    if (ap1)
    {
        element = ESPForm.getElementEventConfigItem("psw1");
        element.value.trim();
        ap1 &= element.success && element.value != "null" && element.value.length() > 0;
        psw = element.value;

        if (ap1)
            wifiMulti.addAP(ssid.c_str(), psw.c_str());
    }

    element = ESPForm.getElementEventConfigItem("ssid2");
    element.value.trim();
    ap2 = element.success && element.value != "null" && element.value.length() > 0;
    ssid = element.value;

    if (ap2)
    {
        element = ESPForm.getElementEventConfigItem("psw2");
        element.value.trim();
        ap2 &= element.success && element.value != "null" && element.value.length() > 0;
        psw = element.value;

        if (ap2)
            wifiMulti.addAP(ssid.c_str(), psw.c_str());
    }

    element = ESPForm.getElementEventConfigItem("ssid3");
    element.value.trim();
    ap3 = element.success && element.value != "null" && element.value.length() > 0;
    ssid = element.value;

    if (ap3)
    {
        element = ESPForm.getElementEventConfigItem("psw3");
        element.value.trim();
        ap2 &= element.success && element.value != "null" && element.value.length() > 0;
        psw = element.value;

        if (ap3)
            wifiMulti.addAP(ssid.c_str(), psw.c_str());
    }
    if (ap1 || ap2 || ap3)
        Serial.println(" success");
    else
        Serial.println(" failed, no value set");

    return ap1 || ap2 || ap3;
}

void scanWiFi()
{
    //Send the javascript code to run in the web browser
    //Hide the WiFi List by calling showWiFiList function in main.js
    ESPForm.runScript("showWiFiList(false);");
    WiFiInfo list;
    ESPForm.scanWiFi(list, 20);

    //Construct the nesting array of WiFi data to run in javascript function
    String arr = "[";
    for (size_t i = 0; i < list.count(); i++)
    {
        if (i > 0)
            arr += ",";
        arr += "['" + list.getInfo(i).ssid + "'," + String(list.getInfo(i).channel) + "," + String(list.getInfo(i).encType) + "," + String(list.getInfo(i).quality) + "]";
    }
    arr += "]";
    list.clear();

    //Send the javascript code to run in the web browser
    //Create the WiFi List by calling setWiFiList function in main.js
    //The arr string is in form of nesting array in javascript
    //Function setWiFiList in main.js takes this array to create the tale for WiFi list
    ESPForm.runScript("setWiFiList(" + arr + ");");

    //Send the javascript code to run in the web browser
    //Show the WiFi List by calling showWiFiList function in main.js
    ESPForm.runScript("showWiFiList(true);");
}

//The websocket event callback to handle the message sent by client (web browser)
void formElementEventCallback(HTMLElementItem element)
{

    //If scan-wifi button clicked
    if (element.id == "scan-wifi")
        scanWiFi();

    //If save-btn button clicked
    else if (element.id == "save-btn")
        ESPForm.saveElementEventConfig("/wifi.json", ESPFormStorage_SPIFFS);

    //If boot-btn button clicked
    else if (element.id == "boot-btn")
        ESP.restart();
}

void serverTimeoutCallback()
{

    //If server timeout (no client connected within specific time)

    Serial.println("MAIN:  Server Timeout");
    Serial.println("***********************************");
    Serial.println();

    //Load the config, try to reconnect wifi if WiFi config avalailable, start the server again if WiFi is not connected

    if (loadConfig())
    {
        if (!startWiFi())
        {
            Serial.println("MAIN:  Start server");
            ESPForm.startServer();
        }
    }
    else
    {
        Serial.println("MAIN:  Start server");
        ESPForm.startServer();
    }
}