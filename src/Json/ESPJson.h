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
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
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

#ifndef ESPJson_H
#define ESPJson_H

#include <Arduino.h>
#include <memory>

static const char ESPJson_STR_1[] PROGMEM = ",";
static const char ESPJson_STR_2[] PROGMEM = "\"";
static const char ESPJson_STR_3[] PROGMEM = ":";
static const char ESPJson_STR_4[] PROGMEM = "%d";
static const char ESPJson_STR_5[] PROGMEM = "%f";
static const char ESPJson_STR_6[] PROGMEM = "false";
static const char ESPJson_STR_7[] PROGMEM = "true";
static const char ESPJson_STR_8[] PROGMEM = "{";
static const char ESPJson_STR_9[] PROGMEM = "}";
static const char ESPJson_STR_10[] PROGMEM = "[";
static const char ESPJson_STR_11[] PROGMEM = "]";
static const char ESPJson_STR_12[] PROGMEM = "string";
static const char ESPJson_STR_13[] PROGMEM = "int";
static const char ESPJson_STR_14[] PROGMEM = "double";
static const char ESPJson_STR_15[] PROGMEM = "bool";
static const char ESPJson_STR_16[] PROGMEM = "object";
static const char ESPJson_STR_17[] PROGMEM = "array";
static const char ESPJson_STR_18[] PROGMEM = "null";
static const char ESPJson_STR_19[] PROGMEM = "undefined";
static const char ESPJson_STR_20[] PROGMEM = ".";
static const char ESPJson_STR_21[] PROGMEM = "\"root\":";
static const char ESPJson_STR_22[] PROGMEM = "    ";
static const char ESPJson_STR_24[] PROGMEM = "\n";
static const char ESPJson_STR_25[] PROGMEM = ": ";
static const char ESPJson_STR_26[] PROGMEM = "root";
static const char ESPJson_STR_27[] PROGMEM = "/";

class ESPJson;
class ESPJsonArray;

class ESPJsonData
{
    friend class ESPJson;
    friend class ESPJsonArray;

public:
    ESPJsonData();
    ~ESPJsonData();

    /*
    Get array data as ESPJsonArray object from ESPJsonData object.
    
    @param jsonArray - The returning ESPJsonArray object.

    @return bool status for successful operation.

    This should call after parse or get function.

   */
    bool getArray(ESPJsonArray &jsonArray);

    /*
    Get array data as ESPJson object from ESPJsonData object.
    
    @param jsonArray - The returning ESPJson object.

    @return bool status for successful operation.

    This should call after parse or get function.

   */
    bool getJSON(ESPJson &json);

    /*
    The String value of parses data.
   */
    String stringValue = "";

    /*
    The int value of parses data.
   */
    int intValue = 0;

    /*
    The double value of parses data.
   */
    double doubleValue = 0.0;

    /*
    The bool value of parses data.
   */
    bool boolValue = false;

    /*
    The type String of parses data.
   */
    String type = "";

    /*
    The type (number) of parses data.
   */
    uint8_t typeNum = 0;

    /*
    The success flag of parsing data.
   */
    bool success = false;

private:
    int _type = 0;
    int _k_start = 0;
    int _start = 0;
    int _end = 0;
    int _tokenIndex = 0;
    int _depth = 0;
    int _len = 0;
    std::string _dbuf = "";
};

class ESPJson
{
    friend class ESPJsonArray;
    friend class ESPJsonData;

public:
    typedef enum
    {
        JSON_UNDEFINED = 0,
        JSON_OBJECT = 1,
        JSON_ARRAY = 2,
        JSON_STRING = 3,
        JSON_INT = 4,
        JSON_DOUBLE = 5,
        JSON_BOOL = 6,
        JSON_NULL = 7
    } jsonDataType;

    typedef enum
    {
        PRINT_MODE_NONE = -1,
        PRINT_MODE_PLAIN = 0,
        PRINT_MODE_PRETTY = 1
    } PRINT_MODE;

