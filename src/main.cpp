#include <WiFi.h>
#include <AsyncTCP.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <Logging.h>
#include <ModbusBridgeWiFi.h>
#include <ModbusClientRTU.h>
#include "config.h"
#include "pages.h"

AsyncWebServer webServer(80);
Config config;
Preferences prefs;
ModbusClientRTU *MBclient;
ModbusBridgeWiFi MBbridge;
WiFiManager wm;
bool configChanged = false;
uint32_t lastTelemetry = 0;
const uint32_t TELEMETRY_INTERVAL = 3600000;

void setup() {
  debugSerial.begin(115200);
  dbgln();
  dbgln("[config] load")
  prefs.begin("modbusRtuGw");
  config.begin(&prefs);
  if (config.getWebPassword().isEmpty()) {
    debugSerial.println();
    debugSerial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    debugSerial.println("! WARNING: No web password set!                  !");
    debugSerial.println("! Set one via the Config page to secure the web UI.");
    debugSerial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }
  debugSerial.end();
  debugSerial.begin(config.getSerialBaudRate(), config.getSerialConfig());
  dbgln("[wifi] start");
  WiFi.mode(WIFI_STA);
  wm.setClass("invert");
  auto reboot = false;
  wm.setAPCallback([&reboot](WiFiManager *wifiManager){reboot = true;});
  wm.autoConnect();
  if (reboot){
    ESP.restart();
  }
  dbgln("[wifi] finished");
  dbgln("[modbus] start");

  MBUlogLvl = LOG_LEVEL_WARNING;
  RTUutils::prepareHardwareSerial(modbusSerial);
#if defined(RX_PIN) && defined(TX_PIN)
  // use rx and tx-pins if defined in platformio.ini
  modbusSerial.begin(config.getModbusBaudRate(), config.getModbusConfig(), RX_PIN, TX_PIN );
  dbgln("Use user defined RX/TX pins");
#else
  // otherwise use default pins for hardware-serial2
  modbusSerial.begin(config.getModbusBaudRate(), config.getModbusConfig());
#endif

  MBclient = new ModbusClientRTU(config.getModbusRtsPin());
  MBclient->setTimeout(1000);
  MBclient->begin(modbusSerial, MODBUS_QUEUE_DEPTH);
  for (uint8_t i = 1; i < 248; i++)
  {
    MBbridge.attachServer(i, i, ANY_FUNCTION_CODE, MBclient);
  }
  if (config.getBridgeEnabled()) {
    MBbridge.start(config.getTcpPort(), 10, config.getTcpTimeout());
    dbgln("[modbus] bridge started");
  } else {
    dbgln("[modbus] bridge disabled by config");
  }
  dbgln("[modbus] finished");
  setupPages(&webServer, MBclient, &MBbridge, &config, &wm, &configChanged);
  webServer.begin();
  if (MDNS.begin(config.getHostname().c_str())) {
    MDNS.addService("http", "tcp", 80);
    dbg("[mdns] started at "); dbg(config.getHostname()); dbgln(".local");
  } else {
    dbgln("[mdns] failed to start");
  }
  dbgln("[setup] finished");
}

void loop() {
  esp_task_wdt_reset();

  if (configChanged) {
    configChanged = false;
    dbgln("[system] config changed, rebooting...");
    delay(100);
    prefs.end();
    ESP.restart();
  }

  uint32_t now = millis();
  if (lastTelemetry == 0 || now - lastTelemetry >= TELEMETRY_INTERVAL) {
    lastTelemetry = now;
    dbg("[telemetry] uptime="); dbg(now / 1000);
    dbg("s rssi="); dbg(WiFi.RSSI());
    dbg("dBm ip="); dbg(WiFi.localIP().toString());
    dbg(" hostname="); dbg(config.getHostname());
    dbgln("");
  }

  delay(10);
}