/*
 * The ESPForm for Arduino v 1.0.1
 * 
 * May 11, 2020
 * 
 * The simple HTML Form Elements data interchange library for ESP32/ESP8266 through the Webserver.
 * 
 * This allows user to send and receive the HTML form elements to/from device (ESP32/ESP8266).
 * 
 * The supported HTML Form Elements are input, select, option, textarea, radio, checkbox and button.
 * 
 * Thesse HTML form elements can add the eventlistener to send the data based on events to the device (ESP32/ESP8266).
 * 
 * The supported devices are Espressif's ESP32 and ESP8266 MCUs.
 * 
 * 
 * This library based on the Wrbsocket library from Markus Sattler with some modification to proper working with BearSSL WiFi client for ESP8266.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2020 K. Suwatchai (Mobizt)
 * 
 * Copyright (c) 2015 Markus Sattler. All rights reserved.
 * 
 * 
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <Arduino.h>

#ifndef ESPFormClass_CPP
#define ESPFormClass_CPP

#include "ESPForm.h"

ESPFormClass::ESPFormClass()
{
    _debug = false;
    _sdOk = false;
    _idle_to._serverStarted = false;
    _apStarted = false;
#if defined(ESP32)
    _sdConfigSet = false;
    _index = -1;
    _xTaskHandle = NULL;
#elif defined(ESP8266)
    _sdPin = 15;
#endif
}

ESPFormClass::~ESPFormClass()
{
    terminateServer();
}

void ESPFormClass::terminateServer()
{
    stopServer();
    if (_config)
    {
        _config->clear();
        _config.reset();
        _config = nullptr;
    }
    if (_webSocketPtr)
    {
        _webSocketPtr.reset();
        _webSocketPtr = nullptr;
        _webServerPtr.reset();
        _webServerPtr = nullptr;
    }
    _file_info.clear();
#if defined(ESP8266)
    if (_dnsServerPtr)
    {
        _dnsServerPtr.reset();
        _dnsServerPtr = nullptr;
    }
#endif
}

size_t ESPFormClass::getFileCount()
{
    return _file_info.size();
}

void ESPFormClass::deleteAllFiles()
{
    _file_info.clear();
}

void ESPFormClass::addFileData(PGM_P content, const char *fileName)
{
    file_content_info_t f;
    f.content = content;
    f.name = fileName;
    f.gzip = false;
    f.len = strlen_P(content);
    _file_info.push_back(f);
}

void ESPFormClass::addFileData(const uint8_t *content, const char *fileName, size_t length, bool gzip)
{
    file_content_info_t f;
    f.content = reinterpret_cast<const char *>(content);
    f.len = length;
    f.gzip = gzip;
    f.name = fileName;
    _file_info.push_back(f);
}

void ESPFormClass::addFile(const char *fileName, const char *filePath, ESPFormStorageType storagetype)
{
    file_content_info_t f;
    f.content = NULL;
    f.name = fileName;
    f.path = filePath;
    f.storageType = storagetype;
    _file_info.push_back(f);
}

void ESPFormClass::setIP(IPAddress local_ip, IPAddress gateway, IPAddress subnet)
{
    _ipConfig = true;
    _ip = local_ip;
    _gateway = gateway;
    _subnet = subnet;
}

void ESPFormClass::setAP(const char *ssid, const char *psw, int channel, int ssid_hidden, int max_connection)
{
    _apStarted = false;
    _apSSID = ssid;
    _apPSW = psw;
    _channel = channel;
    _ssid_hidden = ssid_hidden;
    _max_connection = max_connection;

    _skipSelfAP = false;
    WiFiInfo t;
    scanWiFi(t, 10);
    for (size_t i = 0; i < t.count(); i++)
    {
        if (strcmp(t.getInfo(i).ssid.c_str(), ssid) == 0)
        {

#if defined(ESP32)
            uint64_t chipId = ESP.getEfuseMac();
            char *_buf = newPtr(23);
            snprintf(_buf, 23, "%s-%04X%08X", _apSSID.c_str(), (uint16_t)(chipId >> 32), (uint32_t)chipId);
            _apSSID = _buf;
            delPtr(_buf);
#elif defined(ESP8266)
            uint32_t chipId = ESP.getChipId();
            char *_buf = newPtr(23);
            snprintf(_buf, 23, "%s-%04X", _apSSID.c_str(), chipId);
            _apSSID = _buf;
            delPtr(_buf);
#endif

            break;
        }
    }
    t.clear();
    _skipSelfAP = true;
}

void ESPFormClass::begin(ElementEventCallback eventCallback, IdleTimeoutCallback timeoutCallback, unsigned long timeout, bool debug)
{

    _debug = debug;
    _elementEventCallback = std::move(eventCallback);
    _idle_to._idleTimeoutCallback = std::move(timeoutCallback);
    _idle_to._idleTimeTimeout = timeout;
    _webSocketPtr.reset(new WebSocketsServer(webSocketPort));
#if defined(ESP32)
    _webServerPtr.reset(new WebServer(webServerPort));
#elif defined(ESP8266)
    _webServerPtr.reset(new ESP8266WebServer(webServerPort));
#endif
    WiFi.setAutoReconnect(true);
}

void ESPFormClass::addElementEventListener(const String &id, ESPFormEventType event, const char *defaultValue)
{
    prepareConfig();

    ESPJson json;
    std::string s;
    p_memCopy(s, ESPFORM_STR_16);
    json.add(s.c_str(), id);
    p_memCopy(s, ESPFORM_STR_17, true);
    json.add(s.c_str(), (int)event);
    p_memCopy(s, ESPFORM_STR_18, true);
    if (defaultValue != NULL)
        json.add(s.c_str(), defaultValue);
    else
        json.add(s.c_str());
    _config->add(json);
}

void ESPFormClass::saveElementEventConfig(const String &fileName, ESPFormStorageType storagetype)
{
    prepareConfig();

    if (_config->size() == 0)
        return;

    ESPJson json;
    std::string s;
    p_memCopy(s, ESPFORM_STR_23);
    json.add(s.c_str(), *_config);
    String js;
    json.toString(js, false);

#ifdef ESP32
    if (storagetype == ESPFormStorage_SPIFFS)
    {
        SPIFFS.begin(true);
        file = SPIFFS.open(fileName, "w");
        file.print(js);
        file.close();
    }
    else if (storagetype == ESPFormStorage_SD)
    {
        if (!_sdOk)
            _sdOk = sdTest();
        if (_sdOk)
        {
            file = SD.open(fileName, FILE_WRITE);
            file.print(js);
            file.close();
        }
    }
#elif defined(ESP8266)
    if (storagetype == ESPFormStorage_SPIFFS)
    {
        SPIFFS.begin();
        _file = SPIFFS.open(fileName, "w");
        _file.print(js);
        _file.close();
    }
    else if (storagetype == ESPFormStorage_SD)
    {
        if (!_sdOk)
            _sdOk = sdTest();
        if (_sdOk)
        {
            file = SD.open(fileName, FILE_WRITE);
            file.print(js);
            file.close();
        }
    }
#endif
}

void ESPFormClass::loadElementEventConfig(const String &fileName, ESPFormStorageType storagetype)
{
    ESPJson js;

#ifdef ESP32
    if (storagetype == ESPFormStorage_SPIFFS)
    {
        SPIFFS.begin(true);
        file = SPIFFS.open(fileName, "r");
        js.setJsonData(file.readString());
        file.close();
    }
    else if (storagetype == ESPFormStorage_SD)
    {
        if (!_sdOk)
            _sdOk = sdTest();
        if (_sdOk)
        {
            file = SD.open(fileName, FILE_READ);
            js.setJsonData(file.readString());
            file.close();
        }
    }
#elif defined(ESP8266)
    if (storagetype == ESPFormStorage_SPIFFS)
    {
        SPIFFS.begin();
        _file = SPIFFS.open(fileName, "r");
        js.setJsonData(_file.readString());
        _file.close();
    }
    else if (storagetype == ESPFormStorage_SD)
    {
        if (!_sdOk)
            _sdOk = sdTest();
        if (_sdOk)
        {
            file = SD.open(fileName, FILE_READ);
            js.setJsonData(file.readString());
            file.close();
        }
    }
#endif

    ESPJsonData d;
    std::string s;
    p_memCopy(s, ESPFORM_STR_13);
    p_memCopy(s, ESPFORM_STR_23);
    js.get(d, s.c_str());
    if (d.success)
    {
        _config.reset();
        _config = nullptr;
        _config = std::shared_ptr<ESPJsonArray>(new ESPJsonArray());
        d.getArray(*_config);
    }
}

HTMLElementItem ESPFormClass::getElementEventConfigItem(const String &id)
{

    ESPJsonData jsonData;
    bool res = false;
    std::string s;
    HTMLElementItem element;

    prepareConfig();

    for (size_t k = 0; k < _config->size(); k++)
    {
        getPath(0, k, s);
        _config->get(jsonData, s.c_str());
        if (jsonData.success)
        {

            if (id == jsonData.stringValue)
            {
                element.id = id;
                res = true;
                getPath(1, k, s);
                _config->get(jsonData, s.c_str());
                if (jsonData.success)
                {
                    res &= true;
                    element.event = (ESPFormEventType)jsonData.intValue;
                }
                getPath(2, k, s);
                _config->get(jsonData, s.c_str());
                if (jsonData.success)
                {
                    res &= true;
                    element.value = jsonData.stringValue;
                }
                element.success = res;
                break;
            }
        }
    }
    return element;
}

void ESPFormClass::setElementEventConfigItem(HTMLElementItem &element)
{

    ESPJsonData jsonData;
    std::string s;

    prepareConfig();

    for (size_t k = 0; k < _config->size(); k++)
    {
        getPath(0, k, s);
        _config->get(jsonData, s.c_str());
        if (jsonData.success)
        {
            if (element.id == jsonData.stringValue)
            {
                if (element.event > 0)
                {
                    getPath(1, k, s);
                    _config->set(s.c_str(), (int)element.event);
                }

                getPath(2, k, s);
                _config->set(s.c_str(), element.value);
                break;
            }
        }
    }
}

void ESPFormClass::removeElementEventConfigItem(const String &id)
{
    ESPJsonData jsonData;
    std::string s;

    prepareConfig();

    for (size_t k = 0; k < _config->size(); k++)
    {
        getPath(0, k, s);
        _config->get(jsonData, s.c_str());
        if (jsonData.success)
        {
            if (id == jsonData.stringValue)
            {
                _config->remove(k);
                break;
            }
        }
    }
}

void ESPFormClass::clearElementEventConfig()
{
    prepareConfig();
    _config->clear();
}

void ESPFormClass::prepareConfig()
{
    if (!_config)
        _config = std::shared_ptr<ESPJsonArray>(new ESPJsonArray());
}

String ESPFormClass::getElementEventString(ESPFormEventType event)
{
    std::string buf;
    switch (event)
    {
    case EVENT_UNDEFINED:
        p_memCopy(buf, ESPFORM_STR_39);
        break;
    case EVENT_ON_CLICK:
        p_memCopy(buf, ESPFORM_STR_40);
        break;
    case EVENT_ON_DBLCLICK:
        p_memCopy(buf, ESPFORM_STR_41);
        break;
    case EVENT_ON_MOUSEDOWN:
        p_memCopy(buf, ESPFORM_STR_42);
        break;
    case EVENT_ON_MOUSEMOVE:
        p_memCopy(buf, ESPFORM_STR_43);
        break;
    case EVENT_ON_MOUSEOUT:
        p_memCopy(buf, ESPFORM_STR_44);
        break;
    case EVENT_ON_MOUSEOVER:
        p_memCopy(buf, ESPFORM_STR_45);
        break;
    case EVENT_ON_MOUSEUP:
        p_memCopy(buf, ESPFORM_STR_46);
        break;
    case EVENT_ON_MOUSEWHEEL:
        p_memCopy(buf, ESPFORM_STR_47);
        break;
    case EVENT_ON_WHEEL:
        p_memCopy(buf, ESPFORM_STR_48);
        break;
    case EVENT_ON_KEYDOWN:
        p_memCopy(buf, ESPFORM_STR_49);
        break;
    case EVENT_ON_KEYPRESS:
        p_memCopy(buf, ESPFORM_STR_50);
        break;
    case EVENT_ON_KEYUP:
        p_memCopy(buf, ESPFORM_STR_51);
        break;
    case EVENT_ON_CHANGE:
        p_memCopy(buf, ESPFORM_STR_52);
        break;
    case EVENT_ON_SUBMIT:
        p_memCopy(buf, ESPFORM_STR_53);
        break;
    case EVENT_ON_INPUT:
        p_memCopy(buf, ESPFORM_STR_54);
        break;
    case EVENT_ON_FOCUS:
        p_memCopy(buf, ESPFORM_STR_55);
        break;
    case EVENT_ON_CONTEXTMENU:
        p_memCopy(buf, ESPFORM_STR_56);
        break;
    case EVENT_ON_SELECT:
        p_memCopy(buf, ESPFORM_STR_57);
        break;
    case EVENT_ON_SEARCH:
        p_memCopy(buf, ESPFORM_STR_58);
        break;
    case EVENT_ON_RESET:
        p_memCopy(buf, ESPFORM_STR_59);
        break;
    case EVENT_ON_INVALID:
        p_memCopy(buf, ESPFORM_STR_60);
        break;

    default:
        break;
    }
    return buf.c_str();
}

String ESPFormClass::getWiFiEncrytionTypeString(EncriptionType encType)
{
    std::string buf;

#if defined(ESP32)
    switch (encType)
    {
    case WIFI_AUTH_WEP:
        p_memCopy(buf, ESPFORM_STR_63);
        break;
    case WIFI_AUTH_WPA_PSK:
        p_memCopy(buf, ESPFORM_STR_68);
        break;
    case WIFI_AUTH_WPA2_PSK:
        p_memCopy(buf, ESPFORM_STR_69);
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        p_memCopy(buf, ESPFORM_STR_70);
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        p_memCopy(buf, ESPFORM_STR_71);
        break;
    case WIFI_AUTH_MAX:
        p_memCopy(buf, ESPFORM_STR_62);
        break;
    default:
        break;
    }
#elif defined(ESP8266)
    switch (encType)
    {
    case ENC_TYPE_WEP:
        p_memCopy(buf, ESPFORM_STR_63);
        break;
    case ENC_TYPE_TKIP:
        p_memCopy(buf, ESPFORM_STR_64);
        break;
    case ENC_TYPE_CCMP:
        p_memCopy(buf, ESPFORM_STR_65);
        break;
    case ENC_TYPE_NONE:
        p_memCopy(buf, ESPFORM_STR_66);
        break;
    case ENC_TYPE_AUTO:
        p_memCopy(buf, ESPFORM_STR_67);
        break;
    default:
        break;
    }
#endif

    return buf.c_str();
}

void ESPFormClass::getElementContent(const char *id)
{
    std::string s;
    p_memCopy(s, ESPFORM_STR_19);
    s += id;
    p_memCopy(s, ESPFORM_STR_22);
    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_73);
        Serial.println(b);
        delPtr(b);
    }

    _webSocketPtr->broadcastTXT(s.c_str(), s.length());
}

void ESPFormClass::setElementContent(const char *id, const String &content)
{
    std::string s;
    p_memCopy(s, ESPFORM_STR_20);
    s += id;
    p_memCopy(s, ESPFORM_STR_21);
    s += content.c_str();
    p_memCopy(s, ESPFORM_STR_22);
    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_74);
        Serial.println(b);
        delPtr(b);
    }

    _webSocketPtr->broadcastTXT(s.c_str(), s.length());
}

void ESPFormClass::runScript(const String &script)
{
    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_75);
        Serial.println(b);
        delPtr(b);
    }
    _webSocketPtr->broadcastTXT(script.c_str(), script.length());
}

void ESPFormClass::getPath(uint8_t type, int index, std::string &buf)
{
    char *t = getIntString(index);
    p_memCopy(buf, ESPFORM_STR_14, true);
    buf += t;
    delPtr(t);
    p_memCopy(buf, ESPFORM_STR_15);

    switch (type)
    {
    case 0:
        p_memCopy(buf, ESPFORM_STR_16);
        break;
    case 1:
        p_memCopy(buf, ESPFORM_STR_17);
        break;

    case 2:
        p_memCopy(buf, ESPFORM_STR_18);
        break;

    default:
        break;
    }
}

void ESPFormClass::stopServer()
{

    if (_webSocketPtr)
    {
        _webSocketPtr->disconnect();
        _webSocketPtr->close();
    }
    if (_webServerPtr)
        _webServerPtr->close();

    _apStarted = false;
    _idle_to._serverStarted = false;
    _idle_to._idleTime = 0;
    _idle_to._clientCount = 0;
    _idle_to._idleStarted = false;
    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.setAutoReconnect(true);
}

void ESPFormClass::startServer()
{

    if (!_apStarted && _apSSID.length() > 0 && _apSSID.length() < 32 && _apPSW.length() >= 8 && _apPSW.length() < 64)
    {
        startAP();
        startDNSServer();
        _apStarted = true;
    }
    if (!_idle_to._serverStarted)
    {
        startWebSocket();
        startWebServer();
        if (!_taskCreated)
            serverRun();
        _idle_to._serverStarted = true;
    }
}

void ESPFormClass::startAP()
{
    if (_ipConfig)
    {
        if (!WiFi.softAPConfig(_ip, _gateway, _subnet) && _debug)
        {
            char *b = getPGMString(ESPFORM_STR_76);
            Serial.println(b);
            delPtr(b);
            return;
        }
    }

    if (WiFi.softAP(_apSSID.c_str(), _apPSW.c_str(), _channel, _ssid_hidden, _max_connection))
    {
        IPAddress address = WiFi.softAPIP();
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_77);
            Serial.printf(b, _apSSID.c_str(), _apPSW.c_str(), address.toString().c_str());
            delPtr(b);
        }
    }
    else
    {
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_78);
            Serial.println(b);
            delPtr(b);
        }
    }
}

void ESPFormClass::startDNSServer()
{
#ifdef ESP32
    std::string s;
    p_memCopy(s, ESPFORM_STR_4);
    MDNS.begin(s.c_str());
#elif defined(ESP8266)

    _dnsServerPtr.reset(new DNSServer());
    _dnsServerPtr->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServerPtr->start(dnsPort, "*", WiFi.softAPIP());

    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_79);
        Serial.println(b);
        delPtr(b);
    }
#endif
}

void ESPFormClass::startWebServer()
{

#ifdef ESP32
    _webServerPtr->on("/", std::bind(&ESPFormClass::handleFileRead, this));
#elif defined(ESP8266)
    std::string s;
    p_memCopy(s, ESPFORM_STR_24);
    _webServerPtr->on("/", std::bind(&ESPFormClass::handleFileRead, this));
#endif
    _webServerPtr->onNotFound(std::bind(&ESPFormClass::handleNotFound, this));
    _webServerPtr->begin();
    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_80);
        Serial.println(b);
        delPtr(b);
    }
}

void ESPFormClass::startWebSocket()
{

    _webSocketPtr->begin();
    _webSocketPtr->onEvent(std::bind(&ESPFormClass::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_81);
        Serial.println(b);
        delPtr(b);
    }
}

void ESPFormClass::handleNotFound()
{
    if (!isIP(_webServerPtr->hostHeader()))
        goLandingPage();
    else if (!handleFileRead())
    {
        std::string s, s2;
        p_memCopy(s, ESPFORM_STR_7);
        p_memCopy(s2, ESPFORM_STR_38);
        _webServerPtr->send(404, s.c_str(), s2.c_str());
    }
}

void ESPFormClass::getMIME(const String &ext, String &mime)
{
    mime = "";
    char *tmp = new char[50];
    for (int i = 0; i < maxType; i++)
    {
        if (strcmp_P(ext.c_str(), MIMEInfo[i].endsWith) == 0)
        {
            memset(tmp, 0, 50);
            strcpy_P(tmp, MIMEInfo[i].mimeType);
            mime = tmp;
            break;
        }
    }
    delete[] tmp;
}

bool ESPFormClass::handleFileRead()
{
    bool res = false;
    std::string s1, s2, s3, s4, s5;
    p_memCopy(s1, ESPFORM_STR_13);
    p_memCopy(s2, ESPFORM_STR_12);
    String path = _webServerPtr->uri();

    prepareConfig();

    if (path.endsWith(s1.c_str()))
        path += s2.c_str();

    if (_debug)
    {
        char *b = getPGMString(ESPFORM_STR_82);
        Serial.print(b);
        delPtr(b);
        Serial.println(path);
    }

    p_memCopy(s1, ESPFORM_STR_10, true);
    p_memCopy(s2, ESPFORM_STR_11, true);

    _webServerPtr->sendHeader(s1.c_str(), s2.c_str());

    p_memCopy(s1, ESPFORM_STR_8, true);
    p_memCopy(s2, ESPFORM_STR_9, true);

    p_memCopy(s3, ESPFORM_STR_25, true);
    p_memCopy(s4, ESPFORM_STR_26, true);
    p_memCopy(s5, ESPFORM_STR_27, true);

    if (strcmp(path.c_str(), s3.c_str()) == 0)
    {
        std::string filename;
        bool fvc = false;
        for (size_t i = 0; i < _file_info.size(); i++)
        {
            if (_file_info[i].name.find_first_of("/") != 0)
                p_memCopy(filename, ESPFORM_STR_13, true);
            filename += _file_info[i].name.c_str();

            if (filename == s3)
            {
                fvc = true;
                break;
            }
        }

        if (!fvc)
        {
            _webServerPtr->sendHeader(s1.c_str(), s2.c_str());
            _webServerPtr->send_P(200, MIMEInfo[ico].mimeType, (const char *)favicon_gz, sizeof(favicon_gz));
            return true;
        }
    }

    if (strcmp(path.c_str(), s4.c_str()) == 0)
    {
        _webServerPtr->sendHeader(s1.c_str(), s2.c_str());
        _webServerPtr->send_P(200, MIMEInfo[js].mimeType, (const char *)espform_js_gz, sizeof(espform_js_gz));
        res = true;
    }
    else if (strcmp(path.c_str(), s5.c_str()) == 0)
    {
        String ap = "";
        ESPJsonData jsonData;
        String id;
        int event = 0;
        String value;
        std::string s, s2, s3, s4, s5, s6, s7;
        p_memCopy(s2, ESPFORM_STR_32);
        p_memCopy(s3, ESPFORM_STR_33);
        p_memCopy(s4, ESPFORM_STR_34);
        p_memCopy(s5, ESPFORM_STR_35);
        p_memCopy(s6, ESPFORM_STR_36);
        p_memCopy(s7, ESPFORM_STR_37);

        for (size_t k = 0; k < _config->size(); k++)
        {
            getPath(0, k, s);
            _config->get(jsonData, s.c_str());
            if (jsonData.success)
                id = jsonData.stringValue;
            getPath(1, k, s);
            _config->get(jsonData, s.c_str());
            if (jsonData.success)
                event = jsonData.intValue;
            getPath(2, k, s);
            _config->get(jsonData, s.c_str());
            if (jsonData.success)
                value = jsonData.stringValue;

            if (value.length() > 0 && jsonData.typeNum != ESPJson::JSON_NULL)
                ap += s2.c_str() + id + s3.c_str() + value + s4.c_str();

            char *a = getIntString(event);
            ap += s5.c_str() + id + s6.c_str() + a + s7.c_str();
            delPtr(a);
        }

#ifdef ESP32
        _webServerPtr->send(200, MIMEInfo[js].mimeType, ap);
#elif defined(ESP8266)
        _webServerPtr->send(200, MIMEInfo[js].mimeType, ap.c_str(), ap.length());
#endif
        res = true;
    }
    else
    {
        std::string filename;
        String ext;
        String mime;
        for (size_t i = 0; i < _file_info.size(); i++)
        {
            if (_file_info[i].name.find_first_of("/") != 0)
                p_memCopy(filename, ESPFORM_STR_13, true);
            filename += _file_info[i].name.c_str();

            if (path.endsWith(filename.c_str()))
            {
                ext = _file_info[i].name.c_str();
                ext = ext.substring(ext.lastIndexOf("."), ext.length());

                if (_file_info[i].path.length() > 0)
                {
                    getMIME(ext, mime);

                    ESPJson js;
                    ESPJsonData d;
#ifdef ESP32
                    if (_file_info[i].storageType == ESPFormStorage_SPIFFS)
                    {
                        SPIFFS.begin(true);
                        if (SPIFFS.exists(_file_info[i].path.c_str()))
                        {
                            file = SPIFFS.open(_file_info[i].path.c_str(), "r");
                            _webServerPtr->streamFile(file, mime.c_str());
                            file.close();
                            res = true;
                        }
                    }
                    else if (_file_info[i].storageType == ESPFormStorage_SD)
                    {
                        if (!_sdOk)
                            _sdOk = sdTest();
                        if (_sdOk)
                        {
                            if (SD.exists(_file_info[i].path.c_str()))
                            {
                                file = SD.open(_file_info[i].path.c_str(), FILE_READ);
                                _webServerPtr->streamFile(file, mime.c_str());
                                file.close();
                                res = true;
                            }
                        }
                    }

#elif defined(ESP8266)

                    if (_file_info[i].storageType == ESPFormStorage_SPIFFS)
                    {
                        if (SPIFFS.begin())
                        {
                            if (SPIFFS.exists(_file_info[i].path.c_str()))
                            {
                                _file = SPIFFS.open(_file_info[i].path.c_str(), "r");
                                _webServerPtr->streamFile(_file, mime.c_str());
                                _file.close();
                                res = true;
                            }
                        }
                    }
                    else if (_file_info[i].storageType == ESPFormStorage_SD)
                    {
                        if (!_sdOk)
                            _sdOk = sdTest();
                        if (_sdOk)
                        {
                            if (SD.exists(_file_info[i].path.c_str()))
                            {
                                file = SD.open(_file_info[i].path.c_str(), FILE_READ);
                                _webServerPtr->streamFile(file, mime.c_str());
                                file.close();
                                res = true;
                            }
                        }
                    }

#endif
                }
                else if (_file_info[i].content && _file_info[i].path.length() == 0)
                {

                    p_memCopy(s3, ESPFORM_STR_28, true);
                    if (strcmp(ext.c_str(), s3.c_str()) == 0)
                    {

                        if (!_webServerPtr->hasArg("espf"))
                        {
                            _webServerPtr->sendHeader(s1.c_str(), s2.c_str());
                            _webServerPtr->send_P(200, MIMEInfo[html].mimeType, (const char *)loader_html_gz, sizeof(loader_html_gz));
                        }
                        else
                        {
                            if (_file_info[i].gzip)
                                _webServerPtr->sendHeader(s1.c_str(), s2.c_str());
                            _webServerPtr->send_P(200, MIMEInfo[html].mimeType, _file_info[i].content, _file_info[i].len);
                        }

                        res = true;
                    }
                    else
                    {
                        getMIME(ext, mime);
                        if (_file_info[i].gzip)
                            _webServerPtr->sendHeader(s1.c_str(), s2.c_str());
                        _webServerPtr->send_P(200, mime.c_str(), _file_info[i].content, _file_info[i].len);
                        res = true;
                        break;
                    }
                    break;
                }
            }
        }
    }
    return res;
}

void ESPFormClass::goLandingPage()
{
    std::string s1, s2, s3;
    p_memCopy(s1, ESPFORM_STR_5);
    p_memCopy(s2, ESPFORM_STR_6);
    s2 += toIpString(_webServerPtr->client().localIP()).c_str();
    p_memCopy(s3, ESPFORM_STR_7);
    _webServerPtr->sendHeader(s1.c_str(), s2.c_str(), true);
    _webServerPtr->send(302, s3.c_str(), "");
}

bool ESPFormClass::isIP(String str)
{
    IPAddress ip;
    return ip.fromString(str);
}

String ESPFormClass::toIpString(IPAddress ip)
{
    return ip.toString();
    /*
    String result;
    for (uint8_t i = 0; i < 3; i++)
        result += String((ip >> (8 * i)) & 0xff) + ".";
    result += String(((ip >> 8 * 3)) & 0xff);
    return result;
    */
}

