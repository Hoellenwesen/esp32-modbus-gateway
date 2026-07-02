#include "config.h"

static inline uint8_t encodeDataBits(uint8_t value) {
    return ((value - 5) << 2) & 0xc;
}

static inline uint8_t encodeStopBits(uint8_t value) {
    return (value << 4) & 0x30;
}

Config::Config()
    :_prefs(NULL)
    ,_tcpPort(502)
    ,_tcpTimeout(10000)
    ,_modbusBaudRate(9600)
    ,_modbusConfig(SERIAL_8N1)
    ,_modbusRtsPin(-1)
    ,_serialBaudRate(115200)
    ,_serialConfig(SERIAL_8N1)
    ,_webPassword("")
    ,_bridgeEnabled(true)
    ,_dirty(false)
{}

void Config::begin(Preferences *prefs)
{
    _prefs = prefs;
    _tcpPort = _prefs->getUShort("tcpPort", _tcpPort);
    _tcpTimeout = _prefs->getULong("tcpTimeout", _tcpTimeout);
    _modbusBaudRate = _prefs->getULong("modbusBaudRate", _modbusBaudRate);
    _modbusConfig = _prefs->getULong("modbusConfig", _modbusConfig);
    _modbusRtsPin = _prefs->getChar("modbusRtsPin", _modbusRtsPin);
    _serialBaudRate = _prefs->getULong("serialBaudRate", _serialBaudRate);
    _serialConfig = _prefs->getULong("serialConfig", _serialConfig);
    _webPassword = _prefs->getString("webPassword", _webPassword);
    _bridgeEnabled = _prefs->getBool("bridgeEn", _bridgeEnabled);
}

uint16_t Config::getTcpPort(){
    return _tcpPort;
}

void Config::setTcpPort(uint16_t value){
    if (_tcpPort == value) return;
    _tcpPort = value;
    _dirty = true;
}

uint32_t Config::getTcpTimeout(){
    return _tcpTimeout;
}

void Config::setTcpTimeout(uint32_t value){
    if (_tcpTimeout == value) return;
    _tcpTimeout = value;
    _dirty = true;
}

uint32_t Config::getModbusConfig(){
    return _modbusConfig;
}

uint32_t Config::getModbusBaudRate(){
    return _modbusBaudRate;
}

void Config::setModbusBaudRate(uint32_t value){
    if (_modbusBaudRate == value) return;
    _modbusBaudRate = value;
    _dirty = true;
}

uint8_t Config::getModbusDataBits(){
    return ((_modbusConfig & 0xc) >> 2) + 5;
}

void Config::setModbusDataBits(uint8_t value){
    uint8_t encoded = encodeDataBits(value);
    if ((_modbusConfig & 0xc) == encoded) return;
    _modbusConfig = (_modbusConfig & 0xfffffff3) | encoded;
    _dirty = true;
}

uint8_t Config::getModbusParity(){
    return _modbusConfig & 0x3;
}

void Config::setModbusParity(uint8_t value){
    value &= 0x3;
    if ((_modbusConfig & 0x3) == value) return;
    _modbusConfig = (_modbusConfig & 0xfffffffc) | value;
    _dirty = true;
}

uint8_t Config::getModbusStopBits(){
    return (_modbusConfig & 0x30) >> 4;
}

void Config::setModbusStopBits(uint8_t value){
    uint8_t encoded = encodeStopBits(value);
    if ((_modbusConfig & 0x30) == encoded) return;
    _modbusConfig = (_modbusConfig & 0xffffffcf) | encoded;
    _dirty = true;
}

int8_t Config::getModbusRtsPin(){
    return _modbusRtsPin;
}

void Config::setModbusRtsPin(int8_t value){
    if (_modbusRtsPin == value) return;
    _modbusRtsPin = value;
    _dirty = true;
}

uint32_t Config::getSerialConfig(){
    return _serialConfig;
}

uint32_t Config::getSerialBaudRate(){
    return _serialBaudRate;
}

void Config::setSerialBaudRate(uint32_t value){
    if (_serialBaudRate == value) return;
    _serialBaudRate = value;
    _dirty = true;
}

uint8_t Config::getSerialDataBits(){
    return ((_serialConfig & 0xc) >> 2) + 5;
}

void Config::setSerialDataBits(uint8_t value){
    uint8_t encoded = encodeDataBits(value);
    if ((_serialConfig & 0xc) == encoded) return;
    _serialConfig = (_serialConfig & 0xfffffff3) | encoded;
    _dirty = true;
}

uint8_t Config::getSerialParity(){
    return _serialConfig & 0x3;
}

void Config::setSerialParity(uint8_t value){
    value &= 0x3;
    if ((_serialConfig & 0x3) == value) return;
    _serialConfig = (_serialConfig & 0xfffffffc) | value;
    _dirty = true;
}

uint8_t Config::getSerialStopBits(){
    return (_serialConfig & 0x30) >> 4;
}

void Config::setSerialStopBits(uint8_t value){
    uint8_t encoded = encodeStopBits(value);
    if ((_serialConfig & 0x30) == encoded) return;
    _serialConfig = (_serialConfig & 0xffffffcf) | encoded;
    _dirty = true;
}


String Config::getWebPassword(){
    return _webPassword;
}

void Config::setWebPassword(String value){
    if (_webPassword == value) return;
    _webPassword = value;
    _dirty = true;
}

void Config::setBridgeEnabled(bool value){
    if (_bridgeEnabled == value) return;
    _bridgeEnabled = value;
    _dirty = true;
}

bool Config::getBridgeEnabled(){
    return _bridgeEnabled;
}

void Config::save() {
    if (!_prefs || !_dirty) return;
    _prefs->putUShort("tcpPort", _tcpPort);
    _prefs->putULong("tcpTimeout", _tcpTimeout);
    _prefs->putULong("modbusBaudRate", _modbusBaudRate);
    _prefs->putULong("modbusConfig", _modbusConfig);
    _prefs->putChar("modbusRtsPin", _modbusRtsPin);
    _prefs->putULong("serialBaudRate", _serialBaudRate);
    _prefs->putULong("serialConfig", _serialConfig);
    _prefs->putString("webPassword", _webPassword);
    _prefs->putBool("bridgeEn", _bridgeEnabled);
    _dirty = false;
}