    typedef struct
    {
        bool matched = false;
        std::string tk = "";
    } path_tk_t;

    typedef struct
    {
        int index;
        bool firstTk;
        bool lastTk;
        bool success;
    } single_child_parent_t;

    typedef struct
    {
        uint16_t index;
        uint8_t type;
    } eltk_t;

    typedef struct
    {
        uint16_t index;
        uint8_t type;
        uint16_t olen;
        uint16_t oindex;
        int depth;
        bool omark;
        bool ref;
        bool skip;
    } el_t;

    typedef struct
    {
        int index;
        uint16_t oindex;
        uint16_t olen;
        uint8_t type;
        int depth;
        bool omark;
        bool ref;
        bool skip;
    } tk_index_t;

    /**
    * JSON type identifier. Basic types are:
    * 	o Object
    * 	o Array
    * 	o String
    * 	o Other primitive: number, boolean (true/false) or null
    */
    typedef enum
    {
        JSMN_UNDEFINED = 0,
        JSMN_OBJECT = 1,
        JSMN_ARRAY = 2,
        JSMN_STRING = 3,
        JSMN_PRIMITIVE = 4
    } espjs_type_t;

    enum espjs_err
    {
        /* Not enough tokens were provided */
        JSMN_ERROR_NOMEM = -1,
        /* Invalid character inside JSON string */
        JSMN_ERROR_INVAL = -2,
        /* The string is not a full JSON packet, more bytes expected */
        JSMN_ERROR_PART = -3
    };

    /**
    * JSON token description.
    * type		type (object, array, string etc.)
    * start	start position in JSON data string
    * end		end position in JSON data string
    */
    typedef struct
    {
        espjs_type_t type;
        int start;
        int end;
        int size;
#ifdef JSMN_PARENT_LINKS
        int parent;
#endif
    } espjs_tok_t;

    /**
    * JSON parser. Contains an array of token blocks available. Also stores
    * the string being parsed now and current position in that string
    */
    typedef struct
    {
        unsigned int pos;     /* offset in the JSON string */
        unsigned int toknext; /* next token to allocate */
        int toksuper;         /* superior token node, e.g parent object or array */
    } espjs_parser;

    ESPJson();
    ESPJson(std::string &data);
    ~ESPJson();

    /*
    Clear internal buffer of ESPJson object.z
    
    @return instance of an object.

   */
    ESPJson &clear();

    /*
    Set JSON data (JSON object string) to ESPJson object.
    
    @param data - The JSON object string.

    @return instance of an object.

   */
    ESPJson &setJsonData(const String &data);

    /*
    Add null to ESPJson object.
    
    @param key - The new key string that null to be added.

    @return instance of an object.

   */
    ESPJson &add(const String &key);

    /*
    Add string to ESPJson object.
    
    @param key - The new key string that string value to be added.

    @param value - The string value for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, const String &value);

    /*
    Add string (chars array) to ESPJson object.
    
    @param key - The new key string that string (chars array) value to be added.

    @param value - The char array for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, const char *value);

    /*
    Add integer/unsigned short to ESPJson object.
    
    @param key - The new key string in which value to be added.

    @param value - The integer/unsigned short value for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, int value);
    ESPJson &add(const String &key, unsigned short value);

    /*
    Add float to ESPJson object.
    
    @param key - The new key string that double value to be added.

    @param value - The double value for the new specified key.

    @return instance of an object.

   */

    ESPJson &add(const String &key, float value);

    /*
    Add double to ESPJson object.
    
    @param key - The new key string that double value to be added.

    @param value - The double value for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, double value);

    /*
    Add boolean to ESPJson object.
    
    @param key - The new key string that bool value to be added.

    @param value - The boolean value for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, bool value);

    /*
    Add nested ESPJson object into ESPJson object.
    
    @param key - The new key string that ESPJson object to be added.

    @param json - The ESPJson object for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, ESPJson &json);

    /*
    Add nested ESPJsonArray object into ESPJson object.
    
    @param key - The new key string that ESPJsonArray object to be added.

    @param arr - The ESPJsonArray for the new specified key.

    @return instance of an object.

   */
    ESPJson &add(const String &key, ESPJsonArray &arr);