void ESPFormClass::webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{
    String json = "";
    switch (type)
    {
    case WStype_ERROR:
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_83);
            Serial.printf(b, num);
            delPtr(b);
        }
        break;
    case WStype_BIN:
        break;
    case WStype_FRAGMENT_TEXT_START:
        break;
    case WStype_FRAGMENT_BIN_START:
        break;
    case WStype_FRAGMENT:
        break;
    case WStype_FRAGMENT_FIN:
        break;
    case WStype_PING:
        break;
    case WStype_PONG:
        break;
    case WStype_DISCONNECTED:
        _idle_to._clientCount--;
        if (_idle_to._clientCount == 0 && _idle_to._idleTimeoutCallback != nullptr && !_idle_to._idleStarted)
        {
            _idle_to._idleTime = millis();
            _idle_to._idleStarted = true;
        }
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_84);
            Serial.printf(b, num);
            delPtr(b);
        }

        break;
    case WStype_CONNECTED:
        _idle_to._idleStarted = false;
        _idle_to._idleTime = 0;
        _idle_to._clientCount++;
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_85);
            Serial.printf(b, num);
            delPtr(b);
        }

        break;
    case WStype_TEXT:

        ESPJson json;
        ESPJsonData jsonData;
        json.setJsonData((char *)payload);
        String type, id, value;
        uint8_t event = 0;
        std::string s, s2;

        p_memCopy(s, ESPFORM_STR_30, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            event = jsonData.intValue;

        p_memCopy(s, ESPFORM_STR_29, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            type = jsonData.stringValue;
        p_memCopy(s, ESPFORM_STR_16, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            id = jsonData.stringValue;
        p_memCopy(s, ESPFORM_STR_18, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            value = jsonData.stringValue;

        p_memCopy(s, ESPFORM_STR_30, true);
        p_memCopy(s2, ESPFORM_STR_31, true);

        if (strcmp(type.c_str(), s.c_str()) == 0 || strcmp(type.c_str(), s2.c_str()) == 0)
        {
            HTMLElementItem element;
            element.event = (ESPFormEventType)event;
            element.value = value;
            element.success = true;
            element.type = type;
            element.id = id;
            setElementEventConfigItem(element);
            if (_elementEventCallback)
                _elementEventCallback(element);
        }
        if (_debug)
        {
            char *b = getPGMString(ESPFORM_STR_86);
            Serial.printf(b, lenght, num);
            delPtr(b);
        }

        break;
    }
}

void ESPFormClass::serverRun()
{

#ifdef ESP32

    if (_index == -1)
        objIndex++;

    _taskName = "task_";
    _taskName += String(objIndex).c_str();
    _webSocket.push_back(*_webSocketPtr.get());
    _webServer.push_back(*_webServerPtr.get());
    _idleTimeoutInfo.push_back(_idle_to);
    _index = objIndex - 1;

    TaskFunction_t taskCode = [](void *param) {
        unsigned long _lastReconnectMillis = 0;
        unsigned long _reconnectTimeout = 10000;

        for (;;)
        {
            yield();

            if (_idleTimeoutInfo[objIndex - 1].get()._serverStarted)
            {
                _webSocket[objIndex - 1].get().loop();
                _webServer[objIndex - 1].get().handleClient();

                if (_idleTimeoutInfo[objIndex - 1].get()._clientCount == 0 && _idleTimeoutInfo[objIndex - 1].get()._idleTimeoutCallback != nullptr && !_idleTimeoutInfo[objIndex - 1].get()._idleStarted)
                {
                    _idleTimeoutInfo[objIndex - 1].get()._idleTime = millis();
                    _idleTimeoutInfo[objIndex - 1].get()._idleStarted = true;
                }

                if (_idleTimeoutInfo[objIndex - 1].get()._idleStarted)
                {
                    if (millis() - _idleTimeoutInfo[objIndex - 1].get()._idleTime > _idleTimeoutInfo[objIndex - 1].get()._idleTimeTimeout)
                    {
                        _idleTimeoutInfo[objIndex - 1].get()._idleTime = millis();
                        if (_idleTimeoutInfo[objIndex - 1].get()._idleTimeoutCallback != nullptr && _idleTimeoutInfo[objIndex - 1].get()._serverStarted)
                            _idleTimeoutInfo[objIndex - 1].get()._idleTimeoutCallback();
                    }
                }
            }
            else
            {

                if (WiFi.status() != WL_CONNECTED)
                {
                    if (_lastReconnectMillis == 0)
                    {
                        WiFi.reconnect();
                        _lastReconnectMillis = millis();
                    }
                    if (WiFi.status() != WL_CONNECTED)
                    {
                        if (millis() - _lastReconnectMillis > _reconnectTimeout)
                            _lastReconnectMillis = 0;
                    }
                    else
                        _lastReconnectMillis = 0;
                }
            }

            vTaskDelay(10);
        }

        vTaskDelete(NULL);
    };

    BaseType_t xReturned = xTaskCreatePinnedToCore(taskCode, _taskName.c_str(), 10000, NULL, 3, &_xTaskHandle, 1);

    _taskCreated = xReturned == pdPASS;

#elif defined(ESP8266)
    if (_apStarted)
        _dnsServerPtr->processNextRequest();

    if (_idle_to._serverStarted)
    {
        _webSocketPtr->loop();
        _webServerPtr->handleClient();

        if (_idle_to._clientCount == 0 && _idle_to._idleTimeoutCallback != nullptr && !_idle_to._idleStarted)
        {
            _idle_to._idleTime = millis();
            _idle_to._idleStarted = true;
        }

        if (_idle_to._idleStarted)
        {
            if (millis() - _idle_to._idleTime > _idle_to._idleTimeTimeout)
            {
                _idle_to._idleTime = millis();
                if (_idle_to._idleTimeoutCallback != nullptr)
                    _idle_to._idleTimeoutCallback();
            }
        }
    }
    else
    {
        reconnect();
    }
    _taskCreated = true;
    _callback.set_scheduled_callback(std::bind(&ESPFormClass::serverRun, this));
#endif
}

#if defined(ESP32)
bool ESPFormClass::sdBegin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss)
{
    _sck = sck;
    _miso = miso;
    _mosi = mosi;
    _ss = ss;
    _sdConfigSet = true;
    SPI.begin(_sck, _miso, _mosi, _ss);
    return SD.begin(_ss, SPI);
}

bool ESPFormClass::sdBegin(void)
{
    _sdConfigSet = false;
    return SD.begin();
}
#elif defined(ESP8266)
bool ESPFormClass::sdBegin(uint8_t csPin)
{
    _sdPin = csPin;
    return SD.begin(csPin);
}

bool ESPFormClass::reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (_lastReconnectMillis == 0)
        {
            WiFi.reconnect();
            _lastReconnectMillis = millis();
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            if (millis() - _lastReconnectMillis > _reconnectTimeout)
                _lastReconnectMillis = 0;
            return false;
        }
        else
        {
            _lastReconnectMillis = 0;
        }
    }
    return WiFi.status() == WL_CONNECTED;
}
#endif

