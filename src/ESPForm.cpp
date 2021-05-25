/*
 * The ESPForm for Arduino v 1.0.3
 * 
 * May 25, 2021
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
 * Copyright (c) 2021 K. Suwatchai (Mobizt)
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
    _sd_rdy = false;
    _flash_rdy = false;
    _idle_to._serverStarted = false;
    _ap_started = false;
#if defined(ESP32)
    _index = -1;
    _xTaskHandle = NULL;
#elif defined(ESP8266)
    _sd_config.ss = SD_CS_PIN;
#endif
}

ESPFormClass::~ESPFormClass()
{
    terminateServer();
}

void ESPFormClass::terminateServer()
{
    stopServer();
    if (_form_config)
    {
        _form_config->clear();
        _form_config.reset();
        _form_config = nullptr;
    }
    if (_web_socket_ptr)
    {
        _web_socket_ptr.reset();
        _web_socket_ptr = nullptr;
        _web_server_ptr.reset();
        _web_server_ptr = nullptr;
    }
    _file_info.clear();
#if defined(ESP8266)
    if (_dns_server_ptr)
    {
        _dns_server_ptr.reset();
        _dns_server_ptr = nullptr;
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
    _ap_started = false;
    _ap_ssid = ssid;
    _ap_psw = psw;
    _channel = channel;
    _ssid_hidden = ssid_hidden;
    _max_connection = max_connection;

    _skip_self_ap = false;
    WiFiInfo t;
    int_scanWiFi(&t, NULL, 10);
    for (size_t i = 0; i < t.count(); i++)
    {
        if (strcmp(t.getInfo(i).ssid.c_str(), ssid) == 0)
        {

#if defined(ESP32)
            uint64_t chipId = ESP.getEfuseMac();
            char *_buf = newS(23);
            snprintf(_buf, 23, "%s-%04X%08X", _ap_ssid.c_str(), (uint16_t)(chipId >> 32), (uint32_t)chipId);
            _ap_ssid = _buf;
            delS(_buf);
#elif defined(ESP8266)
            uint32_t chipId = ESP.getChipId();
            char *_buf = newS(23);
            snprintf(_buf, 23, "%s-%04X", _ap_ssid.c_str(), chipId);
            _ap_ssid = _buf;
            delS(_buf);
#endif

            break;
        }
    }
    t.clear();
    _skip_self_ap = true;
}

void ESPFormClass::begin(ElementEventCallback eventCallback, IdleTimeoutCallback timeoutCallback, unsigned long timeout, bool debug)
{

    _debug = debug;
    _elementEventCallback = std::move(eventCallback);
    _idle_to._idleTimeoutCallback = std::move(timeoutCallback);
    _idle_to._idleTimeTimeout = timeout;
    _web_socket_ptr.reset(new WebSocketsServer(_web_socket_port));

#if defined(ESP32)
    _web_server_ptr.reset(new WebServer(_web_server_port));
#elif defined(ESP8266)
    _web_server_ptr.reset(new ESP8266WebServer(_web_server_port));
#endif
    WiFi.setAutoReconnect(true);
}

void ESPFormClass::addElementEventListener(const String &id, ESPFormEventType event, const char *defaultValue)
{
    prepareConfig();

    FirebaseJson json;
    std::string s;
    appendP(s, espform_str_16);
    json.add(s.c_str(), id);
    appendP(s, espform_str_17, true);
    json.add(s.c_str(), (int)event);
    appendP(s, espform_str_18, true);
    if (defaultValue != NULL)
        json.add(s.c_str(), defaultValue);
    else
        json.add(s.c_str());
    _form_config->add(json);
}

void ESPFormClass::saveElementEventConfig(const String &fileName, ESPFormStorageType storagetype)
{
    prepareConfig();

    if (_form_config->size() == 0)
        return;

    FirebaseJson json;
    std::string s;
    appendP(s, espform_str_23);
    json.add(s.c_str(), *_form_config);
    String js;
    json.toString(js, false);

    if (storagetype == esp_form_storage_flash)
    {
        if (!_flash_rdy)
            _flash_rdy = flashTest();

        if (!_flash_rdy)
            return;

        _file = FLASH_FS.open(fileName, "w");
        _file.print(js);
        _file.close();
    }
    else if (storagetype == esp_form_storage_sd)
    {
        if (!_sd_rdy)
            _sd_rdy = sdTest(_file);

        if (!_sd_rdy)
            return;

        _file = SD_FS.open(fileName, FILE_WRITE);
        _file.print(js);
        _file.close();
    }
}

void ESPFormClass::loadElementEventConfig(const String &fileName, ESPFormStorageType storagetype)
{
    FirebaseJson js;

    if (storagetype == esp_form_storage_flash)
    {
        if (!_flash_rdy)
            _flash_rdy = flashTest();

        if (!_flash_rdy)
            return;

        _file = FLASH_FS.open(fileName, "r");
        js.setJsonData(_file.readString());
        _file.close();
    }
    else if (storagetype == esp_form_storage_sd)
    {
        if (!_sd_rdy)
            _sd_rdy = sdTest(_file);

        if (!_sd_rdy)
            return;

        _file = SD_FS.open(fileName, FILE_READ);
        js.setJsonData(_file.readString());
        _file.close();
    }

    FirebaseJsonData d;
    std::string s;
    appendP(s, espform_str_13);
    appendP(s, espform_str_23);
    js.get(d, s.c_str());
    if (d.success)
    {
        _form_config.reset();
        _form_config = nullptr;
        _form_config = std::shared_ptr<FirebaseJsonArray>(new FirebaseJsonArray());
        d.getArray(*_form_config);
    }
}

ESPFormClass::HTMLElementItem ESPFormClass::getElementEventConfigItem(const String &id)
{

    FirebaseJsonData jsonData;
    bool res = false;
    std::string s;
    HTMLElementItem element;

    prepareConfig();

    for (size_t k = 0; k < _form_config->size(); k++)
    {
        getPath(0, k, s);
        _form_config->get(jsonData, s.c_str());
        if (jsonData.success)
        {

            if (id == jsonData.stringValue)
            {
                element.id = id;
                res = true;
                getPath(1, k, s);
                _form_config->get(jsonData, s.c_str());
                if (jsonData.success)
                {
                    res &= true;
                    element.event = (ESPFormEventType)jsonData.intValue;
                }
                getPath(2, k, s);
                _form_config->get(jsonData, s.c_str());
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

    FirebaseJsonData jsonData;
    std::string s;

    prepareConfig();

    for (size_t k = 0; k < _form_config->size(); k++)
    {
        getPath(0, k, s);
        _form_config->get(jsonData, s.c_str());
        if (jsonData.success)
        {
            if (element.id == jsonData.stringValue)
            {
                if (element.event > 0)
                {
                    getPath(1, k, s);
                    _form_config->set(s.c_str(), (int)element.event);
                }

                getPath(2, k, s);
                _form_config->set(s.c_str(), element.value);
                break;
            }
        }
    }
}

void ESPFormClass::removeElementEventConfigItem(const String &id)
{
    FirebaseJsonData jsonData;
    std::string s;

    prepareConfig();

    for (size_t k = 0; k < _form_config->size(); k++)
    {
        getPath(0, k, s);
        _form_config->get(jsonData, s.c_str());
        if (jsonData.success)
        {
            if (id == jsonData.stringValue)
            {
                _form_config->remove(k);
                break;
            }
        }
    }
}

void ESPFormClass::clearElementEventConfig()
{
    prepareConfig();
    _form_config->clear();
}

void ESPFormClass::prepareConfig()
{
    if (!_form_config)
        _form_config = std::shared_ptr<FirebaseJsonArray>(new FirebaseJsonArray());
}

String ESPFormClass::getElementEventString(ESPFormEventType event)
{
    std::string buf;
    switch (event)
    {
    case EVENT_UNDEFINED:
        appendP(buf, espform_str_39);
        break;
    case EVENT_ON_CLICK:
        appendP(buf, espform_str_40);
        break;
    case EVENT_ON_DBLCLICK:
        appendP(buf, espform_str_41);
        break;
    case EVENT_ON_MOUSEDOWN:
        appendP(buf, espform_str_42);
        break;
    case EVENT_ON_MOUSEMOVE:
        appendP(buf, espform_str_43);
        break;
    case EVENT_ON_MOUSEOUT:
        appendP(buf, espform_str_44);
        break;
    case EVENT_ON_MOUSEOVER:
        appendP(buf, espform_str_45);
        break;
    case EVENT_ON_MOUSEUP:
        appendP(buf, espform_str_46);
        break;
    case EVENT_ON_MOUSEWHEEL:
        appendP(buf, espform_str_47);
        break;
    case EVENT_ON_WHEEL:
        appendP(buf, espform_str_48);
        break;
    case EVENT_ON_KEYDOWN:
        appendP(buf, espform_str_49);
        break;
    case EVENT_ON_KEYPRESS:
        appendP(buf, espform_str_50);
        break;
    case EVENT_ON_KEYUP:
        appendP(buf, espform_str_51);
        break;
    case EVENT_ON_CHANGE:
        appendP(buf, espform_str_52);
        break;
    case EVENT_ON_SUBMIT:
        appendP(buf, espform_str_53);
        break;
    case EVENT_ON_INPUT:
        appendP(buf, espform_str_54);
        break;
    case EVENT_ON_FOCUS:
        appendP(buf, espform_str_55);
        break;
    case EVENT_ON_CONTEXTMENU:
        appendP(buf, espform_str_56);
        break;
    case EVENT_ON_SELECT:
        appendP(buf, espform_str_57);
        break;
    case EVENT_ON_SEARCH:
        appendP(buf, espform_str_58);
        break;
    case EVENT_ON_RESET:
        appendP(buf, espform_str_59);
        break;
    case EVENT_ON_INVALID:
        appendP(buf, espform_str_60);
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
        appendP(buf, espform_str_63);
        break;
    case WIFI_AUTH_WPA_PSK:
        appendP(buf, espform_str_68);
        break;
    case WIFI_AUTH_WPA2_PSK:
        appendP(buf, espform_str_69);
        break;
    case WIFI_AUTH_WPA_WPA2_PSK:
        appendP(buf, espform_str_70);
        break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
        appendP(buf, espform_str_71);
        break;
    case WIFI_AUTH_MAX:
        appendP(buf, espform_str_62);
        break;
    default:
        break;
    }
#elif defined(ESP8266)
    switch (encType)
    {
    case ENC_TYPE_WEP:
        appendP(buf, espform_str_63);
        break;
    case ENC_TYPE_TKIP:
        appendP(buf, espform_str_64);
        break;
    case ENC_TYPE_CCMP:
        appendP(buf, espform_str_65);
        break;
    case ENC_TYPE_NONE:
        appendP(buf, espform_str_66);
        break;
    case ENC_TYPE_AUTO:
        appendP(buf, espform_str_67);
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
    appendP(s, espform_str_19);
    s += id;
    appendP(s, espform_str_22);
    if (_debug)
    {
        char *b = strP(espform_str_73);
        Serial.println(b);
        delS(b);
    }

    _web_socket_ptr->broadcastTXT(s.c_str(), s.length());
}

void ESPFormClass::setElementContent(const char *id, const String &content)
{
    std::string s;
    appendP(s, espform_str_20);
    s += id;
    appendP(s, espform_str_21);
    s += content.c_str();
    appendP(s, espform_str_22);
    if (_debug)
    {
        char *b = strP(espform_str_74);
        Serial.println(b);
        delS(b);
    }

    _web_socket_ptr->broadcastTXT(s.c_str(), s.length());
}

void ESPFormClass::runScript(const String &script)
{
    if (_debug)
    {
        char *b = strP(espform_str_75);
        Serial.println(b);
        Serial.println(script.c_str());
        delS(b);
    }
    _web_socket_ptr->broadcastTXT(script.c_str(), script.length());
}

void ESPFormClass::getPath(uint8_t type, int index, std::string &buf)
{
    char *t = intStr(index);
    appendP(buf, espform_str_14, true);
    buf += t;
    delS(t);
    appendP(buf, espform_str_15);

    switch (type)
    {
    case 0:
        appendP(buf, espform_str_16);
        break;
    case 1:
        appendP(buf, espform_str_17);
        break;

    case 2:
        appendP(buf, espform_str_18);
        break;

    default:
        break;
    }
}

void ESPFormClass::stopServer()
{
    if (_web_socket_ptr)
    {
        _web_socket_ptr->disconnect();
        _web_socket_ptr->close();
    }
    if (_web_server_ptr)
        _web_server_ptr->close();

    _ap_started = false;
    _idle_to._serverStarted = false;
    _idle_to._serverRun = false;
    _idle_to._idleTime = 0;
    _idle_to._clientCount = 0;
    _idle_to._idleStarted = false;
}

void ESPFormClass::startServer()
{
    stopServer();

    if (!_ap_started && _ap_ssid.length() > 0 && _ap_ssid.length() < 32 && _ap_psw.length() >= 8 && _ap_psw.length() < 64)
    {
        startAP();
        startDNSServer();
        _ap_started = true;
    }
    if (!_idle_to._serverStarted)
    {
        startWebSocket();
        startWebServer();
        if (!_task_created)
        {
            _idle_to._serverRun = true;
            serverRun();
        }

        _idle_to._serverStarted = true;
    }
}

void ESPFormClass::startAP()
{
    stopAP();

    if (_ipConfig)
    {
        if (!WiFi.softAPConfig(_ip, _gateway, _subnet) && _debug)
        {
            char *b = strP(espform_str_76);
            Serial.println(b);
            delS(b);
            return;
        }
    }

    WiFi.enableAP(true);

    if (WiFi.softAP(_ap_ssid.c_str(), _ap_psw.c_str(), _channel, _ssid_hidden, _max_connection))
    {
        IPAddress address = WiFi.softAPIP();
        if (_debug)
        {
            char *b = strP(espform_str_77);
            Serial.printf(b, _ap_ssid.c_str(), _ap_psw.c_str(), address.toString().c_str());
            delS(b);
        }
    }
    else
    {
        if (_debug)
        {
            char *b = strP(espform_str_78);
            Serial.println(b);
            delS(b);
        }
    }
}

void ESPFormClass::stopAP()
{
    WiFi.softAPdisconnect(true);
}

void ESPFormClass::startDNSServer()
{
#ifdef ESP32
    std::string s;
    appendP(s, espform_str_4);
    MDNS.begin(s.c_str());
#elif defined(ESP8266)
    _dns_server_ptr.reset(new DNSServer());
    _dns_server_ptr->setErrorReplyCode(DNSReplyCode::NoError);
    _dns_server_ptr->start(_dns_port, "*", WiFi.softAPIP());

    if (_debug)
    {
        char *b = strP(espform_str_79);
        Serial.println(b);
        delS(b);
    }
#endif
}

void ESPFormClass::startWebServer()
{

#ifdef ESP32
    _web_server_ptr->on("/", std::bind(&ESPFormClass::handleFileRead, this));
#elif defined(ESP8266)
    std::string s;
    appendP(s, espform_str_24);
    _web_server_ptr->on("/", std::bind(&ESPFormClass::handleFileRead, this));
#endif
    _web_server_ptr->onNotFound(std::bind(&ESPFormClass::handleNotFound, this));
    _web_server_ptr->begin();
    if (_debug)
    {
        char *b = strP(espform_str_80);
        Serial.println(b);
        delS(b);
    }
}

void ESPFormClass::startWebSocket()
{
    if (_web_socket_ptr)
    {
        _web_socket_ptr->begin();
        _web_socket_ptr->onEvent(std::bind(&ESPFormClass::webSocketEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        if (_debug)
        {
            char *b = strP(espform_str_81);
            Serial.println(b);
            delS(b);
        }
    }
}

void ESPFormClass::handleNotFound()
{
    if (!isIP(_web_server_ptr->hostHeader()))
        goLandingPage();
    else if (!handleFileRead())
    {
        std::string s, s2;
        appendP(s, espform_str_7);
        appendP(s2, espform_str_38);
        _web_server_ptr->send(404, s.c_str(), s2.c_str());
    }
}

void ESPFormClass::getMIME(const String &ext, String &mime)
{
    mime = "";
    char *tmp = new char[50];
    for (int i = 0; i < maxType; i++)
    {
        if (strcmp_P(ext.c_str(), ESPForm_MIMEInfo[i].endsWith) == 0)
        {
            memset(tmp, 0, 50);
            strcpy_P(tmp, ESPForm_MIMEInfo[i].mimeType);
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
    appendP(s1, espform_str_13);
    appendP(s2, espform_str_12);
    String path = _web_server_ptr->uri();

    prepareConfig();

    if (path.endsWith(s1.c_str()))
        path += s2.c_str();

    if (_debug)
    {
        char *b = strP(espform_str_82);
        Serial.print(b);
        delS(b);
        Serial.println(path);
    }

    appendP(s1, espform_str_10, true);
    appendP(s2, espform_str_11, true);

    _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());

    appendP(s1, espform_str_8, true);
    appendP(s2, espform_str_9, true);

    appendP(s3, espform_str_25, true);
    appendP(s4, espform_str_26, true);
    appendP(s5, espform_str_27, true);

    if (strcmp(path.c_str(), s3.c_str()) == 0)
    {
        std::string filename;
        bool fvc = false;
        for (size_t i = 0; i < _file_info.size(); i++)
        {
            if (_file_info[i].name.find_first_of("/") != 0)
                appendP(filename, espform_str_13, true);
            filename += _file_info[i].name.c_str();

            if (filename == s3)
            {
                fvc = true;
                break;
            }
        }

        if (!fvc)
        {
            _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());
            _web_server_ptr->send_P(200, ESPForm_MIMEInfo[ico].mimeType, (const char *)favicon_gz, sizeof(favicon_gz));
            return true;
        }
    }

    if (strcmp(path.c_str(), s4.c_str()) == 0)
    {
        _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());
        _web_server_ptr->send_P(200, ESPForm_MIMEInfo[js].mimeType, (const char *)espform_js_gz, sizeof(espform_js_gz));
        res = true;
    }
    else if (strcmp(path.c_str(), s5.c_str()) == 0)
    {
        String ap = "";
        FirebaseJsonData jsonData;
        String id;
        int event = 0;
        String value;
        std::string s, s2, s3, s4, s5, s6, s7;
        appendP(s2, espform_str_32);
        appendP(s3, espform_str_33);
        appendP(s4, espform_str_34);
        appendP(s5, espform_str_35);
        appendP(s6, espform_str_36);
        appendP(s7, espform_str_37);

        for (size_t k = 0; k < _form_config->size(); k++)
        {
            getPath(0, k, s);
            _form_config->get(jsonData, s.c_str());
            if (jsonData.success)
                id = jsonData.stringValue;
            getPath(1, k, s);
            _form_config->get(jsonData, s.c_str());
            if (jsonData.success)
                event = jsonData.intValue;
            getPath(2, k, s);
            _form_config->get(jsonData, s.c_str());
            if (jsonData.success)
                value = jsonData.stringValue;

            if (value.length() > 0 && jsonData.typeNum != FirebaseJson::JSON_NULL)
                ap += s2.c_str() + id + s3.c_str() + value + s4.c_str();

            char *a = intStr(event);
            ap += s5.c_str() + id + s6.c_str() + a + s7.c_str();
            delS(a);
        }

#ifdef ESP32
        _web_server_ptr->send(200, ESPForm_MIMEInfo[js].mimeType, ap);
#elif defined(ESP8266)
        _web_server_ptr->send(200, ESPForm_MIMEInfo[js].mimeType, ap.c_str(), ap.length());
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
                appendP(filename, espform_str_13, true);
            filename += _file_info[i].name.c_str();

            if (path.endsWith(filename.c_str()))
            {
                ext = _file_info[i].name.c_str();
                ext = ext.substring(ext.lastIndexOf("."), ext.length());

                if (_file_info[i].path.length() > 0)
                {
                    getMIME(ext, mime);

                    FirebaseJson js;
                    FirebaseJsonData d;
                    if (_file_info[i].storageType == esp_form_storage_flash)
                    {
                        if (!_flash_rdy)
                            _flash_rdy = flashTest();
                        if (_flash_rdy)
                        {
                            if (FLASH_FS.exists(_file_info[i].path.c_str()))
                            {
                                _file = FLASH_FS.open(_file_info[i].path.c_str(), "r");
                                _web_server_ptr->streamFile(_file, mime.c_str());
                                _file.close();
                                res = true;
                            }
                        }
                    }
                    else if (_file_info[i].storageType == esp_form_storage_sd)
                    {
                        if (!_sd_rdy)
                            _sd_rdy = sdTest(_file);
                        if (_sd_rdy)
                        {
                            if (SD_FS.exists(_file_info[i].path.c_str()))
                            {
                                _file = SD_FS.open(_file_info[i].path.c_str(), FILE_READ);
                                _web_server_ptr->streamFile(_file, mime.c_str());
                                _file.close();
                                res = true;
                            }
                        }
                    }
                }
                else if (_file_info[i].content && _file_info[i].path.length() == 0)
                {

                    appendP(s3, espform_str_28, true);
                    if (strcmp(ext.c_str(), s3.c_str()) == 0)
                    {

                        if (!_web_server_ptr->hasArg("espf"))
                        {
                            _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());
                            _web_server_ptr->send_P(200, ESPForm_MIMEInfo[html].mimeType, (const char *)loader_html_gz, sizeof(loader_html_gz));
                        }
                        else
                        {
                            if (_file_info[i].gzip)
                                _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());
                            _web_server_ptr->send_P(200, ESPForm_MIMEInfo[html].mimeType, _file_info[i].content, _file_info[i].len);
                        }

                        res = true;
                    }
                    else
                    {
                        getMIME(ext, mime);
                        if (_file_info[i].gzip)
                            _web_server_ptr->sendHeader(s1.c_str(), s2.c_str());
                        _web_server_ptr->send_P(200, mime.c_str(), _file_info[i].content, _file_info[i].len);
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
    appendP(s1, espform_str_5);
    appendP(s2, espform_str_6);
    s2 += toIpString(_web_server_ptr->client().localIP()).c_str();
    appendP(s3, espform_str_7);
    _web_server_ptr->sendHeader(s1.c_str(), s2.c_str(), true);
    _web_server_ptr->send(302, s3.c_str(), "");
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
    switch (type)
    {
    case WStype_ERROR:
        if (_debug)
        {
            char *b = strP(espform_str_83);
            Serial.printf(b, num);
            delS(b);
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
            char *b = strP(espform_str_84);
            Serial.printf(b, num);
            delS(b);
        }

        break;
    case WStype_CONNECTED:
        _idle_to._idleStarted = false;
        _idle_to._idleTime = 0;
        _idle_to._clientCount++;
        if (_debug)
        {
            char *b = strP(espform_str_85);
            Serial.printf(b, num);
            delS(b);
        }

        break;
    case WStype_TEXT:

        FirebaseJson json;
        FirebaseJsonData jsonData;
        json.setJsonData((char *)payload);
        String type, id, value;
        uint8_t event = 0;
        std::string s, s2;

        appendP(s, espform_str_30, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            event = jsonData.intValue;

        appendP(s, espform_str_29, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            type = jsonData.stringValue;
        appendP(s, espform_str_16, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            id = jsonData.stringValue;
        appendP(s, espform_str_18, true);
        json.get(jsonData, s.c_str());
        if (jsonData.success)
            value = jsonData.stringValue;

        appendP(s, espform_str_30, true);
        appendP(s2, espform_str_31, true);

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
            char *b = strP(espform_str_86);
            Serial.printf(b, lenght, num);
            delS(b);
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
    _webSocket.push_back(*_web_socket_ptr.get());
    _webServer.push_back(*_web_server_ptr.get());
    _idleTimeoutInfo.push_back(_idle_to);
    _index = objIndex - 1;

    static ESPFormClass *_this = this;

    TaskFunction_t taskCode = [](void *param) {
        for (;;)
        {
            if (!_idleTimeoutInfo[objIndex - 1].get()._serverRun)
                break;

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
                _this->reconnect();
            }

            yield();

            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
        _this->_xTaskHandle = NULL;
        vTaskDelete(NULL);
    };

    BaseType_t xReturned = xTaskCreatePinnedToCore(taskCode, _taskName.c_str(), 10000, NULL, 3, &_xTaskHandle, 1);

    _task_created = xReturned == pdPASS;

#elif defined(ESP8266)
    if (_ap_started)
        _dns_server_ptr->processNextRequest();

    if (_idle_to._serverStarted)
    {
        _web_socket_ptr->loop();
        _web_server_ptr->handleClient();

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
    _task_created = true;
    set_scheduled_callback(std::bind(&ESPFormClass::serverRun, this));
#endif
}

#if defined(ESP8266)
void ESPFormClass::set_scheduled_callback(callback_function_t callback)
{
    _callback_function = std::move([callback]() { schedule_function(callback); });
    _callback_function();
}
#endif

bool ESPFormClass::reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (_last_recon_millis == 0)
        {
            WiFi.reconnect();
            _last_recon_millis = millis();
        }
        if (WiFi.status() != WL_CONNECTED)
        {
            if (millis() - _last_recon_millis > _reccon_tmo)
                _last_recon_millis = 0;
            return false;
        }
        else
        {
            _last_recon_millis = 0;
        }
    }
    return WiFi.status() == WL_CONNECTED;
}

bool ESPFormClass::sdBegin(int8_t ss, int8_t sck, int8_t miso, int8_t mosi)
{
#if defined(CARD_TYPE_SD)
    _sd_config.sck = sck;
    _sd_config.miso = miso;
    _sd_config.mosi = mosi;
    _sd_config.ss = ss;
#if defined(ESP32)
    if (ss > -1)
    {
        SPI.begin(sck, miso, mosi, ss);
        return SD_FS.begin(ss, SPI);
    }
    else
        return SD_FS.begin();
#elif defined(ESP8266)
    if (ss > -1)
        return SD_FS.begin(ss);
    else
        return SD_FS.begin(SD_CS_PIN);
#endif
#endif
}

#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
bool ESPFormClass::sdBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{
    if (config)
    {
        _sd_config.sd_mmc_mountpoint = mountpoint;
        _sd_config.sd_mmc_mode1bit = mode1bit;
        _sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
    }
    return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
}
#endif
#endif

bool ESPFormClass::sdMMCBegin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed)
{
#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
    _sd_config.sd_mmc_mountpoint = mountpoint;
    _sd_config.sd_mmc_mode1bit = mode1bit;
    _sd_config.sd_mmc_format_if_mount_failed = format_if_mount_failed;
    return SD_FS.begin(mountpoint, mode1bit, format_if_mount_failed);
#endif
#endif
    return false;
}

bool ESPFormClass::flashTest()
{
#if defined(ESP32)
    if (FORMAT_FLASH == 1)
        _flash_rdy = FLASH_FS.begin(true);
    else
        _flash_rdy = FLASH_FS.begin();
#elif defined(ESP8266)
    _flash_rdy = FLASH_FS.begin();
#endif
    return _flash_rdy;
}

bool ESPFormClass::sdTest(fs::File file)
{
    std::string filepath = "/sdtest01.txt";
#if defined(CARD_TYPE_SD)
    if (!sdBegin(_sd_config.ss, _sd_config.sck, _sd_config.miso, _sd_config.mosi))
        return false;
#endif
#if defined(ESP32)
#if defined(CARD_TYPE_SD_MMC)
    if (!sdBegin(_sd_config.sd_mmc_mountpoint, _sd_config.sd_mmc_mode1bit, _sd_config.sd_mmc_format_if_mount_failed))
        return false;
#endif
#endif
    file = SD_FS.open(filepath.c_str(), FILE_WRITE);
    if (!file)
        return false;

    if (!file.write(32))
    {
        file.close();
        return false;
    }

    file.close();

    file = SD_FS.open(filepath.c_str());
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
    file.close();

    SD_FS.remove(filepath.c_str());

    std::string().swap(filepath);

    _sd_rdy = true;

    return true;
}

char *ESPFormClass::strP(PGM_P pgm)
{
    size_t len = strlen_P(pgm) + 1;
    char *buf = newS(len);
    strcpy_P(buf, pgm);
    buf[len - 1] = 0;
    return buf;
}

void ESPFormClass::delS(char *p)
{
    if (p != nullptr)
        delete[] p;
}

char *ESPFormClass::newS(size_t len)
{
    char *p = new char[len];
    memset(p, 0, len);
    return p;
}

char *ESPFormClass::intStr(int value)
{
    char *buf = newS(36);
    memset(buf, 0, 36);
    itoa(value, buf, 10);
    return buf;
}

void ESPFormClass::appendP(std::string &buf, PGM_P p, bool empty)
{
    if (empty)
        buf.clear();
    char *b = strP(p);
    buf += b;
    delS(b);
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

void ESPFormClass::scanWiFi(WiFiScanResultItemCallback scanCallback, uint8_t max, bool showHidden)
{
    int_scanWiFi(nullptr, scanCallback, max, showHidden);
}

void ESPFormClass::int_scanWiFi(WiFiInfo *result, WiFiScanResultItemCallback scanCallback, uint8_t max, bool showHidden)
{
    if (result)
        result->_r.clear();
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

        if (strcmp(WiFi.SSID(indices[i]).c_str(), _ap_ssid.c_str()) == 0 && _skip_self_ap)
            continue;

        if (indices[i] == -1)
            continue;

        if (WiFi.SSID(indices[i]).length() == 0)
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
            if (result)
                result->_r.push_back(r);
            else if (scanCallback)
                scanCallback(r);
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
    return _form_config->size();
}

bool ESPFormClass::setClock(float offset)
{

    char *server1 = strP(espform_str_61);
    char *server2 = strP(espform_str_61);

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

    delS(server1);
    delS(server2);
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