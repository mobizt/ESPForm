/*
 * ESPJson, version 1.0.0
 * 
 * The Easiest ESP8266/ESP32 Arduino library for parse, create and edit JSON object using a relative path.
 * 
 * May 10, 2020
 * 
 * Features
 * - None recursive operations
 * - Parse and edit JSON object directly with a specified relative path. 
 * - Prettify JSON string 
 * 
 * 
 * The zserge's JSON object parser library used as part of this library
 * 
 * The MIT License (MIT)
 * Copyright (c) 2020 K. Suwatchai (Mobizt)
 * Copyright (c) 2012–2018, Serge Zaitsev, zaitsev.serge@gmail.com
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

#ifndef ESPJson_CPP
#define ESPJson_CPP

#include "ESPJson.h"

ESPJson::ESPJson()
{
    _init();
}

ESPJson::ESPJson(std::string &data)
{
    _init();
    _setJsonData(data);
}

ESPJson::~ESPJson()
{
    clear();
    _parser.reset();
    _parser = nullptr;
    _finalize();
}

void ESPJson::_init()
{
    _finalize();
    _qt = _getPGMString(ESPJson_STR_2);
    _tab = _getPGMString(ESPJson_STR_22);
    _brk1 = _getPGMString(ESPJson_STR_8);
    _brk2 = _getPGMString(ESPJson_STR_9);
    _brk3 = _getPGMString(ESPJson_STR_10);
    _brk4 = _getPGMString(ESPJson_STR_11);
    _cm = _getPGMString(ESPJson_STR_1);
    _pr2 = _getPGMString(ESPJson_STR_3);
    _nl = _getPGMString(ESPJson_STR_24);
    _nll = _getPGMString(ESPJson_STR_18);
    _pr = _getPGMString(ESPJson_STR_25);
    _pd = _getPGMString(ESPJson_STR_4);
    _pf = _getPGMString(ESPJson_STR_5);
    _fls = _getPGMString(ESPJson_STR_6);
    _tr = _getPGMString(ESPJson_STR_7);
    _string = _getPGMString(ESPJson_STR_12);
    _int = _getPGMString(ESPJson_STR_13);
    _dbl = _getPGMString(ESPJson_STR_14);
    _bl = _getPGMString(ESPJson_STR_15);
    _obj = _getPGMString(ESPJson_STR_16);
    _arry = _getPGMString(ESPJson_STR_17);
    _undef = _getPGMString(ESPJson_STR_19);
    _dot = _getPGMString(ESPJson_STR_20);
}

void ESPJson::_finalize()
{
    _delPtr(_qt);
    _delPtr(_tab);
    _delPtr(_brk1);
    _delPtr(_brk2);
    _delPtr(_brk3);
    _delPtr(_brk4);
    _delPtr(_cm);
    _delPtr(_pr2);
    _delPtr(_nl);
    _delPtr(_nll);
    _delPtr(_pr);
    _delPtr(_pd);
    _delPtr(_pf);
    _delPtr(_fls);
    _delPtr(_tr);
    _delPtr(_string);
    _delPtr(_int);
    _delPtr(_dbl);
    _delPtr(_bl);
    _delPtr(_obj);
    _delPtr(_arry);
    _delPtr(_undef);
    _delPtr(_dot);
}

ESPJson &ESPJson::_setJsonData(std::string &data)
{
    return setJsonData(data.c_str());
}

ESPJson &ESPJson::setJsonData(const String &data)
{
    if (data.length() > 0)
    {
        int p1 = _strpos(data.c_str(), _brk1, 0);
        int p2 = _rstrpos(data.c_str(), _brk2, data.length() - 1);
        if (p1 != -1)
            p1 += 1;
        if (p1 != -1 && p2 != -1)
            _rawbuf = data.substring(p1, p2).c_str();
    }
    else
        _rawbuf.clear();

    return *this;
}

ESPJson &ESPJson::clear()
{
    std::string().swap(_rawbuf);
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    clearPathTk();
    _tokens.reset();
    _tokens = nullptr;
    return *this;
}

ESPJson &ESPJson::add(const String &key)
{
    _addNull(key.c_str());
    return *this;
}

ESPJson &ESPJson::add(const String &key, const String &value)
{
    _addString(key.c_str(), value.c_str());
    return *this;
}
ESPJson &ESPJson::add(const String &key, const char *value)
{
    _addString(key.c_str(), value);
    return *this;
}
ESPJson &ESPJson::add(const String &key, int value)
{
    _addInt(key.c_str(), value);
    return *this;
}

ESPJson &ESPJson::add(const String &key, unsigned short value)
{
    _addInt(key.c_str(), value);
    return *this;
}

ESPJson &ESPJson::add(const String &key, float value)
{
    _addDouble(key.c_str(), value);
    return *this;
}

ESPJson &ESPJson::add(const String &key, double value)
{
    _addDouble(key.c_str(), value);
    return *this;
}

ESPJson &ESPJson::add(const String &key, bool value)
{
    _addBool(key.c_str(), value);
    return *this;
}

ESPJson &ESPJson::add(const String &key, ESPJson &json)
{
    _addJson(key.c_str(), &json);
    return *this;
}

ESPJson &ESPJson::add(const String &key, ESPJsonArray &arr)
{
    _addArray(key.c_str(), &arr);
    return *this;
}

template <typename T>
ESPJson &ESPJson::add(const String &key, T value)
{
    if (std::is_same<T, int>::value)
        _addInt(key, value);
    else if (std::is_same<T, double>::value)
        _addDouble(key, value);
    else if (std::is_same<T, bool>::value)
        _addBool(key, value);
    else if (std::is_same<T, const char *>::value)
        _addString(key, value);
    else if (std::is_same<T, ESPJson &>::value)
        _addJson(key, &value);
    else if (std::is_same<T, ESPJsonArray &>::value)
        _addArray(key, &value);
    return *this;
}

void ESPJson::_addString(const std::string &key, const std::string &value)
{
    _add(key.c_str(), value.c_str(), key.length(), value.length(), true, true);
}

void ESPJson::_addInt(const std::string &key, int value)
{
    char *buf = getIntString(value);
    _add(key.c_str(), buf, key.length(), 60, false, true);
    _delPtr(buf);
}

void ESPJson::_addDouble(const std::string &key, double value)
{
    char *buf = getDoubleString(value);
    _trimDouble(buf);
    _add(key.c_str(), buf, key.length(), 60, false, true);
    _delPtr(buf);
}

void ESPJson::_addBool(const std::string &key, bool value)
{
    if (value)
        _add(key.c_str(), _tr, key.length(), 6, false, true);
    else
        _add(key.c_str(), _fls, key.length(), 7, false, true);
}

void ESPJson::_addNull(const std::string &key)
{
    _add(key.c_str(), _nll, key.length(), 6, false, true);
}

void ESPJson::_addJson(const std::string &key, ESPJson *json)
{
    std::string s;
    json->_toStdString(s);
    _add(key.c_str(), s.c_str(), key.length(), s.length(), false, true);
    std::string().swap(s);
}

void ESPJson::_addArray(const std::string &key, ESPJsonArray *arr)
{
    String arrStr;
    arr->toString(arrStr);
    _add(key.c_str(), arrStr.c_str(), key.length(), arrStr.length(), false, true);
}

char *ESPJson::getFloatString(float value)
{
    char *buf = _newPtr(36);
    dtostrf(value, 7, 6, buf);
    return buf;
}

char *ESPJson::getIntString(int value)
{
    char *buf = _newPtr(36);
    itoa(value, buf, 10);
    return buf;
}

char *ESPJson::getBoolString(bool value)
{
    char *buf = nullptr;
    if (value)
        buf = _getPGMString(ESPJson_STR_7);
    else
        buf = _getPGMString(ESPJson_STR_6);
    return buf;
}

char *ESPJson::getDoubleString(double value)
{
    char *buf = _newPtr(36);
    dtostrf(value, 12, 9, buf);
    return buf;
}

void ESPJson::_trimDouble(char *buf)
{
    size_t i = strlen(buf) - 1;
    while (buf[i] == '0' && i > 0)
    {
        if (buf[i - 1] == '.')
        {
            i--;
            break;
        }
        if (buf[i - 1] != '0')
            break;
        i--;
    }
    if (i < strlen(buf) - 1)
        buf[i] = '\0';
}

void ESPJson::toString(String &buf, bool prettify)
{
    char *nbuf = _newPtr(2);
    if (prettify)
        _parse(nbuf, PRINT_MODE_PRETTY);
    else
        _parse(nbuf, PRINT_MODE_PLAIN);
    buf = _jsonData._dbuf.c_str();
    std::string().swap(_jsonData._dbuf);
    _delPtr(nbuf);
}

void ESPJson::_toStdString(std::string &s, bool isJson)
{
    s.clear();
    size_t bufSize = 10;
    char *buf = _newPtr(bufSize);
    if (isJson)
        strcat(buf, _brk1);
    else
        strcat(buf, _brk3);
    s += buf;
    s += _rawbuf;
    buf = _newPtr(buf, bufSize);
    if (isJson)
        strcat(buf, _brk2);
    else
        strcat(buf, _brk4);
    s += buf;
    _delPtr(buf);
}

ESPJson &ESPJson::_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString, bool isJson)
{
    size_t bufSize = klen + vlen + 20;
    char *buf = _newPtr(bufSize);
    if (_rawbuf.length() > 0)
        strcpy_P(buf, ESPJson_STR_1);
    if (isJson)
    {
        strcat(buf, _qt);
        strcat(buf, key);
        strcat(buf, _qt);
        strcat_P(buf, _pr2);
    }
    if (isString)
        strcat(buf, _qt);
    strcat(buf, value);
    if (isString)
        strcat(buf, _qt);
    _rawbuf += buf;
    _delPtr(buf);
    return *this;
}

ESPJson &ESPJson::_addArrayStr(const char *value, size_t len, bool isString)
{
    char *nbuf = _newPtr(2);
    _add(nbuf, value, 0, len, isString, false);
    _delPtr(nbuf);
    return *this;
}

bool ESPJson::get(ESPJsonData &jsonData, const String &path, bool prettify)
{
    clearPathTk();
    _strToTk(path.c_str(), _pathTk, '/');
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    if (prettify)
        _parse(path.c_str(), PRINT_MODE_PRETTY);
    else
        _parse(path.c_str(), PRINT_MODE_PLAIN);
    if (_jsonData.success)
    {
        if (_jsonData._type == ESPJson::JSMN_STRING && _jsonData._dbuf.c_str()[0] == '"' && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] == '"')
            _jsonData.stringValue = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2).c_str();
        else
            _jsonData.stringValue = _jsonData._dbuf.c_str();
    }
    jsonData = _jsonData;
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    clearPathTk();
    _tokens.reset();
    _tokens = nullptr;
    return _jsonData.success;
}

size_t ESPJson::iteratorBegin(const char *data)
{
    if (data)
        setJsonData(data);
    _espjs_parse(true);
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + 20;
    char *buf = _newPtr(bufLen);
    char *nbuf = _newPtr(2);
    strcpy(buf, s.c_str());
    std::string().swap(s);
    int depth = -1;
    _collectTk = true;
    _eltk.clear();
    for (uint16_t i = 0; i < _tokenCount; i++)
        _parseToken(i, buf, depth, nbuf, -2, PRINT_MODE_NONE);
    _el.clear();
    _delPtr(buf);
    _delPtr(nbuf);
    return _eltk.size();
}

void ESPJson::iteratorEnd()
{
    _eltk.clear();
    clearPathTk();
    _jsonData.stringValue = "";
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    clearPathTk();
    _tokens.reset();
    _tokens = nullptr;
}

void ESPJson::iteratorGet(size_t index, int &type, String &key, String &value)
{
    if (_eltk.size() < index + 1)
        return;
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + 20;
    char *buf = _newPtr(bufLen);
    strcpy(buf, s.c_str());
    std::string().swap(s);
    if (_eltk[index].type == 0)
    {
        ESPJson::espjs_tok_t *h = &_tokens.get()[_eltk[index].index];
        size_t len = h->end - h->start + 3;
        char *k = _newPtr(len);
        strncpy(k, buf + h->start, h->end - h->start);
        ESPJson::espjs_tok_t *g = &_tokens.get()[_eltk[index].index + 1];
        size_t len2 = g->end - g->start + 3;
        char *v = _newPtr(len2);
        strncpy(v, buf + g->start, g->end - g->start);
        key = k;
        value = v;
        type = JSON_OBJECT;
        _delPtr(k);
        _delPtr(v);
    }
    else if (_eltk[index].type == 1)
    {
        ESPJson::espjs_tok_t *g = &_tokens.get()[_eltk[index].index];
        size_t len2 = g->end - g->start + 3;
        char *v = _newPtr(len2);
        strncpy(v, buf + g->start, g->end - g->start);
        value = v;
        key = "";
        type = JSON_ARRAY;
        _delPtr(v);
    }
    _delPtr(buf);
}

void ESPJson::_espjs_parse(bool collectTk)
{
    std::string s;
    _toStdString(s);
    int bufLen = s.length() + 20;
    char *buf = _newPtr(bufLen);
    strcpy(buf, s.c_str());
    std::string().swap(s);
    _tokens.reset();
    _collectTk = collectTk;
    _eltk.clear();
    int cnt = espjs_parse(_parser.get(), buf, bufLen, (ESPJson::espjs_tok_t *)NULL, 0);
    int cnt2 = 0;
    int a = 0;
    int b = 0;
    for (int i = 0; i < bufLen; i++)
    {
        if (buf[i] == ',')
            a++;
        else if (buf[i] == '[' || buf[i] == '{')
            b++;
    }
    cnt2 = 10 + (2 * (a + 1)) + b;

    if (cnt < cnt2)
        cnt = cnt2;

    _tokens = std::shared_ptr<ESPJson::espjs_tok_t>(new ESPJson::espjs_tok_t[cnt + 10]);
    espjs_init(_parser.get());
    _tokenCount = espjs_parse(_parser.get(), buf, bufLen, _tokens.get(), cnt + 10);
    _paresRes = true;
    if (_tokenCount < 0)
        _paresRes = false;
    if (_tokenCount < 1 || _tokens.get()[0].type != ESPJson::JSMN_OBJECT)
        _paresRes = false;
    _jsonData.success = _paresRes;
    _nextToken = 0;
    _nextDepth = 0;
    _tokenMatch = false;
    _refToken = -1;
    _resetParseResult();
    _setElementType();
    _delPtr(buf);
}

void ESPJson::_setMark(int depth, bool mark)
{
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            _el[i].omark = mark;
            break;
        }
    }
}

void ESPJson::_setSkip(int depth, bool skip)
{
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            _el[i].skip = skip;
            break;
        }
    }
}

void ESPJson::_setRef(int depth, bool ref)
{
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (ref)
        {
            if (_el[i].depth == depth - 1)
            {
                _el[i].ref = ref;
                break;
            }
        }
        else
            _el[i].ref = false;
    }
}

void ESPJson::_getTkIndex(int depth, tk_index_t &tk)
{
    tk.oindex = 0;
    tk.olen = 0;
    tk.omark = false;
    tk.type = ESPJson::JSMN_UNDEFINED;
    tk.depth = -1;
    tk.skip = false;
    tk.ref = false;
    tk.index = -1;
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            tk.index = _el[i].index;
            tk.omark = _el[i].omark;
            tk.ref = _el[i].ref;
            tk.type = _el[i].type;
            tk.depth = _el[i].depth;
            tk.oindex = _el[i].oindex;
            tk.olen = _el[i].olen;
            tk.skip = _el[i].skip;
            break;
        }
    }
}

bool ESPJson::_updateTkIndex(uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode, bool advanceCount)
{
    int len = -1;
    bool skip = false;
    bool ref = false;
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            if (_el[i].type == ESPJson::JSMN_OBJECT || _el[i].type == ESPJson::JSMN_ARRAY)
            {
                _el[i].oindex++;
                if (_el[i].oindex >= _el[i].olen)
                {
                    depth = _el[i].depth;
                    len = _el[i].olen;
                    skip = _el[i].skip;
                    if (!_TkRefOk && _el[i].type == ESPJson::JSMN_OBJECT)
                        ref = _el[i].ref;
                    else if (!_TkRefOk && _el[i].type == ESPJson::JSMN_ARRAY && searchIndex > -1)
                        ref = _el[i].ref;
                    if (i > 0)
                        _el.erase(_el.begin() + i);
                    else
                        _el.erase(_el.begin());
                    if (printMode != PRINT_MODE_NONE && !skip)
                    {
                        if (len > 0 && !_arrReplaced)
                        {
                            if (ref)
                                _jsonData._dbuf += _cm;
                            if (_el[i].type == ESPJson::JSMN_OBJECT)
                            {
                                if (printMode == PRINT_MODE_PRETTY)
                                    _jsonData._dbuf += _nl;
                                if (printMode == PRINT_MODE_PRETTY && !ref)
                                {
                                    for (int j = 0; j < depth + 1; j++)
                                        _jsonData._dbuf += _tab;
                                }
                            }
                        }
                        if (ref)
                        {
                            if (!advanceCount)
                                _parseCompleted++;

                            if (!_arrReplaced)
                            {
                                if (_el[i].type == ESPJson::JSMN_OBJECT)
                                {
                                    if (printMode == PRINT_MODE_PRETTY)
                                    {
                                        for (int j = 0; j < depth + 2; j++)
                                            _jsonData._dbuf += _tab;
                                    }
                                    _jsonData._dbuf += _qt;
                                    _jsonData._dbuf += searchKey;
                                    _jsonData._dbuf += _qt;
                                    if (printMode == PRINT_MODE_PRETTY)
                                        _jsonData._dbuf += _pr;
                                    else
                                        _jsonData._dbuf += _pr2;
                                    if (_parseCompleted == (int)_pathTk.size())
                                        _jsonData._dbuf += replace;
                                    else
                                        _insertChilds(replace, printMode);
                                    _arrReplaced = true;
                                    if (printMode == PRINT_MODE_PRETTY)
                                    {
                                        _jsonData._dbuf += _nl;
                                        for (int j = 0; j < depth + 1; j++)
                                            _jsonData._dbuf += _tab;
                                    }
                                }
                                else
                                {
                                    for (int k = _el[i].oindex - 1; k < searchIndex; k++)
                                    {
                                        if (printMode == PRINT_MODE_PRETTY)
                                        {
                                            _jsonData._dbuf += _nl;
                                            for (int j = 0; j < depth + 2; j++)
                                                _jsonData._dbuf += _tab;
                                        }
                                        if (k == searchIndex - 1)
                                        {
                                            if (_parseCompleted == (int)_pathTk.size())
                                                _jsonData._dbuf += replace;
                                            else
                                                _insertChilds(replace, printMode);
                                            _arrReplaced = true;
                                        }
                                        else
                                        {
                                            _jsonData._dbuf += _nll;
                                            _jsonData._dbuf += _cm;
                                        }
                                    }
                                }
                            }
                            _setRef(depth, false);
                            if (!advanceCount)
                                _parseCompleted = _pathTk.size();
                        }

                        if (_el[i].type == ESPJson::JSMN_OBJECT)
                            _jsonData._dbuf += _brk2;
                        else
                        {
                            if (len > 0)
                            {
                                if (printMode == PRINT_MODE_PRETTY)
                                {
                                    _jsonData._dbuf += _nl;
                                    for (int j = 0; j < depth + 1; j++)
                                        _jsonData._dbuf += _tab;
                                }
                            }
                            _jsonData._dbuf += _brk4;
                        }
                    }
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool ESPJson::_updateTkIndex2(std::string &str, uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode)
{
    int len = -1;
    bool skip = false;
    bool ref = false;
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            if (_el[i].type == ESPJson::JSMN_OBJECT || _el[i].type == ESPJson::JSMN_ARRAY)
            {
                _el[i].oindex++;
                if (_el[i].oindex >= _el[i].olen)
                {
                    depth = _el[i].depth;
                    len = _el[i].olen;
                    skip = _el[i].skip;
                    if (!_TkRefOk && _el[i].type == ESPJson::JSMN_OBJECT)
                        ref = _el[i].ref;
                    else if (!_TkRefOk && _el[i].type == ESPJson::JSMN_ARRAY && searchIndex > -1)
                        ref = _el[i].ref;
                    if (i > 0)
                        _el.erase(_el.begin() + i);
                    else
                        _el.erase(_el.begin());
                    if (printMode != PRINT_MODE_NONE && !skip)
                    {
                        if (len > 0)
                        {
                            if (printMode == PRINT_MODE_PRETTY)
                                str += _nl;
                            if (_el[i].type == ESPJson::JSMN_OBJECT)
                            {
                                if (printMode == PRINT_MODE_PRETTY && !ref)
                                {
                                    for (int j = 0; j < depth + 1; j++)
                                        str += _tab;
                                }
                            }
                            else
                            {
                                if (printMode == PRINT_MODE_PRETTY)
                                {
                                    for (int j = 0; j < depth + 1; j++)
                                        str += _tab;
                                }
                            }
                        }
                        if (ref)
                            _setRef(depth, false);
                        if (_el[i].type == ESPJson::JSMN_OBJECT)
                            str += _brk2;
                        else
                            str += _brk4;
                    }
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool ESPJson::_updateTkIndex3(uint16_t index, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode)
{
    int len = -1;
    bool skip = false;
    bool ref = false;
    for (size_t i = 0; i < _el.size(); i++)
    {
        if (_el[i].depth == depth - 1)
        {
            if (_el[i].type == ESPJson::JSMN_OBJECT || _el[i].type == ESPJson::JSMN_ARRAY)
            {
                _el[i].oindex++;
                if (_el[i].oindex >= _el[i].olen)
                {
                    depth = _el[i].depth;
                    len = _el[i].olen;
                    skip = _el[i].skip;
                    if (!_TkRefOk && _el[i].type == ESPJson::JSMN_OBJECT)
                        ref = _el[i].ref;
                    else if (!_TkRefOk && _el[i].type == ESPJson::JSMN_ARRAY && searchIndex > -1)
                        ref = _el[i].ref;
                    if (i > 0)
                        _el.erase(_el.begin() + i);
                    else
                        _el.erase(_el.begin());
                    if (depth < _skipDepth)
                        return false;
                    if (printMode != PRINT_MODE_NONE && skip)
                    {
                        if (len > 0)
                        {
                            if (printMode == PRINT_MODE_PRETTY)
                                _jsonData._dbuf += _nl;
                            if (_el[i].type == ESPJson::JSMN_OBJECT)
                            {
                                if (printMode == PRINT_MODE_PRETTY && !ref)
                                {
                                    for (int j = 0; j < depth + 1 - (_skipDepth + 1); j++)
                                        _jsonData._dbuf += _tab;
                                }
                            }
                            else
                            {
                                if (printMode == PRINT_MODE_PRETTY)
                                {
                                    for (int j = 0; j < depth + 1 - (_skipDepth + 1); j++)
                                        _jsonData._dbuf += _tab;
                                }
                            }
                        }
                        if (ref)
                            _setRef(depth, false);

                        if (_el[i].type == ESPJson::JSMN_OBJECT)
                            _jsonData._dbuf += _brk2;
                        else
                            _jsonData._dbuf += _brk4;
                    }
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

void ESPJson::_insertChilds(char *data, PRINT_MODE printMode)
{
    std::string str = "";
    for (int i = _pathTk.size() - 1; i > _parseCompleted - 1; i--)
    {
        if (_isArrTk(i))
        {
            std::string _str;
            _addArrNodes(_str, str, i, data, printMode);
            str = _str;
            std::string().swap(_str);
        }
        else
        {
            std::string _str;
            _addObjNodes(_str, str, i, data, printMode);
            str = _str;
            std::string().swap(_str);
        }
    }
    if ((int)_pathTk.size() == _parseCompleted)
        str = data;
    _jsonData._dbuf += str;
    std::string().swap(str);
}

void ESPJson::_addArrNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode)
{

    int i = _getArrIndex(index);
    str += _brk3;
    if (printMode == PRINT_MODE_PRETTY)
        str += _nl;
    for (int k = 0; k <= i; k++)
    {
        if (printMode == PRINT_MODE_PRETTY)
        {
            for (int j = 0; j < index + 1; j++)
                str += _tab;
        }
        if (k == i)
        {
            if (index == (int)_pathTk.size() - 1)
                str += data;
            else
                str += str2;
        }
        else
        {
            str += _nll;
            str += _cm;
        }

        if (printMode == PRINT_MODE_PRETTY)
            str += _nl;
    }

    if (printMode == PRINT_MODE_PRETTY)
    {
        for (int j = 0; j < index; j++)
            str += _tab;
    }
    str += _brk4;
}

void ESPJson::_addObjNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode)
{
    str += _brk1;
    if (printMode == PRINT_MODE_PRETTY)
    {
        str += _nl;
        for (int j = 0; j < index + 1; j++)
            str += _tab;
    }
    str += _qt;
    str += _pathTk[index].tk.c_str();
    str += _qt;
    if (printMode == PRINT_MODE_PRETTY)
        str += _pr;
    else
        str += _pr2;
    if (index == (int)_pathTk.size() - 1)
        str += data;
    else
        str += str2;
    if (printMode == PRINT_MODE_PRETTY)
    {
        str += _nl;
        for (int j = 0; j < index; j++)
            str += _tab;
    }
    str += _brk2;
}

void ESPJson::_parseToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode)
{
    tk_index_t tk;
    _getTkIndex(depth, tk);
    ESPJson::espjs_tok_t *h = &_tokens.get()[i];
    bool oskip = false;
    bool ex = false;
    size_t resLen = _jsonData._dbuf.length();
    if (searchIndex == -2)
        tk.skip = true;
    delay(0);
    if (searchIndex > -1)
    {
        tk_index_t tk2;
        int depth2 = depth - 1;
        _getTkIndex(depth2, tk2);
        if (tk.type == ESPJson::JSMN_ARRAY && _parseDepth == depth && tk2.oindex == _parentIndex)
        {
            if (tk.oindex == searchIndex)
            {
                _nextToken = i;
                _nextDepth = depth;
                _parentIndex = tk.oindex;

                if ((int)_pathTk.size() != _parseDepth + 1)
                {
                    _tokenMatch = true;
                    _parseCompleted++;
                }
                else
                {
                    if (!_TkRefOk)
                    {
                        _parseCompleted++;
                        _refTkIndex = i + 1;
                        _refToken = i + 1;
                        _TkRefOk = true;
                        char *dat1 = _newPtr(h->end - h->start + 10);
                        strncpy(dat1, buf + h->start, h->end - h->start);
                        _jsonData.stringValue = dat1;
                        _delPtr(dat1);
                        _jsonData._type = h->type;
                        _jsonData._k_start = h->start;
                        _jsonData._start = h->start;
                        _jsonData._end = h->end;
                        _jsonData._tokenIndex = i;
                        _jsonData._depth = depth;
                        _jsonData._len = h->size;
                        _jsonData.success = true;
                        _setElementType();
                        if (printMode != PRINT_MODE_NONE)
                            _jsonData.stringValue = "";
                        else
                        {
                            std::string().swap(_jsonData._dbuf);
                            std::string().swap(_tbuf);
                            _tokenMatch = true;
                            ex = true;
                        }
                    }
                }
            }
            else
            {
                if (tk.oindex + 1 == tk.olen)
                {
                    _setRef(depth - 1, false);
                    _setRef(depth, true);
                }
            }
        }
    }
    else
    {
        char *key = _newPtr(h->end - h->start + 10);
        strncpy(key, buf + h->start, h->end - h->start);
        if (tk.type != ESPJson::JSMN_UNDEFINED && _parseDepth == depth)
        {
            if (strcmp(searchKey, key) == 0)
            {
                _nextToken = i + 1;
                _nextDepth = depth;
                _parentIndex = tk.oindex;
                if ((int)_pathTk.size() != _parseDepth + 1)
                {
                    _tokenMatch = true;
                    _parseCompleted++;
                }
                else
                {
                    if (!_TkRefOk)
                    {
                        _parseCompleted++;
                        _refTkIndex = i + 1;
                        _refToken = i + 1;
                        _TkRefOk = true;
                        h = &_tokens.get()[i + 1];
                        char *dat2 = _newPtr(h->end - h->start + 10);
                        strncpy(dat2, buf + h->start, h->end - h->start);
                        _jsonData.stringValue = dat2;
                        _delPtr(dat2);
                        _jsonData._type = h->type;
                        _jsonData._k_start = h->start;
                        _jsonData._start = h->start;
                        _jsonData._end = h->end;
                        _jsonData._tokenIndex = i;
                        _jsonData._depth = depth;
                        _jsonData._len = h->size;
                        _jsonData.success = true;
                        _setElementType();
                        if (printMode != PRINT_MODE_NONE)
                            _jsonData.stringValue = "";
                        else
                        {
                            std::string().swap(_jsonData._dbuf);
                            std::string().swap(_tbuf);
                            _tokenMatch = true;
                            ex = true;
                        }
                    }
                }
            }
            else
            {
                if (tk.oindex + 1 == tk.olen)
                {
                    _setRef(depth - 1, false);
                    _setRef(depth, true);
                }
            }
        }
        _delPtr(key);
    }
    if (ex)
        return;
    if (_refTkIndex == i + 1)
    {
        if (tk.type == ESPJson::JSMN_OBJECT)
            oskip = true;
        tk.skip = true;
        _skipDepth = depth;
    }
    h = &_tokens.get()[i];
    if (h->type == ESPJson::JSMN_OBJECT || h->type == ESPJson::JSMN_ARRAY)
    {
        if (printMode != PRINT_MODE_NONE && (tk.skip || _refTkIndex == i + 1))
        {
            if (!tk.omark && i > 0 && resLen > 0)
            {
                if (tk.oindex > 0)
                    _jsonData._dbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                    _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                {
                    for (int j = 0; j < depth - (_skipDepth + 1); j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }
            }
            if (h->type == ESPJson::JSMN_OBJECT)
                _jsonData._dbuf += _brk1;
            else
                _jsonData._dbuf += _brk3;
        }
        el_t e;
        e.index = i;
        e.olen = h->size;
        e.type = h->type;
        e.oindex = 0;
        e.depth = depth;
        e.omark = false;
        e.ref = false;
        if (_refToken != -1)
            e.skip = true;
        else
            e.skip = tk.skip;
        _el.push_back(e);
        depth++;
        if (h->size == 0)
        {
            while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
            {
                delay(0);
            }
        }
    }
    else
    {
        char *tmp = _newPtr(h->end - h->start + 10);
        if (buf[h->start - 1] != '"')
            strncpy(tmp, buf + h->start, h->end - h->start);
        else
            strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
        if (h->size > 0)
        {
            if (printMode != PRINT_MODE_NONE && tk.skip && !oskip)
            {
                if (tk.oindex > 0)
                    _jsonData._dbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size > 0)
                {
                    for (int j = 0; j < depth - (_skipDepth + 1); j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }
                _jsonData._dbuf += tmp;
                if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _pr;
                else
                    _jsonData._dbuf += _pr2;
            }
            if (_collectTk)
            {
                eltk_t el;
                el.index = i;
                el.type = 0;
                _eltk.push_back(el);
            }
            tmp = _newPtr(tmp, h->end - h->start + 10);
            strncpy(tmp, buf + h->start, h->end - h->start);
            h = &_tokens.get()[i + 1];
            if (h->type != ESPJson::JSMN_OBJECT && h->type != ESPJson::JSMN_ARRAY)
            {
                _delPtr(tmp);
                tmp = _newPtr(h->end - h->start + 10);
                strncpy(tmp, buf + h->start, h->end - h->start);
                if (printMode != PRINT_MODE_NONE && tk.skip)
                {
                    if (buf[h->start - 1] != '"')
                        strncpy(tmp, buf + h->start, h->end - h->start);
                    else
                        strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
                    _jsonData._dbuf += tmp;
                }
                i++;
                while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
                {
                    delay(0);
                }
            }
            else
            {
                if (_refToken == i + 1)
                {
                    _setSkip(depth, true);
                }
                _setMark(depth, true);
            }
        }
        else
        {
            if (printMode != PRINT_MODE_NONE && tk.skip)
            {
                if (tk.oindex > 0 && resLen > 0)
                {
                    _jsonData._dbuf += _cm;
                }
                if (printMode == PRINT_MODE_PRETTY && resLen > 0)
                    _jsonData._dbuf += _nl;

                if (printMode == PRINT_MODE_PRETTY && tk.olen > 0 && resLen > 0)
                {
                    for (int j = 0; j < depth - (_skipDepth + 1); j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }
                _jsonData._dbuf += tmp;
            }
            while (_updateTkIndex3(i, depth, searchKey, searchIndex, printMode))
            {
                delay(0);
            }
            if (_collectTk)
            {
                eltk_t el;
                el.index = i;
                el.type = 1;
                _eltk.push_back(el);
            }
        }
        _delPtr(tmp);

        if (_refToken == -1 && _skipDepth == depth)
            _setSkip(depth, false);
    }
    _nextToken = i + 1;
    _refToken = -1;
}

void ESPJson::_compileToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex, bool removeTk)
{
    if (_tokenMatch)
        return;
    tk_index_t tk;
    _getTkIndex(depth, tk);
    ESPJson::espjs_tok_t *h = &_tokens.get()[i];
    bool insertFlag = false;
    bool ex = false;
    delay(0);
    if (searchIndex > -1)
    {
        tk_index_t tk2;
        int depth2 = depth - 1;
        _getTkIndex(depth2, tk2);
        if (tk.type == ESPJson::JSMN_ARRAY && _parseDepth == depth && tk2.oindex == _parentIndex)
        {
            if (tk.oindex == searchIndex)
            {
                _nextToken = i;
                _nextDepth = depth;
                _parentIndex = tk.oindex;
                if ((int)_pathTk.size() != _parseDepth + 1)
                {
                    _tokenMatch = true;
                    _parseCompleted++;
                    _refTkIndex = i + 1;
                }
                else
                {
                    if (!_TkRefOk)
                    {
                        _parseCompleted++;
                        _refTkIndex = i + 1;
                        _refToken = i + 1;
                        _TkRefOk = true;
                        single_child_parent_t p = _findSCParent(depth);
                        if (p.success)
                        {
                            _remTkIndex = p.index + 1;
                            _remFirstTk = p.firstTk;
                            _remLastTk = p.lastTk;
                        }
                        else
                        {
                            _remTkIndex = i + 1;
                            _remFirstTk = tk.oindex == 0;
                            _remLastTk = tk.oindex + 1 == tk.olen;
                        }
                    }
                }
            }
            else
            {
                if (tk.oindex + 1 == tk.olen)
                {
                    _setRef(depth - 1, false);
                    _setRef(depth, true);
                }
            }
        }
    }
    else
    {
        char *key = _newPtr(h->end - h->start + 10);
        strncpy(key, buf + h->start, h->end - h->start);
        if (tk.type != ESPJson::JSMN_UNDEFINED && _parseDepth == depth)
        {
            if (strcmp(searchKey, key) == 0)
            {
                _nextToken = i + 1;
                _nextDepth = depth;
                _parentIndex = tk.oindex;
                if ((int)_pathTk.size() != _parseDepth + 1)
                {
                    _tokenMatch = true;
                    _parseCompleted++;
                    _refTkIndex = i + 1;
                }
                else
                {
                    if (!_TkRefOk)
                    {
                        _parseCompleted++;
                        _refTkIndex = i + 1;
                        _refToken = i + 1;
                        _TkRefOk = true;
                        single_child_parent_t p = _findSCParent(depth);
                        if (p.success)
                        {
                            _remTkIndex = p.index + 1;
                            _remFirstTk = p.firstTk;
                            _remLastTk = p.lastTk;
                        }
                        else
                        {
                            _remTkIndex = i + 1;
                            _remFirstTk = tk.oindex == 0;
                            _remLastTk = tk.oindex + 1 == tk.olen;
                        }
                    }
                }
            }
            else
            {
                if (tk.oindex + 1 == tk.olen)
                {
                    _setRef(depth - 1, false);
                    _setRef(depth, true);
                }
            }
        }
        else
        {
            if (_tokenCount == 1 && h->size == 0 && !removeTk)
            {
                _insertChilds(replace, printMode);
                _nextToken = i + 1;
                _nextDepth = 0;
                _parseCompleted = _pathTk.size();
                _tokenMatch = true;
                ex = true;
            }
        }
        _delPtr(key);
    }
    if (ex)
        return;

    h = &_tokens.get()[i];
    if (h->type == ESPJson::JSMN_OBJECT || h->type == ESPJson::JSMN_ARRAY)
    {
        if (printMode != PRINT_MODE_NONE && !tk.skip)
        {
            if (!tk.omark && i > 0)
            {
                if (tk.oindex > 0)
                    _jsonData._dbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                    _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                {
                    for (int j = 0; j < depth; j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }
            }
            if (_refToken == -1)
            {
                if (h->type == ESPJson::JSMN_OBJECT)
                    _jsonData._dbuf += _brk1;
                else
                    _jsonData._dbuf += _brk3;
            }
            else if (_refToken != -1 && searchIndex > -1)
                _jsonData._dbuf += replace;
        }
        el_t e;
        e.index = i;
        e.olen = h->size;
        e.type = h->type;
        e.oindex = 0;
        e.depth = depth;
        e.omark = false;
        e.ref = false;
        if (_refToken != -1)
            e.skip = true;
        else
            e.skip = tk.skip;
        _el.push_back(e);
        depth++;
        if (h->size == 0)
        {
            while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
            {
                delay(0);
            }
        }
    }
    else
    {
        if (_refTkIndex == refTokenIndex && refTokenIndex > -1)
        {
            _refToken = refTokenIndex;
            _refTkIndex = -1;
            insertFlag = true;
        }
        char *tmp = _newPtr(h->end - h->start + 10);
        if (buf[h->start - 1] != '"')
            strncpy(tmp, buf + h->start, h->end - h->start);
        else
            strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
        if (h->size > 0)
        {
            if (printMode != PRINT_MODE_NONE && !tk.skip)
            {
                if (tk.oindex > 0)
                    _jsonData._dbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size > 0)
                {
                    for (int j = 0; j < depth; j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }
                _jsonData._dbuf += tmp;
                if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _pr;
                else
                    _jsonData._dbuf += _pr2;
            }
            tmp = _newPtr(tmp, h->end - h->start + 10);
            strncpy(tmp, buf + h->start, h->end - h->start);
            h = &_tokens.get()[i + 1];
            if (h->type != ESPJson::JSMN_OBJECT && h->type != ESPJson::JSMN_ARRAY)
            {
                tmp = _newPtr(tmp, h->end - h->start + 10);
                strncpy(tmp, buf + h->start, h->end - h->start);

                if (printMode != PRINT_MODE_NONE && !tk.skip)
                {
                    if (buf[h->start - 1] != '"')
                        strncpy(tmp, buf + h->start, h->end - h->start);
                    else
                        strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
                    if (_refToken == i + 1)
                    {
                        if (!insertFlag)
                            _jsonData._dbuf += replace;
                        else
                            _insertChilds(replace, printMode);
                    }
                    else
                        _jsonData._dbuf += tmp;
                }
                i++;
                while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
                {
                    delay(0);
                }
            }
            else
            {
                if (_refToken == i + 1)
                {
                    _setSkip(depth, true);
                    _skipDepth = depth;
                    if (!insertFlag)
                        _jsonData._dbuf += replace;
                    else
                        _insertChilds(replace, printMode);
                    if (printMode != PRINT_MODE_NONE && (depth > 0 || tk.oindex == tk.olen - 1))
                    {
                        if (printMode == PRINT_MODE_PRETTY)
                            _jsonData._dbuf += _nl;
                        if (printMode == PRINT_MODE_PRETTY)
                        {
                            for (int j = 0; j < depth; j++)
                                _jsonData._dbuf += _tab;
                        }
                        _jsonData._dbuf += _brk2;
                    }
                }
                _setMark(depth, true);
            }
        }
        else
        {
            if (printMode != PRINT_MODE_NONE && !tk.skip)
            {
                if (tk.oindex > 0)
                    _jsonData._dbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY)
                    _jsonData._dbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && tk.olen > 0)
                {
                    for (int j = 0; j < depth; j++)
                        _jsonData._dbuf += _tab;
                    _jsonData._dbuf += _tab;
                }

                if (_refToken == i + 1 && !_arrInserted)
                {
                    if (!insertFlag)
                        _jsonData._dbuf += replace;
                    else
                        _insertChilds(replace, printMode);
                    _arrInserted = true;
                }
                else
                    _jsonData._dbuf += tmp;
            }
            while (_updateTkIndex(i, depth, searchKey, searchIndex, replace, printMode, removeTk))
            {
                delay(0);
            }
        }
        _delPtr(tmp);

        if (_refToken == -1 && _skipDepth == depth)
            _setSkip(depth, false);
    }
    _nextToken = i + 1;
    _refToken = -1;
}

void ESPJson::_removeToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex, bool removeTk)
{
    bool ncm = false;
    tk_index_t tk;
    _getTkIndex(depth, tk);
    ESPJson::espjs_tok_t *h = &_tokens.get()[i];
    delay(0);
    if (refTokenIndex == i && refTokenIndex > -1)
        ncm = _remFirstTk;
    if (refTokenIndex != i || (refTokenIndex == i && _remLastTk))
        _jsonData._dbuf += _tbuf;
    _tbuf.clear();
    bool flag = tk.oindex > 0 && !ncm && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] != '{' && _jsonData._dbuf.c_str()[_jsonData._dbuf.length() - 1] != '[';
    if (refTokenIndex == i + 1 && refTokenIndex > -1)
    {
        _refToken = refTokenIndex;
        _refTkIndex = -1;
        tk.skip = true;
    }
    h = &_tokens.get()[i];
    if (h->type == ESPJson::JSMN_OBJECT || h->type == ESPJson::JSMN_ARRAY)
    {
        if (printMode != PRINT_MODE_NONE && !tk.skip)
        {
            if (!tk.omark && i > 0)
            {
                if (flag)
                    _tbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                    _tbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size >= 0)
                {
                    for (int j = 0; j < depth; j++)
                        _tbuf += _tab;
                    _tbuf += _tab;
                }
            }
            if (_refToken == -1)
            {
                if (h->type == ESPJson::JSMN_OBJECT)
                    _tbuf += _brk1;
                else
                    _tbuf += _brk3;
            }
            else if (_refToken != -1 && searchIndex > -1)
                _tbuf += replace;
        }
        el_t e;
        e.index = i;
        e.olen = h->size;
        e.type = h->type;
        e.oindex = 0;
        e.depth = depth;
        e.omark = false;
        e.ref = false;
        if (_refToken != -1)
            e.skip = true;
        else
            e.skip = tk.skip;
        _el.push_back(e);
        depth++;
        if (h->size == 0)
        {
            while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
            {
                delay(0);
            }
        }
    }
    else
    {
        char *tmp = _newPtr(h->end - h->start + 10);
        if (buf[h->start - 1] != '"')
            strncpy(tmp, buf + h->start, h->end - h->start);
        else
            strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
        if (h->size > 0)
        {
            if (printMode != PRINT_MODE_NONE && !tk.skip)
            {
                if (flag)
                    _tbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY)
                    _tbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && h->size > 0)
                {
                    for (int j = 0; j < depth; j++)
                        _tbuf += _tab;
                    _tbuf += _tab;
                }
                _tbuf += tmp;
                if (printMode == PRINT_MODE_PRETTY)
                    _tbuf += _pr;
                else
                    _tbuf += _pr2;
            }
            tmp = _newPtr(tmp, h->end - h->start + 10);
            strncpy(tmp, buf + h->start, h->end - h->start);
            h = &_tokens.get()[i + 1];
            if (h->type != ESPJson::JSMN_OBJECT && h->type != ESPJson::JSMN_ARRAY)
            {
                tmp = _newPtr(tmp, h->end - h->start + 10);
                strncpy(tmp, buf + h->start, h->end - h->start);
                if (printMode != PRINT_MODE_NONE && !tk.skip)
                {
                    if (buf[h->start - 1] != '"')
                        strncpy(tmp, buf + h->start, h->end - h->start);
                    else
                        strncpy(tmp, buf + h->start - 1, h->end - h->start + 2);
                    _tbuf += tmp;
                }
                i++;
                while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
                {
                    delay(0);
                }
            }
            else
            {
                if (_refToken == i + 1)
                {
                    _setSkip(depth, true);
                    _skipDepth = depth;
                    _tbuf += replace;
                    if (printMode != PRINT_MODE_NONE && (depth > 0 || tk.oindex == tk.olen - 1))
                    {
                        if (printMode == PRINT_MODE_PRETTY)
                            _tbuf += _nl;
                        if (printMode == PRINT_MODE_PRETTY)
                        {
                            for (int j = 0; j < depth; j++)
                                _tbuf += _tab;
                        }
                        _tbuf += _brk2;
                    }
                }
                _setMark(depth, true);
            }
        }
        else
        {
            if (printMode != PRINT_MODE_NONE && !tk.skip)
            {
                if (flag)
                    _tbuf += _cm;
                if (printMode == PRINT_MODE_PRETTY)
                    _tbuf += _nl;
                if (printMode == PRINT_MODE_PRETTY && tk.olen > 0)
                {
                    for (int j = 0; j < depth; j++)
                        _tbuf += _tab;
                    _tbuf += _tab;
                }
                _tbuf += tmp;
            }
            while (_updateTkIndex2(_tbuf, i, depth, searchKey, searchIndex, replace, printMode))
            {
                delay(0);
            }
        }
        _delPtr(tmp);

        if (_refToken == -1 && _skipDepth == depth)
            _setSkip(depth, false);
    }
    _nextToken = i + 1;
    _refToken = -1;
    _lastTk.olen = tk.olen;
    _lastTk.oindex = tk.oindex;
    _lastTk.type = tk.type;
    _lastTk.depth = tk.depth;
    _lastTk.index = tk.index;
    _lastTk.skip = tk.skip;
}

ESPJson::single_child_parent_t ESPJson::_findSCParent(int depth)
{
    single_child_parent_t res;
    res.index = -1;
    res.firstTk = false;
    res.lastTk = false;
    res.success = false;
    for (int i = depth; i >= 0; i--)
    {
        bool match = false;
        for (size_t j = 0; j < _el.size(); j++)
        {
            if (_el[j].depth == i - 1 && _el[i].olen == 1)
            {
                match = true;
                res.index = _el[i].index;
                res.firstTk = _el[j].oindex == 0;
                res.lastTk = _el[j].oindex + 1 == _el[j].olen;
                res.success = true;
            }
        }
        if (!match)
            break;
    }
    return res;
}

void ESPJson::_get(const char *key, int depth, int index)
{
    _tokenMatch = false;
    if (_paresRes)
    {
        std::string s;
        _toStdString(s);
        int bufLen = s.length() + 20;
        char *buf = _newPtr(bufLen);
        strcpy(buf, s.c_str());
        std::string().swap(s);

        if (_jsonData.success)
        {
            _jsonData._dbuf.clear();
            _parseDepth = depth;
            if (_nextToken < 0)
                _nextToken = 0;
            for (uint16_t i = _nextToken; i < _tokenCount; i++)
            {
                _parseToken(i, buf, _nextDepth, (char *)key, index, PRINT_MODE_NONE);
                if (_tokenMatch)
                    break;
            }
        }
        _delPtr(buf);
        if (!_tokenMatch)
        {
            _paresRes = false;
            _jsonData.success = false;
            _resetParseResult();
        }
    }
}

void ESPJson::_strToTk(const std::string &str, std::vector<path_tk_t> &tk, char delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    std::string s;
    while (current != std::string::npos)
    {
        s = str.substr(previous, current - previous);
        _trim(s);
        if (s.length() > 0)
        {
            path_tk_t tk_t;
            tk_t.tk = s;
            tk.push_back(tk_t);
        }

        previous = current + 1;
        current = str.find(delim, previous);
        delay(0);
    }
    s = str.substr(previous, current - previous);
    _trim(s);
    if (s.length() > 0)
    {
        path_tk_t tk_t;
        tk_t.tk = s;
        tk.push_back(tk_t);
    }
    std::string().swap(s);
}

void ESPJson::_ltrim(std::string &str, const std::string &chars)
{
    str.erase(0, str.find_first_not_of(chars));
}

void ESPJson::_rtrim(std::string &str, const std::string &chars)
{
    str.erase(str.find_last_not_of(chars) + 1);
}

void ESPJson::_trim(std::string &str, const std::string &chars)
{
    _ltrim(str, chars);
    _rtrim(str, chars);
}

void ESPJson::_parse(const char *path, PRINT_MODE printMode)
{
    clearPathTk();
    _strToTk(path, _pathTk, '/');
    _espjs_parse();
    if (!_jsonData.success)
        return;
    _jsonData.success = false;
    char *nbuf = _newPtr(2);
    int len = _pathTk.size();
    _nextDepth = -1;
    _nextToken = 0;
    _skipDepth = -1;
    _parentIndex = -1;
    _TkRefOk = false;
    _parseCompleted = 0;
    _arrReplaced = false;
    _refTkIndex = -1;
    _remTkIndex = -1;
    _remFirstTk = false;
    _remLastTk = false;
    _el.clear();
    _eltk.clear();
    if (len == 0)
    {
        _parse(nbuf, 0, -2, printMode);
        _jsonData.success = true;
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            if (_isStrTk(i))
                _parse(_pathTk[i].tk.c_str(), i, -1, printMode);
            else if (_isArrTk(i))
                _parse(nbuf, i, _getArrIndex(i), printMode);
            else
                _parse(_pathTk[i].tk.c_str(), i, -1, printMode);
        }
        _jsonData.success = _parseCompleted == len;
    }
    _el.clear();
    _eltk.clear();
    _delPtr(nbuf);
    clearPathTk();
    std::string().swap(_tbuf);
    _tokens.reset();
    _tokens = nullptr;
}

void ESPJson::clearPathTk()
{
    size_t len = _pathTk.size();
    for (size_t i = 0; i < len; i++)
        std::string().swap(_pathTk[i].tk);
    for (size_t i = 0; i < len; i++)
        _pathTk.erase(_pathTk.end());
    _pathTk.clear();
    std::vector<path_tk_t>().swap(_pathTk);
}

void ESPJson::_parse(const char *key, int depth, int index, PRINT_MODE printMode)
{
    _tokenMatch = false;
    if (_paresRes)
    {
        std::string s;
        _toStdString(s);
        int bufLen = s.length() + 20;
        char *buf = _newPtr(bufLen);
        strcpy(buf, s.c_str());
        std::string().swap(s);
        _parseDepth = depth;
        if (_nextToken < 0)
            _nextToken = 0;

        for (uint16_t i = _nextToken; i < _tokenCount; i++)
        {

            int oDepth = _nextDepth;

            _parseToken(i, buf, _nextDepth, (char *)key, index, printMode);

            if (oDepth > _nextDepth && index == -1)
            {
                if (_nextDepth > -1 && _nextDepth < (int)_pathTk.size())
                {
                    if (_pathTk[_nextDepth].matched)
                    {
                        _tokenMatch = false;
                        break;
                    }
                }
            }

            if (_tokenMatch)
            {
                _pathTk[depth].matched = true;
                break;
            }
        }

        _delPtr(buf);
        if (!_tokenMatch)
        {
            _paresRes = false;
            _jsonData.success = false;
        }
    }
}

void ESPJson::_compile(const char *key, int depth, int index, const char *replace, PRINT_MODE printMode, int refTokenIndex, bool removeTk)
{
    _tokenMatch = false;
    if (_paresRes)
    {
        std::string s;
        _toStdString(s);
        int bufLen = s.length() + 20;
        char *buf = _newPtr(bufLen);
        strcpy(buf, s.c_str());
        std::string().swap(s);
        _parseDepth = depth;
        if (_nextToken < 0)
            _nextToken = 0;
        for (uint16_t i = _nextToken; i < _tokenCount; i++)
        {
            _compileToken(i, buf, _nextDepth, (char *)key, index, printMode, (char *)replace, refTokenIndex, removeTk);
            if (_tokenMatch)
                break;
        }
        _delPtr(buf);
        if (!_tokenMatch)
        {
            _paresRes = false;
            _jsonData.success = false;
        }
    }
}

void ESPJson::_remove(const char *key, int depth, int index, const char *replace, int refTokenIndex, bool removeTk)
{
    if (_paresRes)
    {
        std::string s;
        _toStdString(s);
        int bufLen = s.length() + 20;
        char *buf = _newPtr(bufLen);
        strcpy(buf, s.c_str());
        std::string().swap(s);
        _parseDepth = depth;
        if (_nextToken < 0)
            _nextToken = 0;
        for (uint16_t i = _nextToken; i < _tokenCount; i++)
        {
            _removeToken(i, buf, _nextDepth, (char *)key, index, PRINT_MODE_PLAIN, (char *)replace, refTokenIndex, removeTk);
        }
        _delPtr(buf);
    }
}

bool ESPJson::_isArrTk(int index)
{
    if (index < (int)_pathTk.size())
        return _pathTk[index].tk.c_str()[0] == '[' && _pathTk[index].tk.c_str()[_pathTk[index].tk.length() - 1] == ']';
    else
        return false;
}
bool ESPJson::_isStrTk(int index)
{
    if (index < (int)_pathTk.size())
        return _pathTk[index].tk.c_str()[0] == '"' && _pathTk[index].tk.c_str()[_pathTk[index].tk.length() - 1] == '"';
    else
        return false;
}

int ESPJson::_getArrIndex(int index)
{
    int res = -1;
    if (index < (int)_pathTk.size())
    {
        res = atoi(_pathTk[index].tk.substr(1, _pathTk[index].tk.length() - 2).c_str());
        if (res < 0)
            res = 0;
    }
    return res;
}

void ESPJson::set(const String &path)
{
    _setNull(path.c_str());
}

void ESPJson::set(const String &path, const String &value)
{
    _setString(path.c_str(), value.c_str());
}

void ESPJson::set(const String &path, const char *value)
{
    _setString(path.c_str(), value);
}

void ESPJson::set(const String &path, int value)
{
    _setInt(path.c_str(), value);
}

void ESPJson::set(const String &path, unsigned short value)
{
    _setInt(path.c_str(), value);
}

void ESPJson::set(const String &path, double value)
{
    _setDouble(path.c_str(), value);
}

void ESPJson::set(const String &path, bool value)
{
    _setBool(path.c_str(), value);
}

void ESPJson::set(const String &path, ESPJson &json)
{
    _setJson(path.c_str(), &json);
}

void ESPJson::set(const String &path, ESPJsonArray &arr)
{
    _setArray(path.c_str(), &arr);
}

template <typename T>
bool ESPJson::set(const String &path, T value)
{
    if (std::is_same<T, int>::value)
        return _setInt(path, value);
    else if (std::is_same<T, double>::value)
        return _setDouble(path, value);
    else if (std::is_same<T, bool>::value)
        return _setBool(path, value);
    else if (std::is_same<T, const char *>::value)
        return _setString(path, value);
    else if (std::is_same<T, ESPJson &>::value)
        return _setJson(path, &value);
    else if (std::is_same<T, ESPJsonArray &>::value)
        return _setArray(path, &value);
}

void ESPJson::_setString(const std::string &path, const std::string &value)
{
    char *tmp = _newPtr(value.length() + 20);
    strcpy(tmp, _qt);
    strcat(tmp, value.c_str());
    strcat(tmp, _qt);
    _set(path.c_str(), tmp);
    _delPtr(tmp);
    std::string().swap(_jsonData._dbuf);
}

void ESPJson::_setInt(const std::string &path, int value)
{
    char *tmp = getIntString(value);
    _set(path.c_str(), tmp);
    _delPtr(tmp);
    std::string().swap(_jsonData._dbuf);
}

void ESPJson::_setDouble(const std::string &path, double value)
{
    char *tmp = getDoubleString(value);
    _trimDouble(tmp);
    _set(path.c_str(), tmp);
    _delPtr(tmp);
    std::string().swap(_jsonData._dbuf);
}

void ESPJson::_setBool(const std::string &path, bool value)
{
    if (value)
        _set(path.c_str(), _tr);
    else
        _set(path.c_str(), _fls);
    std::string().swap(_jsonData._dbuf);
}

void ESPJson::_setNull(const std::string &path)
{
    _set(path.c_str(), _nll);
    std::string().swap(_jsonData._dbuf);
}

void ESPJson::_setJson(const std::string &path, ESPJson *json)
{
    std::string s;
    json->_toStdString(s);
    _set(path.c_str(), s.c_str());
    std::string().swap(s);
}

void ESPJson::_setArray(const std::string &path, ESPJsonArray *arr)
{
    std::string s;
    arr->_toStdString(s);
    _set(path.c_str(), s.c_str());
    std::string().swap(s);
}

void ESPJson::_set(const char *path, const char *data)
{
    clearPathTk();
    _strToTk(path, _pathTk, '/');
    _espjs_parse();
    if (!_jsonData.success)
        return;
    _jsonData.success = false;
    char *nbuf = _newPtr(2);
    int len = _pathTk.size();
    _nextDepth = -1;
    _nextToken = 0;
    _skipDepth = -1;
    _parentIndex = -1;
    _TkRefOk = false;
    _parseCompleted = 0;
    _arrReplaced = false;
    _arrInserted = false;
    _refTkIndex = -1;
    _remTkIndex = -1;
    _remFirstTk = false;
    _remLastTk = false;
    _el.clear();
    _eltk.clear();
    for (int i = 0; i < len; i++)
    {
        if (_isStrTk(i))
            _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN);
        else if (_isArrTk(i))
            _compile(nbuf, i, _getArrIndex(i), data, PRINT_MODE_PLAIN);
        else
            _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN);
    }
    _el.clear();
    _eltk.clear();
    if (_parseCompleted != len)
    {
        std::string().swap(_jsonData._dbuf);
        std::string().swap(_tbuf);
        int refTokenIndex = _refTkIndex;
        _nextDepth = -1;
        _nextToken = 0;
        _skipDepth = -1;
        _parentIndex = -1;
        _TkRefOk = false;
        _parseCompleted = 0;
        _arrReplaced = false;
        _refTkIndex = -1;
        _tokenMatch = false;
        _paresRes = true;
        for (int i = 0; i < len; i++)
        {
            if (_isStrTk(i))
                _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN, refTokenIndex);
            else if (_isArrTk(i))
                _compile(nbuf, i, _getArrIndex(i), data, PRINT_MODE_PLAIN, refTokenIndex);
            else
                _compile(_pathTk[i].tk.c_str(), i, -1, data, PRINT_MODE_PLAIN, refTokenIndex);
        }
        _el.clear();
        _eltk.clear();
    }
    _delPtr(nbuf);
    if (_jsonData._dbuf.length() >= 2)
    {
        _jsonData.success = true;
        _rawbuf = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2);
    }
    else
        _rawbuf.clear();
    clearPathTk();
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    _tokens.reset();
    _tokens = nullptr;
}

bool ESPJson::remove(const String &path)
{
    clearPathTk();
    _strToTk(path.c_str(), _pathTk, '/');
    _espjs_parse();
    if (!_jsonData.success)
        return false;
    _jsonData.success = false;
    char *nbuf = _newPtr(2);
    int len = _pathTk.size();
    _nextDepth = -1;
    _nextToken = 0;
    _skipDepth = -1;
    _parentIndex = -1;
    _TkRefOk = false;
    _parseCompleted = 0;
    _arrReplaced = false;
    _refTkIndex = -1;
    _remTkIndex = -1;
    _remFirstTk = false;
    _remLastTk = false;
    _el.clear();
    _eltk.clear();
    for (int i = 0; i < len; i++)
    {
        if (_isStrTk(i))
            _compile(_pathTk[i].tk.c_str(), i, -1, nbuf, PRINT_MODE_NONE, -1, true);
        else if (_isArrTk(i))
            _compile(nbuf, i, _getArrIndex(i), nbuf, PRINT_MODE_NONE, -1, true);
        else
            _compile(_pathTk[i].tk.c_str(), i, -1, nbuf, PRINT_MODE_NONE, -1, true);
    }
    _el.clear();
    _eltk.clear();
    std::string().swap(_jsonData._dbuf);
    int refTokenIndex = _remTkIndex;
    if (_parseCompleted == len)
    {
        _nextDepth = -1;
        _nextToken = 0;
        _skipDepth = -1;
        _parentIndex = -1;
        _TkRefOk = false;
        _parseCompleted = 0;
        _arrReplaced = false;
        _refTkIndex = -1;
        _tokenMatch = false;
        _paresRes = true;
        _jsonData.success = true;
        _lastTk.skip = false;
        _lastTk.olen = 0;
        _lastTk.oindex = 0;
        if (_isStrTk(len - 1))
            _remove(_pathTk[len - 1].tk.c_str(), -1, -1, nbuf, refTokenIndex, true);
        else
            _remove(nbuf, -1, _getArrIndex(len - 1), nbuf, refTokenIndex, true);
        _jsonData._dbuf += _tbuf;
        _el.clear();
        _eltk.clear();
    }

    _delPtr(nbuf);
    if (_jsonData._dbuf.length() >= 2)
        _rawbuf = _jsonData._dbuf.substr(1, _jsonData._dbuf.length() - 2);
    else
        _rawbuf.clear();
    clearPathTk();
    std::string().swap(_jsonData._dbuf);
    std::string().swap(_tbuf);
    _tokens.reset();
    _tokens = nullptr;
    return _jsonData.success;
}

void ESPJson::_resetParseResult()
{
    _jsonData._type = 0;
    _jsonData.type = "";
    _jsonData.typeNum = 0;
    _jsonData.stringValue = "";
    _jsonData._dbuf = "";
    _jsonData.intValue = 0;
    _jsonData.doubleValue = 0;
    _jsonData.boolValue = false;
}

void ESPJson::_setElementType()
{
    bool typeSet = false;
    char *buf = _newPtr(20);
    char *tmp = _newPtr(20);
    char *tmp2 = nullptr;
    if (_jsonData._type == ESPJson::JSMN_PRIMITIVE)
    {
        tmp2 = _newPtr(tmp2, _jsonData.stringValue.length() + 1);
        strcpy(tmp2, _jsonData.stringValue.c_str());
    }
    switch (_jsonData._type)
    {
    case ESPJson::JSMN_UNDEFINED:
        strcpy(buf, _undef);
        _jsonData.typeNum = JSON_UNDEFINED;
        break;
    case ESPJson::JSMN_OBJECT:
        strcpy(buf, _obj);
        _jsonData.typeNum = JSON_OBJECT;
        break;
    case ESPJson::JSMN_ARRAY:
        strcpy(buf, _arry);
        _jsonData.typeNum = JSON_ARRAY;
        break;
    case ESPJson::JSMN_STRING:
        strcpy(buf, _string);
        _jsonData.typeNum = JSON_STRING;
        break;
    case ESPJson::JSMN_PRIMITIVE:
        if (!typeSet && strcmp(tmp2, _tr) == 0)
        {
            typeSet = true;
            strcpy(buf, _bl);
            _jsonData.typeNum = JSON_BOOL;
            _jsonData.boolValue = true;
            _jsonData.doubleValue = 1.0;
            _jsonData.intValue = 1;
        }
        else
        {
            if (!typeSet && strcmp(tmp2, _fls) == 0)
            {
                typeSet = true;
                strcpy(buf, _bl);
                _jsonData.typeNum = JSON_BOOL;
                _jsonData.boolValue = false;
                _jsonData.doubleValue = 0.0;
                _jsonData.intValue = 0;
            }
        }
        strcpy(tmp, _dot);
        if (!typeSet && _strpos(tmp2, tmp, 0) > -1)
        {
            typeSet = true;
            strcpy(buf, _dbl);
            _jsonData.typeNum = JSON_DOUBLE;
            _jsonData.doubleValue = atof(tmp2);
            _jsonData.intValue = atoi(tmp2);
            _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
        }
        if (!typeSet && strcmp(tmp2, _nll) == 0)
        {
            typeSet = true;
            strcpy(buf, _nll);
            _jsonData.typeNum = JSON_NULL;
        }
        if (!typeSet)
        {
            typeSet = true;
            double d = atof(tmp2);
            if (d > 0x7fffffff)
            {
                strcpy(buf, _dbl);
                _jsonData.doubleValue = d;
                _jsonData.intValue = atoi(tmp2);
                _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
                _jsonData.typeNum = JSON_DOUBLE;
            }
            else
            {
                _jsonData.intValue = atoi(tmp2);
                _jsonData.doubleValue = atof(tmp2);
                _jsonData.boolValue = atof(tmp2) > 0 ? true : false;
                strcpy(buf, _int);
                _jsonData.typeNum = JSON_INT;
            }
        }
        break;
    default:
        break;
    }
    _jsonData.type = buf;
    _delPtr(buf);
    _delPtr(tmp);
    if (tmp2)
        _delPtr(tmp2);
}

int ESPJson::_strpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
        return -1;
    char *_haystack = _newPtr(len - offset + 1);
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = strstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    _delPtr(_haystack);
    return r;
}

int ESPJson::_rstrpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
        return -1;
    char *_haystack = _newPtr(len - offset + 1);
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = _rstrstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    _delPtr(_haystack);
    return r;
}

char *ESPJson::_rstrstr(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    const char *haystack_end = haystack + strlen(haystack) - needle_length;
    const char *p;
    size_t i;
    for (p = haystack_end; p >= haystack; --p)
    {
        for (i = 0; i < needle_length; ++i)
        {
            if (p[i] != needle[i])
                goto next;
        }
        return (char *)p;
    next:;
    }
    return 0;
}

void ESPJson::_delPtr(char *p)
{
    if (p != nullptr)
        delete[] p;
}

char *ESPJson::_newPtr(size_t len)
{
    char *p = new char[len];
    memset(p, 0, len);
    return p;
}

char *ESPJson::_newPtr(char *p, size_t len)
{
    _delPtr(p);
    p = _newPtr(len);
    return p;
}

char *ESPJson::_newPtr(char *p, size_t len, char *d)
{
    _delPtr(p);
    p = _newPtr(len);
    strcpy(p, d);
    return p;
}

char *ESPJson::_getPGMString(PGM_P pgm)
{
    size_t len = strlen_P(pgm) + 1;
    char *buf = _newPtr(len);
    strcpy_P(buf, pgm);
    buf[len - 1] = 0;
    return buf;
}

/**
 * Allocates a fresh unused token from the token pool.
 */