    /*
    Get the ESPJson object serialized string.

    @param buf - The returning String object. 

    @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline. 

   */
    void toString(String &buf, bool prettify = false);

    /*
    Get the value from the specified node path in ESPJson object.

    @param jsonData - The returning ESPJsonData that holds the returned data.

    @param path - Relative path to the specific node in ESPJson object.

    @param prettify - The bool flag for a prettifying string in ESPJsonData's stringValue.

    @return boolean status of the operation.

    The ESPJsonData object hold the returned data which can be read from the following properties

    jsonData.stringValue - contains the returned string.

    jsonData.intValue - contains the returned integer value.

    jsonData.doubleValue - contains the returned double value.

    jsonData.boolValue - contains the returned boolean value.

    jsonData.success - used to determine the result of Firebase.get operation.

    jsonData.type - used to determine the type of returned value in string represent 
    the types of value e.g. string, int, double, boolean, array, object, null and undefined.

    jsonData.typeNum - used to determine the type of returned value is an integer as represented by the following value.
    
    JSON_UNDEFINED = 0
    JSON_OBJECT = 1
    JSON_ARRAY = 2
    JSON_STRING = 3
    JSON_INT = 4
    JSON_DOUBLE = 5
    JSON_BOOL = 6 and
    JSON_NULL = 7

   */
    bool get(ESPJsonData &jsonData, const String &path, bool prettify = false);

    /*
    Parse and collect all node/array elements in ESPJson object.

    @param data - The JSON data string to parse (optional to replace the internal buffer with new data).

    @return number of child/array elements in ESPJson object.

   */
    size_t iteratorBegin(const char *data = NULL);

    /*
    Get child/array elements from ESPJson objects at specified index.
    
    @param index - The element index to get.

    @param type - The integer which holds the type of data i.e. JSON_OBJECT and JSON_ARR

    @param key - The string which holds the key/name of an object, can return empty String if the data type is an array.

    @param value - The string which holds the value for the element key or array.   

   */
    void iteratorGet(size_t index, int &type, String &key, String &value);

    /*
    Clear all iterator buffer (should be called since iteratorBegin was called).

   */
    void iteratorEnd();

    /*
    Set null to ESPJson object at the specified node path.
    
    @param path - The relative path that null to be set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path);

    /*
    Set String value to ESPJson object at the specified node path.
    
    @param path - The relative path that string value to be set.

    @param value - The string value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, const String &value);

    /*
    Set string (chars array) value to ESPJson object at the specified node path.
    
    @param path - The relative path that string (chars array) to be set.

    @param value - The char array to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, const char *value);

    /*
    Set integer/unsigned short value to ESPJson object at specified node path.
    
    @param path - The relative path that int value to be set.

    @param value - The integer/unsigned short value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, int value);
    void set(const String &path, unsigned short value);

    /*
    Set the double value to ESPJson object at the specified node path.
    
    @param path - The relative path that double value to be set.

    @param value - The double value to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, double value);

    /*
    Set boolean value to ESPJson object at the specified node path.
    
    @param path - The relative path that bool value to be set.

    @param value - The boolean value to set.


    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, bool value);

    /*
    Set nested ESPJson object to ESPJson object at the specified node path.
    
    @param path - The relative path that nested ESPJson object to be set.

    @param json - The ESPJson object to set.

    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, ESPJson &json);

    /*
    Set nested ESPJsonAtrray object to ESPJson object at specified node path.
    
    @param path - The relative path that nested ESPJsonAtrray object to be set.

    @param arr - The ESPJsonAtrray object to set.


    The relative path can be mixed with array index (number placed inside square brackets) and node names 
    e.g. /myRoot/[2]/Sensor1/myData/[3].

   */
    void set(const String &path, ESPJsonArray &arr);

