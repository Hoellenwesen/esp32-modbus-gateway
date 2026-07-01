#include "debug.h"

WebPrint::WebPrint(Print *serial, AsyncResponseStream *response)
    :_serial(serial)
    ,_response(response)
{}

size_t WebPrint::write(uint8_t arg){
    if (arg == '&'){
        _response->print("&amp;");
    }
    else if (arg == '<'){
        _response->print("&lt;");
    }
    else if (arg == '>'){
        _response->print("&gt;");
    }
    else{
        _response->print((char)arg);
    }
    return _serial->write(arg);
}

size_t WebPrint::write(const uint8_t *buffer, size_t size){
    String text((const char*)buffer, size);
    text.replace("&", "&amp;");
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");
    _response->print(text);
    return _serial->write(buffer, size);
}

String WebPrint::escape(String text, char oldValue, const String& newValue){
    int pos = text.indexOf(oldValue);
    if (pos < 0) return text;
    String result;
    result.reserve(text.length() + 32);
    int last = 0;
    while (pos >= 0) {
        result += text.substring(last, pos) + newValue;
        last = pos + 1;
        pos = text.indexOf(oldValue, last);
    }
    result += text.substring(last);
    return result;
}