ESPJson::espjs_tok_t *ESPJson::espjs_alloc_token(espjs_parser *parser,
                                                 ESPJson::espjs_tok_t *tokens, size_t num_tokens)
{
    ESPJson::espjs_tok_t *tok;
    if (parser->toknext >= num_tokens)
    {
        return NULL;
    }
    tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

/**
 * Fills token type and boundaries.
 */
void ESPJson::espjs_fill_token(espjs_tok_t *token, espjs_type_t type,
                               int start, int end)
{
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
int ESPJson::espjs_parse_primitive(espjs_parser *parser, const char *js,
                                   size_t len, espjs_tok_t *tokens, size_t num_tokens)
{
    espjs_tok_t *token;
    int start;

    start = parser->pos;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        switch (js[parser->pos])
        {
#ifndef JSMN_STRICT
        /* In strict mode primitive must be followed by "," or "}" or "]" */
        case ':':
#endif
        case '\t':
        case '\r':
        case '\n':
        case ' ':
        case ',':
        case ']':
        case '}':
            goto found;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127)
        {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    }
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    if (tokens == NULL)
    {
        parser->pos--;
        return 0;
    }
    token = espjs_alloc_token(parser, tokens, num_tokens);
    if (token == NULL)
    {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    espjs_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
    token->parent = parser->toksuper;
#endif
    parser->pos--;
    return 0;
}

/**
 * Fills next token with JSON string.
 */
int ESPJson::espjs_parse_string(espjs_parser *parser, const char *js,
                                size_t len, espjs_tok_t *tokens, size_t num_tokens)
{
    espjs_tok_t *token;

    int start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        char c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"')
        {
            if (tokens == NULL)
            {
                return 0;
            }
            token = espjs_alloc_token(parser, tokens, num_tokens);
            if (token == NULL)
            {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            espjs_fill_token(token, JSMN_STRING, start + 1, parser->pos);
#ifdef JSMN_PARENT_LINKS
            token->parent = parser->toksuper;
#endif
            return 0;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\' && parser->pos + 1 < len)
        {
            int i;
            parser->pos++;
            switch (js[parser->pos])
            {
            /* Allowed escaped symbols */
            case '\"':
            case '/':
            case '\\':
            case 'b':
            case 'f':
            case 'r':
            case 'n':
            case 't':
                break;
            /* Allows escaped symbol \uXXXX */
            case 'u':
                parser->pos++;
                for (i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++)
                {
                    /* If it isn't a hex character we have an error */
                    if (!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
                          (js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
                          (js[parser->pos] >= 97 && js[parser->pos] <= 102)))
                    { /* a-f */
                        parser->pos = start;
                        return JSMN_ERROR_INVAL;
                    }
                    parser->pos++;
                }
                parser->pos--;
                break;
            /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int ESPJson::espjs_parse(espjs_parser *parser, const char *js, size_t len,
                         espjs_tok_t *tokens, unsigned int num_tokens)
{
    int r;
    int i;
    espjs_tok_t *token;
    int count = parser->toknext;

    for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++)
    {
        char c;
        espjs_type_t type;

        c = js[parser->pos];
        switch (c)
        {
        case '{':
        case '[':
            count++;
            if (tokens == NULL)
            {
                break;
            }
            token = espjs_alloc_token(parser, tokens, num_tokens);
            if (token == NULL)
                return JSMN_ERROR_NOMEM;
            if (parser->toksuper != -1)
            {
                tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
                token->parent = parser->toksuper;
#endif
            }
            token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
            token->start = parser->pos;
            parser->toksuper = parser->toknext - 1;
            break;
        case '}':
        case ']':
            if (tokens == NULL)
                break;
            type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
            if (parser->toknext < 1)
            {
                return JSMN_ERROR_INVAL;
            }
            token = &tokens[parser->toknext - 1];
            for (;;)
            {
                if (token->start != -1 && token->end == -1)
                {
                    if (token->type != type)
                    {
                        return JSMN_ERROR_INVAL;
                    }
                    token->end = parser->pos + 1;
                    parser->toksuper = token->parent;
                    break;
                }
                if (token->parent == -1)
                {
                    if (token->type != type || parser->toksuper == -1)
                    {
                        return JSMN_ERROR_INVAL;
                    }
                    break;
                }
                token = &tokens[token->parent];
            }
#else
            for (i = parser->toknext - 1; i >= 0; i--)
            {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1)
                {
                    if (token->type != type)
                    {
                        return JSMN_ERROR_INVAL;
                    }
                    parser->toksuper = -1;
                    token->end = parser->pos + 1;
                    break;
                }
            }
            /* Error if unmatched closing bracket */
            if (i == -1)
                return JSMN_ERROR_INVAL;
            for (; i >= 0; i--)
            {
                token = &tokens[i];
                if (token->start != -1 && token->end == -1)
                {
                    parser->toksuper = i;
                    break;
                }
            }
#endif
            break;
        case '\"':
            r = espjs_parse_string(parser, js, len, tokens, num_tokens);
            if (r < 0)
                return r;
            count++;
            if (parser->toksuper != -1 && tokens != NULL)
                tokens[parser->toksuper].size++;
            break;
        case '\t':
        case '\r':
        case '\n':
        case ' ':
            break;
        case ':':
            parser->toksuper = parser->toknext - 1;
            break;
        case ',':
            if (tokens != NULL && parser->toksuper != -1 &&
                tokens[parser->toksuper].type != JSMN_ARRAY &&
                tokens[parser->toksuper].type != JSMN_OBJECT)
            {
#ifdef JSMN_PARENT_LINKS
                parser->toksuper = tokens[parser->toksuper].parent;
#else
                for (i = parser->toknext - 1; i >= 0; i--)
                {
                    if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT)
                    {
                        if (tokens[i].start != -1 && tokens[i].end == -1)
                        {
                            parser->toksuper = i;
                            break;
                        }
                    }
                }
#endif
            }
            break;
#ifdef JSMN_STRICT
        /* In strict mode primitives are: numbers and booleans */
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 't':
        case 'f':
        case 'n':
            /* And they must not be keys of the object */
            if (tokens != NULL && parser->toksuper != -1)
            {
                espjs_tok_t *t = &tokens[parser->toksuper];
                if (t->type == JSMN_OBJECT ||
                    (t->type == JSMN_STRING && t->size != 0))
                {
                    return JSMN_ERROR_INVAL;
                }
            }
#else
        /* In non-strict mode every unquoted value is a primitive */
        default:
#endif
            r = espjs_parse_primitive(parser, js, len, tokens, num_tokens);
            if (r < 0)
                return r;
            count++;
            if (parser->toksuper != -1 && tokens != NULL)
                tokens[parser->toksuper].size++;
            break;

#ifdef JSMN_STRICT
        /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif
        }
    }

    if (tokens != NULL)
    {
        for (i = parser->toknext - 1; i >= 0; i--)
        {
            /* Unmatched opened object or array */
            if (tokens[i].start != -1 && tokens[i].end == -1)
            {
                return JSMN_ERROR_PART;
            }
        }
    }

    return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void ESPJson::espjs_init(espjs_parser *parser)
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

ESPJsonArray::ESPJsonArray()
{
    _init();
}
ESPJsonArray::~ESPJsonArray()
{
    _finalize();
    std::string().swap(_jbuf);
};

void ESPJsonArray::_init()
{
    _finalize();

    _pd = _getPGMString(ESPJson_STR_4);
    _pf = _getPGMString(ESPJson_STR_5);
    _fls = _getPGMString(ESPJson_STR_6);
    _tr = _getPGMString(ESPJson_STR_7);
    _brk3 = _getPGMString(ESPJson_STR_10);
    _brk4 = _getPGMString(ESPJson_STR_11);
    _nll = _getPGMString(ESPJson_STR_18);
    _root = _getPGMString(ESPJson_STR_21);
    _root2 = _getPGMString(ESPJson_STR_26);
    _qt = _getPGMString(ESPJson_STR_2);
    _slash = _getPGMString(ESPJson_STR_27);
}

int ESPJsonArray::_strpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
        return -1;
    char *_haystack = _newPtr(len - offset + 1);
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = strstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    _delPtr(_haystack);
    return r;
}

int ESPJsonArray::_rstrpos(const char *haystack, const char *needle, int offset)
{
    size_t len = strlen(haystack);
    size_t len2 = strlen(needle);
    if (len == 0 || len < len2 || len2 == 0)
        return -1;
    char *_haystack = _newPtr(len - offset + 1);
    _haystack[len - offset] = 0;
    strncpy(_haystack, haystack + offset, len - offset);
    char *p = _rstrstr(_haystack, needle);
    int r = -1;
    if (p)
        r = p - _haystack + offset;
    _delPtr(_haystack);
    return r;
}

char *ESPJsonArray::_rstrstr(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    const char *haystack_end = haystack + strlen(haystack) - needle_length;
    const char *p;
    size_t i;
    for (p = haystack_end; p >= haystack; --p)
    {
        for (i = 0; i < needle_length; ++i)
        {
            if (p[i] != needle[i])
                goto next;
        }
        return (char *)p;
    next:;
    }
    return 0;
}

void ESPJsonArray::_delPtr(char *p)
{
    if (p != nullptr)
        delete[] p;
}

char *ESPJsonArray::_newPtr(size_t len)
{
    char *p = new char[len];
    memset(p, 0, len);
    return p;
}

char *ESPJsonArray::_newPtr(char *p, size_t len)
{
    _delPtr(p);
    p = _newPtr(len);
    return p;
}

char *ESPJsonArray::_newPtr(char *p, size_t len, char *d)
{
    _delPtr(p);
    p = _newPtr(len);
    strcpy(p, d);
    return p;
}

char *ESPJsonArray::_getPGMString(PGM_P pgm)
{
    size_t len = strlen_P(pgm) + 1;
    char *buf = _newPtr(len);
    strcpy_P(buf, pgm);
    buf[len - 1] = 0;
    return buf;
}

void ESPJsonArray::_finalize()
{
    _delPtr(_pd);
    _delPtr(_pf);
    _delPtr(_fls);
    _delPtr(_tr);
    _delPtr(_brk3);
    _delPtr(_brk4);
    _delPtr(_nll);
    _delPtr(_root);
    _delPtr(_root2);
    _delPtr(_qt);
    _delPtr(_slash);
}

ESPJsonArray &ESPJsonArray::add()
{
    _addNull();
    return *this;
}

ESPJsonArray &ESPJsonArray::add(const String &value)
{
    _addString(value.c_str());
    return *this;
}
ESPJsonArray &ESPJsonArray::add(const char *value)
{
    _addString(value);
    return *this;
}
ESPJsonArray &ESPJsonArray::add(int value)
{
    _addInt(value);
    return *this;
}

ESPJsonArray &ESPJsonArray::add(unsigned short value)
{
    _addInt(value);
    return *this;
}

ESPJsonArray &ESPJsonArray::add(double value)
{
    _addDouble(value);
    return *this;
}

ESPJsonArray &ESPJsonArray::add(bool value)
{
    _addBool(value);
    return *this;
}

ESPJsonArray &ESPJsonArray::add(ESPJson &json)
{
    _addJson(&json);
    return *this;
}

ESPJsonArray &ESPJsonArray::add(ESPJsonArray &arr)
{
    _addArray(&arr);
    return *this;
}

template <typename T>
ESPJsonArray &ESPJsonArray::add(T value)
{
    if (std::is_same<T, int>::value)
        _addInt(value);
    else if (std::is_same<T, double>::value)
        _addDouble(value);
    else if (std::is_same<T, bool>::value)
        _addBool(value);
    else if (std::is_same<T, const char *>::value)
        _addString(value);
    else if (std::is_same<T, ESPJson &>::value)
        _addJson(&value);
    else if (std::is_same<T, ESPJsonArray &>::value)
        _addArray(&value);
    return *this;
}

void ESPJsonArray::_addString(const std::string &value)
{
    _arrLen++;
    _json._addArrayStr(value.c_str(), value.length(), true);
}

void ESPJsonArray::_addInt(int value)
{
    _arrLen++;
    char *buf = getIntString(value);
    sprintf(buf, _pd, value);
    _json._addArrayStr(buf, 60, false);
    _delPtr(buf);
}

void ESPJsonArray::_addDouble(double value)
{
    _arrLen++;
    char *buf = getDoubleString(value);
    _trimDouble(buf);
    _json._addArrayStr(buf, 60, false);
    _delPtr(buf);
}

void ESPJsonArray::_addBool(bool value)
{
    _arrLen++;
    if (value)
        _json._addArrayStr(_tr, 6, false);
    else
        _json._addArrayStr(_fls, 7, false);
}

void ESPJsonArray::_addNull()
{
    _arrLen++;
    _json._addArrayStr(_nll, 6, false);
}

void ESPJsonArray::_addJson(ESPJson *json)
{
    _arrLen++;
    std::string s;
    json->_toStdString(s);
    _json._addArrayStr(s.c_str(), s.length(), false);
    std::string().swap(s);
}

void ESPJsonArray::_addArray(ESPJsonArray *arr)
{
    _arrLen++;
    String arrStr;
    arr->toString(arrStr);
    _json._addArrayStr(arrStr.c_str(), arrStr.length(), false);
}

bool ESPJsonArray::get(ESPJsonData &jsonData, const String &path)
{
    return _get(jsonData, path.c_str());
}

bool ESPJsonArray::get(ESPJsonData &jsonData, int index)
{
    char *tmp = getIntString(index);
    std::string path = "";
    path += _brk3;
    path += tmp;
    path += _brk4;
    bool ret = _get(jsonData, path.c_str());
    std::string().swap(path);
    _delPtr(tmp);
    return ret;
}

bool ESPJsonArray::_get(ESPJsonData &jsonData, const char *path)
{
    _json._toStdString(_jbuf, false);
    _json._rawbuf = _root;
    _json._rawbuf += _jbuf;
    std::string path2 = _root2;
    path2 += _slash;
    path2 += path;
    _json.clearPathTk();
    _json._strToTk(path2.c_str(), _json._pathTk, '/');
    if (!_json._isArrTk(1))
    {
        _json._jsonData.success = false;
        goto ex_;
    }
    if (_json._getArrIndex(1) < 0)
    {
        _json._jsonData.success = false;
        goto ex_;
    }
    _json._parse(path2.c_str(), ESPJson::PRINT_MODE_NONE);
    if (_json._jsonData.success)
    {
        _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2).c_str();
        if (_json._jsonData._type == ESPJson::JSMN_STRING && _json._jsonData.stringValue.c_str()[0] == '"' && _json._jsonData.stringValue.c_str()[_json._jsonData.stringValue.length() - 1] == '"')
            _json._jsonData.stringValue = _json._jsonData.stringValue.substring(1, _json._jsonData.stringValue.length() - 1).c_str();
        jsonData = _json._jsonData;
    }
ex_:
    _json.clearPathTk();
    _json._tokens.reset();
    _json._tokens = nullptr;
    return _json._jsonData.success;
}

size_t ESPJsonArray::size()
{
    return _arrLen;
}

char *ESPJsonArray::getFloatString(float value)
{
    char *buf = _newPtr(36);
    dtostrf(value, 7, 6, buf);
    return buf;
}

char *ESPJsonArray::getIntString(int value)
{
    char *buf = _newPtr(36);
    itoa(value, buf, 10);
    return buf;
}

char *ESPJsonArray::getBoolString(bool value)
{
    char *buf = nullptr;
    if (value)
        buf = _getPGMString(ESPJson_STR_7);
    else
        buf = _getPGMString(ESPJson_STR_6);
    return buf;
}

char *ESPJsonArray::getDoubleString(double value)
{
    char *buf = _newPtr(36);
    dtostrf(value, 12, 9, buf);
    return buf;
}

void ESPJsonArray::_trimDouble(char *buf)
{
    size_t i = strlen(buf) - 1;
    while (buf[i] == '0' && i > 0)
    {
        if (buf[i - 1] == '.')
        {
            i--;
            break;
        }
        if (buf[i - 1] != '0')
            break;
        i--;
    }
    if (i < strlen(buf) - 1)
        buf[i] = '\0';
}

void ESPJsonArray::toString(String &buf, bool prettify)
{
    char *tmp = _newPtr(20);
    std::string().swap(_json._jsonData._dbuf);
    std::string().swap(_json._tbuf);
    _json._toStdString(_jbuf, false);
    _json._rawbuf = _root;
    _json._rawbuf += _jbuf;
    if (prettify)
        _json._parse(_root2, ESPJson::PRINT_MODE_PRETTY);
    else
        _json._parse(_root2, ESPJson::PRINT_MODE_PLAIN);
    std::string().swap(_json._tbuf);
    std::string().swap(_jbuf);
    _json.clearPathTk();
    _json._tokens.reset();
    _json._tokens = nullptr;
    _delPtr(tmp);
    _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
    buf = _json._jsonData._dbuf.c_str();
    std::string().swap(_json._jsonData._dbuf);
}

ESPJsonArray &ESPJsonArray::clear()
{
    _json.clear();
    std::string().swap(_jbuf);
    return *this;
}
void ESPJsonArray::_set2(int index, const char *value, bool isStr)
{
    char *tmp = _newPtr(50);
    std::string path = _brk3;
    itoa(index, tmp, 10);
    path += tmp;
    path += _brk4;
    _set(path.c_str(), value, isStr);
    std::string().swap(path);
    _delPtr(tmp);
}

void ESPJsonArray::_set(const char *path, const char *value, bool isStr)
{
    _json._jsonData.success = false;
    _json._toStdString(_jbuf, false);
    _json._rawbuf = _root;
    _json._rawbuf += _jbuf;
    char *tmp2 = _newPtr(strlen(value) + 10);
    if (isStr)
        strcpy_P(tmp2, _qt);
    strcat(tmp2, value);
    if (isStr)
        strcat_P(tmp2, _qt);
    std::string path2 = _root2;
    path2 += _slash;
    path2 += path;
    _json.clearPathTk();
    _json._strToTk(path2, _json._pathTk, '/');
    if (!_json._isArrTk(1))
        goto ex_2;
    if (_json._getArrIndex(1) < 0)
        goto ex_2;
    _json._set(path2.c_str(), tmp2);
    _delPtr(tmp2);
    std::string().swap(path2);
    if (_json._jsonData.success)
    {
        std::string().swap(_json._jsonData._dbuf);
        std::string().swap(_json._tbuf);
        _json._parse(_root2, ESPJson::PRINT_MODE_PLAIN);
        if (_json._jsonData.success)
        {
            _arrLen = _json._jsonData._len;
            _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
        }
    }
    else
        _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2);
ex_2:
    std::string().swap(_json._jsonData._dbuf);
    std::string().swap(_json._tbuf);
    std::string().swap(_jbuf);
    _json.clearPathTk();
    _json._tokens.reset();
    _json._tokens = nullptr;
}

void ESPJsonArray::set(int index)
{
    return _setNull(index);
}

void ESPJsonArray::set(const String &path)
{
    _setNull(path);
}

void ESPJsonArray::set(int index, const String &value)
{
    _setString(index, value.c_str());
}

void ESPJsonArray::set(const String &path, const String &value)
{
    _setString(path, value.c_str());
}

void ESPJsonArray::set(int index, const char *value)
{
    _setString(index, value);
}

void ESPJsonArray::set(const String &path, const char *value)
{
    _setString(path, value);
}

void ESPJsonArray::set(int index, int value)
{
    _setInt(index, value);
}

void ESPJsonArray::set(int index, unsigned short value)
{
    _setInt(index, value);
}

void ESPJsonArray::set(const String &path, int value)
{
    _setInt(path, value);
}

void ESPJsonArray::set(const String &path, unsigned short value)
{
    _setInt(path, value);
}

void ESPJsonArray::set(int index, double value)
{
    _setDouble(index, value);
}

void ESPJsonArray::set(const String &path, double value)
{
    _setDouble(path, value);
}

void ESPJsonArray::set(int index, bool value)
{
    _setBool(index, value);
}

void ESPJsonArray::set(const String &path, bool value)
{
    _setBool(path, value);
}

void ESPJsonArray::set(int index, ESPJson &json)
{
    _setJson(index, &json);
}

void ESPJsonArray::set(const String &path, ESPJson &json)
{
    _setJson(path, &json);
}

void ESPJsonArray::set(int index, ESPJsonArray &arr)
{
    _setArray(index, &arr);
}

void ESPJsonArray::set(const String &path, ESPJsonArray &arr)
{
    _setArray(path, &arr);
}

template <typename T>
void ESPJsonArray::set(int index, T value)
{
    if (std::is_same<T, int>::value)
        _setInt(index, value);
    else if (std::is_same<T, double>::value)
        _setDouble(index, value);
    else if (std::is_same<T, bool>::value)
        _setBool(index, value);
    else if (std::is_same<T, const char *>::value)
        _setString(index, value);
    else if (std::is_same<T, ESPJson &>::value)
        _setJson(index, &value);
    else if (std::is_same<T, ESPJsonArray &>::value)
        _setArray(index, &value);
}

template <typename T>
void ESPJsonArray::set(const String &path, T value)
{
    if (std::is_same<T, int>::value)
        _setInt(path, value);
    else if (std::is_same<T, double>::value)
        _setDouble(path, value);
    else if (std::is_same<T, bool>::value)
        _setBool(path, value);
    else if (std::is_same<T, const char *>::value)
        _setString(path, value);
    else if (std::is_same<T, ESPJson &>::value)
        _setJson(path, &value);
    else if (std::is_same<T, ESPJsonArray &>::value)
        _setArray(path, &value);
}

void ESPJsonArray::_setString(int index, const std::string &value)
{
    _set2(index, value.c_str(), true);
}

void ESPJsonArray::_setString(const String &path, const std::string &value)
{
    _set(path.c_str(), value.c_str(), true);
}

void ESPJsonArray::_setInt(int index, int value)
{
    char *tmp = getIntString(value);
    _set2(index, tmp, false);
    _delPtr(tmp);
}

void ESPJsonArray::_setInt(const String &path, int value)
{
    char *tmp = getIntString(value);
    _set(path.c_str(), tmp, false);
    _delPtr(tmp);
}

void ESPJsonArray::_setDouble(int index, double value)
{
    char *tmp = getDoubleString(value);
    _trimDouble(tmp);
    _set2(index, tmp, false);
    _delPtr(tmp);
}

void ESPJsonArray::_setDouble(const String &path, double value)
{
    char *tmp = getDoubleString(value);
    _trimDouble(tmp);
    _set(path.c_str(), tmp, false);
    _delPtr(tmp);
}

void ESPJsonArray::_setBool(int index, bool value)
{
    if (value)
        _set2(index, _tr, false);
    else
        _set2(index, _fls, false);
}

void ESPJsonArray::_setBool(const String &path, bool value)
{
    if (value)
        _set(path.c_str(), _tr, false);
    else
        _set(path.c_str(), _fls, false);
}

void ESPJsonArray::_setNull(int index)
{
    _set2(index, _nll, false);
}

void ESPJsonArray::_setNull(const String &path)
{
    _set(path.c_str(), _nll, false);
}

void ESPJsonArray::_setJson(int index, ESPJson *json)
{
    std::string s;
    json->_toStdString(s);
    _set2(index, s.c_str(), false);
    std::string().swap(s);
}

void ESPJsonArray::_setJson(const String &path, ESPJson *json)
{
    std::string s;
    json->_toStdString(s);
    _set(path.c_str(), s.c_str(), false);
    std::string().swap(s);
}

void ESPJsonArray::_setArray(int index, ESPJsonArray *arr)
{
    std::string s;
    arr->_toStdString(s);
    _set2(index, s.c_str(), false);
    std::string().swap(s);
}

void ESPJsonArray::_setArray(const String &path, ESPJsonArray *arr)
{
    std::string s;
    arr->_toStdString(s);
    _set(path.c_str(), s.c_str(), false);
    std::string().swap(s);
}

bool ESPJsonArray::remove(int index)
{
    char *tmp = getIntString(index);
    std::string path = "";
    path += _brk3;
    path += tmp;
    path += _brk4;
    bool ret = _remove(path.c_str());
    std::string().swap(path);
    _delPtr(tmp);
    return ret;
}

bool ESPJsonArray::remove(const String &path)
{
    return _remove(path.c_str());
}

bool ESPJsonArray::_remove(const char *path)
{
    _json._toStdString(_jbuf, false);
    _json._rawbuf = _root;
    _json._rawbuf += _jbuf;
    char *tmp2 = _newPtr(2);
    std::string path2 = _root2;
    path2 += _slash;
    path2 += path;
    _json._jsonData.success = _json.remove(path2.c_str());
    _delPtr(tmp2);
    std::string().swap(path2);
    if (_json._jsonData.success)
    {
        std::string().swap(_json._jsonData._dbuf);
        std::string().swap(_json._tbuf);
        _json._parse(_root2, ESPJson::PRINT_MODE_PLAIN);
        if (_json._jsonData.success)
        {
            _arrLen = _json._jsonData._len;
            _json._rawbuf = _json._jsonData._dbuf.substr(1, _json._jsonData._dbuf.length() - 2);
        }
    }
    else
        _json._rawbuf = _jbuf.substr(1, _jbuf.length() - 2);
    return _json._jsonData.success;
}

void ESPJsonArray::_toStdString(std::string &s)
{
    _json._toStdString(s, false);
}

ESPJsonData::ESPJsonData()
{
}

ESPJsonData::~ESPJsonData()
{
    std::string().swap(_dbuf);
}

bool ESPJsonData::getArray(ESPJsonArray &jsonArray)
{
    if (typeNum != ESPJson::JSON_ARRAY || !success)
        return false;
    char *tmp = new char[20];
    memset(tmp, 0, 20);
    char *nbuf = new char[2];
    memset(nbuf, 0, 2);
    strcpy_P(tmp, ESPJson_STR_21);
    jsonArray._json._toStdString(jsonArray._jbuf, false);
    jsonArray._json._rawbuf = tmp;
    jsonArray._json._rawbuf += stringValue.c_str();
    memset(tmp, 0, 20);
    strcpy_P(tmp, ESPJson_STR_26);
    std::string().swap(jsonArray._json._jsonData._dbuf);
    std::string().swap(jsonArray._json._tbuf);
    jsonArray._json._parse(tmp, ESPJson::PRINT_MODE_PLAIN);
    jsonArray._json._rawbuf = jsonArray._json._jsonData._dbuf.substr(1, jsonArray._json._jsonData._dbuf.length() - 2).c_str();
    jsonArray._arrLen = jsonArray._json._jsonData._len;
    delete[] tmp;
    delete[] nbuf;
    return jsonArray._json._jsonData.success;
}

bool ESPJsonData::getJSON(ESPJson &json)
{
    if (typeNum != ESPJson::JSON_OBJECT || !success)
        return false;
    json.setJsonData(stringValue);
    json._espjs_parse();
    return json._jsonData.success;
}



#endif