    /*
    Remove the specified node and its content.

    @param path - The relative path to remove its contents/children.

    @return bool value represents the success operation.
    */
    bool remove(const String &path);

    template <typename T>
    ESPJson &add(const String &key, T value);
    template <typename T>
    bool set(const String &path, T value);

private:
    int _nextToken = 0;
    int _refToken = -1;
    int _nextDepth = 0;
    int _parentIndex = -1;
    int _parseDepth = 0;
    int _skipDepth = -1;
    int _parseCompleted = -1;
    int _refTkIndex = -1;
    int _remTkIndex = -1;
    int _tokenCount = 0;
    bool _TkRefOk = false;
    bool _tokenMatch = false;
    bool _remFirstTk = false;
    bool _remLastTk = false;
    bool _collectTk = false;
    bool _paresRes = false;
    bool _arrReplaced = false;
    bool _arrInserted = false;

    char *_qt = nullptr;
    char *_tab = nullptr;
    char *_brk1 = nullptr;
    char *_brk2 = nullptr;
    char *_brk3 = nullptr;
    char *_brk4 = nullptr;
    char *_cm = nullptr;
    char *_nl = nullptr;
    char *_nll = nullptr;
    char *_pr = nullptr;
    char *_pr2 = nullptr;
    char *_pd = nullptr;
    char *_pf = nullptr;
    char *_fls = nullptr;
    char *_tr = nullptr;
    char *_string = nullptr;
    char *_int = nullptr;
    char *_dbl = nullptr;
    char *_bl = nullptr;
    char *_obj = nullptr;
    char *_arry = nullptr;
    char *_undef = nullptr;
    char *_dot = nullptr;

    std::string _rawbuf = "";
    std::string _tbuf = "";
    tk_index_t _lastTk;
    std::vector<path_tk_t> _pathTk = std::vector<path_tk_t>();
    std::vector<eltk_t> _eltk = std::vector<eltk_t>();
    std::vector<el_t> _el = std::vector<el_t>();
    ESPJsonData _jsonData;

    std::shared_ptr<espjs_parser> _parser = std::shared_ptr<espjs_parser>(new espjs_parser());
    std::shared_ptr<espjs_tok_t> _tokens = nullptr;