bool ESPFormClass::sdTest()
{
    File file;
    std::string filepath;
    p_memCopy(filepath, ESPFORM_STR_4);

#if defined(ESP32)
    if (_sdConfigSet)
    {
        SPI.begin(_sck, _miso, _mosi, _ss);
        SD.begin(_ss, SPI);
    }
    else
        SD.begin();
#elif defined(ESP8266)
    SD.begin(_sdPin);
#endif

    file = SD.open(filepath.c_str(), FILE_WRITE);
    if (!file)
        return false;

    if (!file.write(32))
        return false;

    file.close();

    file = SD.open(filepath.c_str());
    if (!file)
        return false;

    while (file.available())
    {
        if (file.read() != 32)
        {
            file.close();
            return false;
        }
    }

    SD.remove(filepath.c_str());
    std::string().swap(filepath);
    return true;
}

char *ESPFormClass::getPGMString(PGM_P pgm)
{
    size_t len = strlen_P(pgm) + 1;
    char *buf = newPtr(len);
    strcpy_P(buf, pgm);
    buf[len - 1] = 0;
    return buf;
}

void ESPFormClass::delPtr(char *p)
{
    if (p != nullptr)
        delete[] p;
}

char *ESPFormClass::newPtr(size_t len)
{
    char *p = new char[len];
    memset(p, 0, len);
    return p;
}

