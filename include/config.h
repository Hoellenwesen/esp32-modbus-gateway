#ifndef CONFIG_H
    #define CONFIG_H
    #include <Arduino.h>
    #include <Preferences.h>
    #define debugSerial Serial
    #define modbusSerial Serial2
    #define MODBUS_QUEUE_DEPTH 1

    class Config{
        private:
            Preferences *_prefs;
            uint16_t _tcpPort;
            uint32_t _tcpTimeout;
            uint32_t _modbusBaudRate;
            uint32_t _modbusConfig;
            int8_t _modbusRtsPin;
            uint32_t _serialBaudRate;
            uint32_t _serialConfig;
            String _webPassword;
            bool _bridgeEnabled;
            String _hostname;
            bool _dirty;
        public:
            Config();
            void begin(Preferences *prefs);
            void save();
            uint16_t getTcpPort();
            void setTcpPort(uint16_t value);
            uint32_t getTcpTimeout();
            void setTcpTimeout(uint32_t value);
            uint32_t getModbusConfig();
            uint32_t getModbusBaudRate();
            void setModbusBaudRate(uint32_t value);
            uint8_t getModbusDataBits();
            void setModbusDataBits(uint8_t value);
            uint8_t getModbusParity();
            void setModbusParity(uint8_t value);
            uint8_t getModbusStopBits();
            void setModbusStopBits(uint8_t value);
            int8_t getModbusRtsPin();
            void setModbusRtsPin(int8_t value);
            uint32_t getSerialConfig();
            uint32_t getSerialBaudRate();
            void setSerialBaudRate(uint32_t value);
            uint8_t getSerialDataBits();
            void setSerialDataBits(uint8_t value);
            uint8_t getSerialParity();
            void setSerialParity(uint8_t value);
            uint8_t getSerialStopBits();
            void setSerialStopBits(uint8_t value);
            String getWebPassword();
            void setWebPassword(String value);
            bool getBridgeEnabled();
            void setBridgeEnabled(bool value);
            String getHostname();
            void setHostname(String value);
    };
    #ifdef ENABLE_DEBUG
    #define dbg(x...) debugSerial.print(x);
    #define dbgln(x...) debugSerial.println(x);
    #else
    #define dbg(x...) ;
    #define dbgln(x...) ;
    #endif
#endif /* CONFIG_H */