    void _init();
    void _finalize();
    ESPJson &_setJsonData(std::string &data);
    ESPJson &_add(const char *key, const char *value, size_t klen, size_t vlen, bool isString = true, bool isJson = true);
    ESPJson &_addArrayStr(const char *value, size_t len, bool isString);
    void _resetParseResult();
    void _setElementType();
    void _addString(const std::string &key, const std::string &value);
    void _addArray(const std::string &key, ESPJsonArray *arr);
    void _addInt(const std::string &key, int value);
    void _addDouble(const std::string &key, double value);
    void _addBool(const std::string &key, bool value);
    void _addNull(const std::string &key);
    void _addJson(const std::string &key, ESPJson *json);
    void _setString(const std::string &path, const std::string &value);
    void _setInt(const std::string &path, int value);
    void _setDouble(const std::string &path, double value);
    void _setBool(const std::string &path, bool value);
    void _setNull(const std::string &path);
    void _setJson(const std::string &path, ESPJson *json);
    void _setArray(const std::string &path, ESPJsonArray *arr);
    void _set(const char *path, const char *data);
    void clearPathTk();
    void _parse(const char *path, PRINT_MODE printMode);
    void _parse(const char *key, int depth, int index, PRINT_MODE printMode);
    void _compile(const char *key, int depth, int index, const char *replace, PRINT_MODE printMode, int refTokenIndex = -1, bool removeTk = false);
    void _remove(const char *key, int depth, int index, const char *replace, int refTokenIndex = -1, bool removeTk = false);
    void _espjs_parse(bool collectTk = false);
    bool _updateTkIndex(uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode, bool advanceCount);
    bool _updateTkIndex2(std::string &str, uint16_t index, int &depth, char *searchKey, int searchIndex, char *replace, PRINT_MODE printMode);
    bool _updateTkIndex3(uint16_t index, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode);
    void _getTkIndex(int depth, tk_index_t &tk);
    void _setMark(int depth, bool mark);
    void _setSkip(int depth, bool skip);
    void _setRef(int depth, bool ref);
    void _insertChilds(char *data, PRINT_MODE printMode);
    void _addObjNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode);
    void _addArrNodes(std::string &str, std::string &str2, int index, char *data, PRINT_MODE printMode);
    void _compileToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex = -1, bool removeTk = false);
    void _parseToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode);
    void _removeToken(uint16_t &i, char *buf, int &depth, char *searchKey, int searchIndex, PRINT_MODE printMode, char *replace, int refTokenIndex = -1, bool removeTk = false);
    single_child_parent_t _findSCParent(int depth);
    bool _isArrTk(int index);
    bool _isStrTk(int index);
    int _getArrIndex(int index);
    char *getFloatString(float value);
    char *getDoubleString(double value);
    char *getIntString(int value);
    char *getBoolString(bool value);
    char *getPGMString(PGM_P pgm);
    void _trimDouble(char *buf);
    void _get(const char *key, int depth, int index = -1);
    void _ltrim(std::string &str, const std::string &chars = " ");
    void _rtrim(std::string &str, const std::string &chars = " ");
    void _trim(std::string &str, const std::string &chars = " ");
    void _toStdString(std::string &s, bool isJson = true);
    void _strToTk(const std::string &str, std::vector<path_tk_t> &tk, char delim);
    int _strpos(const char *haystack, const char *needle, int offset);
    int _rstrpos(const char *haystack, const char *needle, int offset);
    char *_rstrstr(const char *haystack, const char *needle);
    void _delPtr(char *p);
    char *_newPtr(size_t len);
    char *_newPtr(char *p, size_t len);
    char *_newPtr(char *p, size_t len, char *d);
    char *_getPGMString(PGM_P pgm);

    void espjs_init(espjs_parser *parser);
    int espjs_parse(espjs_parser *parser, const char *js, size_t len,
                   espjs_tok_t *tokens, unsigned int num_tokens);
    int espjs_parse_string(espjs_parser *parser, const char *js,
                          size_t len, espjs_tok_t *tokens, size_t num_tokens);
    int espjs_parse_primitive(espjs_parser *parser, const char *js,
                             size_t len, espjs_tok_t *tokens, size_t num_tokens);
    void espjs_fill_token(espjs_tok_t *token, espjs_type_t type,
                         int start, int end);
    espjs_tok_t *espjs_alloc_token(espjs_parser *parser,
                                 espjs_tok_t *tokens, size_t num_tokens);
};

class ESPJsonArray
{

public:
    ESPJsonArray();
    ~ESPJsonArray();
    void _init();
    void _finalize();

    friend class ESPJson;
    friend class ESPJsonData;

    /*
    Add null to ESPJsonArray object.

    @return instance of an object.

   */
    ESPJsonArray &add();

    /*
    Add string to ESPJsonArray object.

    @param value - The string value to add.

    @return instance of an object.

   */
    ESPJsonArray &add(const String &value);

    /*
    Add string (chars arrar) to ESPJsonArray object.

    @param value - The char array to add.

    @return instance of an object.

   */
    ESPJsonArray &add(const char *value);

    /*
    Add integer/unsigned short to ESPJsonArray object.

    @param value - The integer/unsigned short value to add.

    @return instance of an object.

   */
    ESPJsonArray &add(int value);
    ESPJsonArray &add(unsigned short value);

    /*
    Add double to ESPJsonArray object.

    @param value - The double value to add.

    @return instance of an object.

   */
    ESPJsonArray &add(double value);