char *ESPFormClass::newPtr(char *p, size_t len)
{
    delPtr(p);
    p = newPtr(len);
    return p;
}

char *ESPFormClass::newPtr(char *p, size_t len, char *d)
{
    delPtr(p);
    p = newPtr(len);
    strcpy(p, d);
    return p;
}
char *ESPFormClass::getIntString(int value)
{
    char *buf = newPtr(36);
    memset(buf, 0, 36);
    itoa(value, buf, 10);
    return buf;
}

void ESPFormClass::p_memCopy(std::string &buf, PGM_P p, bool empty)
{
    if (empty)
        buf.clear();
    char *b = getPGMString(p);
    buf += b;
    delPtr(b);
}

uint8_t ESPFormClass::getRSSIasQuality(int RSSI)
{
    uint8_t quality = 0;
    if (RSSI <= -100)
        quality = 0;
    else if (RSSI >= -50)
        quality = 100;
    else
        quality = 2 * (RSSI + 100);
    return quality;
}

void ESPFormClass::scanWiFi(WiFiInfo &result, uint8_t max, bool showHidden)
{
    result._r.clear();
    uint8_t netCount = WiFi.scanNetworks(false, showHidden);
    uint8_t *indices = new uint8_t[netCount];
    for (uint8_t i = 0; i < netCount; i++)
        indices[i] = i;

    for (uint8_t i = 0; i < netCount; i++)
        for (uint8_t j = i + 1; j < netCount; j++)
            if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i]))
                std::swap(indices[i], indices[j]);

    std::string t;
    for (uint8_t i = 0; i < netCount; i++)
    {
        if (indices[i] == -1)
            continue;
        t = WiFi.SSID(indices[i]).c_str();
        for (uint8_t j = i + 1; j < netCount; j++)
        {
            if (strcmp(t.c_str(), WiFi.SSID(indices[j]).c_str()) == 0)
            {
                indices[j] = -1;
            }
        }
    }
    t.clear();

    uint count = 0;
    for (uint8_t i = 0; i < netCount; i++)
    {

        if (strcmp(WiFi.SSID(indices[i]).c_str(), _apSSID.c_str()) == 0 && _skipSelfAP)
            continue;

        if (indices[i] == -1)
            continue;

        uint8_t quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
        if (quality > 1 && count < max)
        {
            count++;
            network_info_t r;
            r.ssid = WiFi.SSID(indices[i]);
#if defined(ESP32)
            r.encType = (wifi_auth_mode_t)WiFi.encryptionType(indices[i]);
#elif defined(ESP8266)
            r.encType = (wl_enc_type)WiFi.encryptionType(indices[i]);
#endif

            r.quality = quality;
            r.channel = WiFi.channel(indices[i]);
            result._r.push_back(r);
        }
    }
}

size_t ESPFormClass::getClientCount()
{
    return _idle_to._clientCount;
}

size_t ESPFormClass::getElementCount()
{
    prepareConfig();
    return _config->size();
}

bool ESPFormClass::setClock(float offset)
{

    char *server1 = getPGMString(ESPFORM_STR_61);
    char *server2 = getPGMString(ESPFORM_STR_61);

    configTime(offset * 3600, 0, server1, server2);

    time_t now = time(nullptr);
    uint8_t tryCount = 0;
    while (now < 1577836800)
    {
        now = time(nullptr);
        tryCount++;
        if (tryCount > 50 || now > 1577836800)
            break;
        delay(100);
    }

    delPtr(server1);
    delPtr(server2);
    struct tm timeinfo;
#if defined(ESP32)
    getLocalTime(&timeinfo);
#elif defined(ESP8266)
    gmtime_r(&now, &timeinfo);
#endif
    return now > 1577836800;
}

ESPFormClass ESPForm = ESPFormClass();

#endif