    /*
    Add boolean to ESPJsonArray object.

    @param value - The boolean value to add.

    @return instance of an object.

   */
    ESPJsonArray &add(bool value);

    /*
    Add nested ESPJson object  to ESPJsonArray object.

    @param json - The ESPJson object to add.

    @return instance of an object.

   */
    ESPJsonArray &add(ESPJson &json);

    /*
    Add nested ESPJsonArray object  to ESPJsonArray object.

    @param arr - The ESPJsonArray object to add.

    @return instance of an object.

   */
    ESPJsonArray &add(ESPJsonArray &arr);

    /*
    Get the array value at the specified index from the ESPJsonArray object.

    @param jsonData - The returning ESPJsonData object that holds data at the specified index.

    @param index - Index of data in ESPJsonArray object.    

    @return boolean status of the operation.

   */
    bool get(ESPJsonData &jsonData, int index);
    bool get(ESPJsonData *jsonData, int index);

    /*
    Get the array value at the specified path from ESPJsonArray object.

    @param jsonData - The returning ESPJsonData object that holds data at the specified path.

    @param path - Relative path to data in ESPJsonArray object.    

    @return boolean status of the operation.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2

   */
    bool get(ESPJsonData &jsonData, const String &path);

    /*
    Get the length of the array in ESPJsonArray object.  

    @return length of the array.

   */
    size_t size();

    /*
    Get the ESPJsonArray object serialized string.

    @param buf - The returning String object. 

    @param prettify - Boolean flag for return the pretty format string i.e. with text indentation and newline. 

   */
    void toString(String &buf, bool prettify = false);

    /*
    Clear all array in ESPJsonArray object.

    @return instance of an object.

   */
    ESPJsonArray &clear();

    /*
    Set null to ESPJsonArray object at specified index.
    
    @param index - The array index that null to be set.

   */
    void set(int index);

    /*
    Set String to ESPJsonArray object at the specified index.
    
    @param index - The array index that String value to be set.

    @param value - The String to set.

   */
    void set(int index, const String &value);

    /*
    Set string (chars array) to ESPJsonArray object at specified index.
    
    @param index - The array index that string (chars array) to be set.

    @param value - The char array to set.

   */
    void set(int index, const char *value);

    /*
    Set integer/unsigned short value to ESPJsonArray object at specified index.
    
    @param index - The array index that int/unsigned short to be set.

    @param value - The integer/unsigned short value to set.

   */
    void set(int index, int value);
    void set(int index, unsigned short value);

    /*
    Set double value to ESPJsonArray object at specified index.
    
    @param index - The array index that double value to be set.

    @param value - The double value to set.

   */
    void set(int index, double value);

    /*
    Set boolean value to ESPJsonArray object at specified index.
    
    @param index - The array index that bool value to be set.

    @param value - The boolean value to set.

   */
    void set(int index, bool value);

    /*
    Set nested ESPJson object to ESPJsonArray object at specified index.
    
    @param index - The array index that nested ESPJson object to be set.

    @param value - The ESPJson object to set.

   */
    void set(int index, ESPJson &json);

    /*
    Set nested ESPJsonArray object to ESPJsonArray object at specified index.
    
    @param index - The array index that nested ESPJsonArray object to be set.

    @param value - The ESPJsonArray object to set.

   */
    void set(int index, ESPJsonArray &arr);

    /*
    Set null to ESPJson object at the specified path.
    
    @param path - The relative path that null to be set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path);

    /*
    Set String to ESPJsonArray object at the specified path.
    
    @param path - The relative path that string value to be set.

    @param value - The String to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, const String &value);

    /*
    Set string (chars array) to ESPJsonArray object at the specified path.
    
    @param path - The relative path that string (chars array) value to be set.

    @param value - The char array to set.

    The relative path must begin with array index (number places inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, const char *value);

    /*
    Set integer/unsigned short value to ESPJsonArray object at specified path.
    
    @param path - The relative path that integer/unsigned short value to be set.

    @param value - The integer value to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, int value);
    void set(const String &path, unsigned short value);

    /*
    Set double value to ESPJsonArray object at specified path.
    
    @param path - The relative path that double value to be set.

    @param value - The double to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, double value);

    /*
    Set boolean value to ESPJsonArray object at specified path.
    
    @param path - The relative path that bool value to be set.

    @param value - The boolean value to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, bool value);

    /*
    Set the nested ESPJson object to ESPJsonArray object at the specified path.
    
    @param path - The relative path that nested ESPJson object to be set.

    @param value - The ESPJson object to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, ESPJson &json);

    /*
    Set the nested ESPJsonArray object to ESPJsonArray object at specified path.
    
    @param path - The relative path that nested ESPJsonArray object to be set.

    @param value - The ESPJsonArray object to set.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would get the data from myData key inside the array indexes 2.

   */
    void set(const String &path, ESPJsonArray &arr);

    /*
    Remove the array value at the specified index from the ESPJsonArray object.

    @param index - The array index to be removed.

    @return bool value represents the successful operation.

    */
    bool remove(int index);

    /*
    Remove the array value at the specified path from ESPJsonArray object.

    @param path - The relative path to array in ESPJsonArray object to be removed.

    @return bool value represents the successful operation.

    The relative path must begin with array index (number placed inside square brackets) followed by 
    other array indexes or node names e.g. /[2]/myData would remove the data of myData key inside the array indexes 2.
    
    */
    bool remove(const String &path);

    template <typename T>
    void set(int index, T value);
    template <typename T>
    void set(const String &path, T value);
    template <typename T>
    ESPJsonArray &add(T value);

private:
    std::string _jbuf = "";
    ESPJson _json;
    size_t _arrLen = 0;
    char *_pd = nullptr;
    char *_pf = nullptr;
    char *_fls = nullptr;
    char *_tr = nullptr;
    char *_brk3 = nullptr;
    char *_brk4 = nullptr;
    char *_nll = nullptr;
    char *_root = nullptr;
    char *_root2 = nullptr;
    char *_qt = nullptr;
    char *_slash = nullptr;

    void _addString(const std::string &value);
    void _addInt(int value);
    void _addDouble(double value);
    void _addBool(bool value);
    void _addNull();
    void _addJson(ESPJson *json);
    void _addArray(ESPJsonArray *arr);
    void _setString(int index, const std::string &value);
    void _setString(const String &path, const std::string &value);
    void _setInt(int index, int value);
    void _setInt(const String &path, int value);
    void _setDouble(int index, double value);
    void _setDouble(const String &path, double value);
    void _setBool(int index, bool value);
    void _setBool(const String &path, bool value);
    void _setNull(int index);
    void _setNull(const String &path);
    void _setJson(int index, ESPJson *json);
    void _setJson(const String &path, ESPJson *json);
    void _setArray(int index, ESPJsonArray *arr);
    void _setArray(const String &path, ESPJsonArray *arr);
    void _toStdString(std::string &s);
    void _set2(int index, const char *value, bool isStr = true);
    void _set(const char *path, const char *value, bool isStr = true);
    bool _get(ESPJsonData &jsonData, const char *path);
    bool _remove(const char *path);
    void _trimDouble(char *buf);
    char *getFloatString(float value);
    char *getDoubleString(double value);
    char *getIntString(int value);
    char *getBoolString(bool value);
    char *_getPGMString(PGM_P pgm);
    int _strpos(const char *haystack, const char *needle, int offset);
    int _rstrpos(const char *haystack, const char *needle, int offset);
    char *_rstrstr(const char *haystack, const char *needle);
    void _delPtr(char *p);
    char *_newPtr(size_t len);
    char *_newPtr(char *p, size_t len);
    char *_newPtr(char *p, size_t len, char *d);
};

#endif