#include "AirNgin.h"
#include "Arduino.h"


bool _MqttCon_IsConnected = false;
bool _WiFi_IsConnected = false;
bool _WiFi_ConnectWorking = false;
String _MqttBroker = "";
bool debugOn=false;

AirNginClient* AirNginClient::instance = nullptr;


AirNginClient::AirNginClient():_MqttObj(_WifiClient), _Reboot(false),_TimerMqttLoopPrev(0) ,_Server(80),_MqttCon_Steps(0) {
}




void AirNginClient::begin(String _name_of_brand, String _brand_version, String _serial_of_device , bool is_debug_enabled) {

  instance = this;
  instance->isDebugEnabled = is_debug_enabled;
  //-------------------------------------------------------- Preparing EEPROM & File
  EEPROM_Start();
  SPIFFS_Start();

  Tools__SettingRead();
  Tools__SettingShowInfo();
  _MqttObj.setBufferSize(4096);
  if(_name_of_brand != ""){
    brand_name = _name_of_brand;
  }

  if(_brand_version != ""){
    brand_version = _brand_version;
  }



  instance->Debug_ConsolePrintln("brand_version :: >> " + brand_version);
  instance->Debug_ConsolePrintln("brand_name :: >> " + brand_name);
  
  if(_serial_of_device != ""){
    _SerialCloud = _serial_of_device;
  }else{
    instance->Debug_ConsolePrintln("Serial Can't Empty");
    return;
  }


  instance->Debug_ConsolePrintln("brand_version :: >> " + brand_version);
  instance->Debug_ConsolePrintln("brand_name :: >> " + brand_name);
  instance->Debug_ConsolePrintln("_SerialCloud :: >> " + _SerialCloud);
  
 

  instance->Debug_ConsolePrintln(".... IOT Setup START ...");

  //........................................... WiFi Settings
  _WifiClient.setTimeout(15);
  _WiFi_ConnectWorking = true;
  WiFi.onEvent(this->IOT__WiFiEvent);

  if (WiFi.isConnected())
  {
    WiFi.disconnect(true);
    delay(100);
  }
  _WiFi_IsConnected = false;

  //............................................ Mqtt Settings
  _MqttCon_IsConnected = false;
  instance->_MqttCon_Steps = 0;
  //........................................... IOT Checker
    Tools__WifiPower(true);
    WiFi.mode(WIFI_STA);

    const char *ssid = _ModemSSID.c_str();
    const char *password = _ModemPass.c_str();
 
  xTaskCreatePinnedToCore(
     AirNginClient::IOT__Checker, // Function to implement the task
      "IOT__Checker",       // Name of the task (for debugging)
      TASK_STACKSIZE_LARGE, // Stack size (bytes)
      NULL,                 // Parameter to pass
      TASK_PRIORITY_HIGH,   // Task priority (Max:1)
      NULL,                 // Task handle
      TASK_CORE_1);         // Core (0/1)

  instance->Debug_ConsolePrintln(".... IOT Setup OK ...");
}


void AirNginClient::EEPROM_Start(){
  if (!EEPROM.begin(EEPROM_SIZE)) {
    byte fail = 0;
    while (!EEPROM.begin(EEPROM_SIZE)) {
      instance->Debug_ConsolePrintln("EEPROM Failed to init: " + String(fail));
      delay(2000);
      if (++fail > 5) {
if(instance->isDebugEnabled)
 instance->Debug__ConsoleBegin();

        Serial.println("EEPROM Failed to init Many Times");
        delay(500);
        Tools__Reboot("EEPROM.begin");
        return;
      }
    }
  }
  instance->Debug_ConsolePrintln("EEPROM Size OK (" + String((byte)(100 * EP_MEMORYEND2 / EEPROM_SIZE)) + "\% Used / " + String(EP_MEMORYEND2 + 2) + " of " + String(EEPROM_SIZE) + " Bytes)");
  if (EP_MEMORYEND2 + 2 >= EEPROM_SIZE) {
if(instance->isDebugEnabled)
    instance->Debug__ConsoleBegin();

    while (true) {
      Serial.println("\r\n!!!!!!!!!!!!!!!!!\r\nEPROM Overflow Address (at " + String(EP_MEMORYEND2 + 2) + ") MAX is " + String(EEPROM_SIZE));
      Serial.flush();
      delay(100);
    }
  }
}



void AirNginClient::SPIFFS_Start(){
  byte fail = 0;
  while (!SPIFFS.begin(true) && fail < 5) {  // Set the true parameter to format the file system if it fails to mount
if(instance->isDebugEnabled)
    instance->Debug__ConsoleBegin();

    Serial.println("SPIFFS Mount Failed");
    delay(1000);
  }
  fail = (SPIFFS.totalBytes() - SPIFFS.usedBytes()) * 100 / SPIFFS.totalBytes();
  if (fail < 3) {
if(instance->isDebugEnabled)
    instance->Debug__ConsoleBegin();
  
    while (true) {
      Serial.println("\r\n!!!!!!!!!!!!!!!!!\r\nSPIFFS Is Full (" + String(fail) + "%) " + String(SPIFFS.usedBytes()) + " of " + String(SPIFFS.totalBytes()) + " Bytes");
      Serial.flush();
      delay(100);
    }
  }
  instance->Debug_ConsolePrintln("FILES Space OK (" + String((byte)(100 * SPIFFS.usedBytes() / SPIFFS.totalBytes())) + "\% Used / " + String(SPIFFS.usedBytes()/1024) + " of " + String(SPIFFS.totalBytes()/1024) + " Kb)\r\n");
}


void AirNginClient::Debug_ConsolePrint(String p1)
{
    if(!instance->isDebugEnabled) return;
  try
  {
    p1.replace("\r\n", "\n");
    p1.replace('\r', '\n');
    p1.replace("\n", "\r\nDebug("+String(instance->brand_version)+")\t     ");
    Serial.print("Debug("+String(instance->brand_version)+")\t     "+p1);
  }
  catch (...)
  {
    Serial.println("\r\nErr Debug_ConsolePrint\r\n");
  }
}

void AirNginClient::Debug_ConsolePrint(int p1) { Debug_ConsolePrint(String(p1)); }
void AirNginClient::Debug_ConsolePrint(long p1) { Debug_ConsolePrint(String(p1)); }
void AirNginClient::Debug_ConsolePrint(float p1) { Debug_ConsolePrint(String(p1)); }


void AirNginClient::Debug_ConsolePrintln(String p1)
{
    if(!instance->isDebugEnabled) return;
  try
  {
    p1.replace("\r\n", "\n");
    p1.replace('\r', '\n');
    p1.replace("\n", "\r\nDebug("+String(instance->brand_version)+")\t     ");
    Serial.println("Debug("+String(instance->brand_version)+")\t     "+p1);
    Serial.flush();
  }
  catch (...)
  {
    Serial.println("\r\nErr Debug_ConsolePrintln\r\n");
  }
}

void AirNginClient::Debug_ConsolePrintln(int p1) { Debug_ConsolePrint(String(p1)); }
void AirNginClient::Debug_ConsolePrintln(long p1) { Debug_ConsolePrint(String(p1)); }
void AirNginClient::Debug_ConsolePrintln(float p1) { Debug_ConsolePrint(String(p1)); }



void AirNginClient::Debug__ConsoleBegin()
{

  // if (!_IsSerialStarted)
  // {
  //   Serial.setRxBufferSize(1024); // تنظیم اندازه بافر فقط در صورت عدم راه‌اندازی
  //   Serial.begin(9600);
  //   _IsSerialStarted = true; // علامت‌گذاری به عنوان شروع شده
  //   Serial.flush();
  // }
}


void AirNginClient::client_Loop()
{
  instance->TimerSec_Refresh();
  if(instance->isConfigMode)
  {
    return;
  }
    try
    {
         if(instance->_MqttCon_Steps != 99){

      if (_MqttCon_IsConnected)
      {
        _IOT_PingTimeout = MIN_15;
        _MqttObj.loop(); // اجرای mqtt.loop
        delay(30);
      }
      else
      {
        if (_IOT_PingTimeout == TIMER_NEED_RUN)
        {
          Network_Reset();
        }
        delay(30);
      }
      if (_IOT_ModemTimeout == TIMER_NEED_RUN)
      {
        _IOT_ModemTimeout = TIMER_EXPIRED_FOREVER;
        if (_Reboot) Send_RebootAndStatus();
      }
      else if (_Reboot)
      {
        if (_IOT_ModemTimeout == TIMER_JOB_DONE) Send_RebootAndStatus();
      }
      }
    }
    catch (...)
    {
      instance->Debug_ConsolePrintln("IOT__Loop Error ");
    }
    delay(TASK_LOOP_DELAY);
  
}

void AirNginClient::IOT__Checker(void *param)
{
    
    while (true)
    {
        instance->TimerSec_Refresh();
        delay(1000);
     
         if(!instance->isConfigMode){
            if (instance->_MqttCon_Steps == 0)
            {
                if (instance->_IOT_ModemTimeout == TIMER_JOB_DONE)
                    instance->_IOT_ModemTimeout = MIN_3;
                instance->IOT__WiFiStart();
                instance->Debug_ConsolePrintln("############ _MqttCon_Steps 0 :: WiFiStart()");
                if (instance->_MqttCon_Steps == 0)
                    instance->_MqttCon_Steps = 1;
            }
            else if (instance->_MqttCon_Steps == 2)
            {
                const char *mqtt_broker = instance->_MqttBroker.c_str();
                const int mqtt_port = 1883;
                instance->_MqttObj.setCallback(Mqtt__OnRecieve);
                instance->_MqttObj.setServer(mqtt_broker, mqtt_port);
                instance->_MqttObj.setBufferSize(2048);
                instance->_MqttObj.setSocketTimeout(15);
                instance->_MqttObj.setKeepAlive(15);
                instance->_MqttCon_Steps = 3;
                instance->Debug_ConsolePrintln("############ _MqttCon_Steps 1 :: MqttObj.setServer()");
            }
            else if (instance->_MqttCon_Steps == 3)
            {
                // instance->Debug_ConsolePrintln("############ _MqttCon_Steps 3 :: call Mqtt__Connect()");
                delay(1000);
                instance->Mqtt__Connect();
            }
            else
            {
                instance->Debug_ConsolePrintln("############ _MqttCon_Steps " + String(instance->_MqttCon_Steps) + " :: No Action");
            }
         }

        delay(1000);

    }
}

void AirNginClient::IOT__WiFiEvent(WiFiEvent_t event)
{   
    
        
    if (instance->_MqttCon_Steps == 99)
            return;

    switch (event) {
    case ARDUINO_EVENT_WIFI_READY:               instance->Debug_ConsolePrintln("WiFi interface ready"); break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:           instance->Debug_ConsolePrintln("Completed scan for access points"); break;
    case ARDUINO_EVENT_WIFI_STA_START:           instance->Debug_ConsolePrintln("WiFi client started"); break;
    case ARDUINO_EVENT_WIFI_STA_STOP:            instance->Debug_ConsolePrintln("WiFi clients stopped"); break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:       instance->Debug_ConsolePrintln("Connected to access point"); break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        instance->Debug_ConsolePrintln("Disconnected from WiFi access point");
        instance->_WiFi_IsConnected = false;
        instance->_MqttCon_IsConnected = false;
        instance->_MqttCon_Steps = 0;
        instance->Debug_ConsolePrintln("WiFi lost connection");
        
         break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE: instance->Debug_ConsolePrintln("Authentication mode of access point has changed"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
         instance->Debug_ConsolePrintln("WiFi connected, IP: " + WiFi.localIP().toString());
         instance->_WiFi_IsConnected = true;
         instance->_IOT_MqttTimeout = TIMER_NEED_RUN;
         instance->_MqttCon_Steps = 2;
      instance->Debug_ConsolePrintln("Obtained IP address: ");
      instance->Debug_ConsolePrintln(String(WiFi.localIP()));
      break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:        instance->Debug_ConsolePrintln("Lost IP address and IP address is reset to 0"); break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:          instance->Debug_ConsolePrintln("WiFi Protected Setup (WPS): succeeded in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_FAILED:           instance->Debug_ConsolePrintln("WiFi Protected Setup (WPS): failed in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:          instance->Debug_ConsolePrintln("WiFi Protected Setup (WPS): timeout in enrollee mode"); break;
    case ARDUINO_EVENT_WPS_ER_PIN:              instance->Debug_ConsolePrintln("WiFi Protected Setup (WPS): pin code in enrollee mode"); break;
    case ARDUINO_EVENT_WIFI_AP_START:           instance->Debug_ConsolePrintln("WiFi access point started"); break;
    case ARDUINO_EVENT_WIFI_AP_STOP:            instance->Debug_ConsolePrintln("WiFi access point  stopped"); break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:    instance->Debug_ConsolePrintln("Client connected"); break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED: instance->Debug_ConsolePrintln("Client disconnected"); break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:   instance->Debug_ConsolePrintln("Assigned IP address to client"); break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:  instance->Debug_ConsolePrintln("Received probe request"); break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:         instance->Debug_ConsolePrintln("AP IPv6 is preferred"); break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:        instance->Debug_ConsolePrintln("STA IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_GOT_IP6:             instance->Debug_ConsolePrintln("Ethernet IPv6 is preferred"); break;
    case ARDUINO_EVENT_ETH_START:               instance->Debug_ConsolePrintln("Ethernet started"); break;
    case ARDUINO_EVENT_ETH_STOP:                instance->Debug_ConsolePrintln("Ethernet stopped"); break;
    case ARDUINO_EVENT_ETH_CONNECTED:           instance->Debug_ConsolePrintln("Ethernet connected"); break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:        instance->Debug_ConsolePrintln("Ethernet disconnected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:              instance->Debug_ConsolePrintln("Obtained IP address"); break;
    default:                                    break;
  }
        delay(100);
    
}

void AirNginClient::Mqtt__Connect()
{
  
  if (_Mqtt_TryConnecting)
    return;
  try
  {
    if (!_MqttObj.connected() || _MqttObj.state() < 0)
    {
      _Mqtt_TryConnecting = true;
      if (!Network_HelthCheck())
      {
        _Mqtt_TryConnecting = false;
        return;
      }
      if (_IOT_MqttTimeout > 0)
      {
        _Mqtt_TryConnecting = false;
        return;
      }
      instance->Debug_ConsolePrintln("Mqtt_Connecting...");

      delay(1500);
      if (_MqttObj.connect(_CloudClientId.c_str(), _MqttUser.c_str(), _MqttPass.c_str()))
      {
        //........................................................... Reconnected
        delay(10);
        instance->Debug_ConsolePrintln("Mqtt_Connect Success Status :: " + String(_MqttObj.state()));
      
        _MqttObj.subscribe((instance->_ProjectCode + "/DeviceToDevice").c_str());
        _MqttObj.subscribe((instance->_ProjectCode + "/ServerToDevice").c_str());
        _MqttObj.subscribe((instance->_ProjectCode + "/DeviceSetting").c_str());
        _MqttObj.subscribe((instance->_ProjectCode + "/Share").c_str());
        _MqttObj.subscribe("Time/Tehran");    
        _MqttCon_IsConnected = true;
        if (_IOT_ModemTimeout > 0)  _IOT_ModemTimeout = TIMER_JOB_DONE;
        // Dvc__CloudSendAllStatus(); // send last status
      }
      //........................................................... Cant Reconnect
      else
      {
        _MqttCon_IsConnected = false;
        if (_IOT_ModemTimeout > 0) _IOT_ModemTimeout = TIMER_JOB_DONE;
        instance->Debug_ConsolePrintln("Mqtt_Connect Fail Status :: " + String(_MqttObj.state()));
        _IOT_MqttTimeout = MIN_5; //  5 Min (300sec) Delay If Retry Failed (To Prevent Hanging Manual Touch)
      }
      delay(50);
    }
    //......................................... Mqtt Has Been Connected (Without Trying)
    else
    {
      // instance->Debug_ConsolePrintln("Mqtt_Connect : OK");
    }
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("Mqtt_Connect Error");
  }
  _Mqtt_TryConnecting = false;
}


void AirNginClient::Mqtt__OnRecieve(char *topic, uint8_t *payload, unsigned int length)
{
  if(instance->CALL_Global_Mqtt_CALLBACK){
    instance->Debug_ConsolePrintln("CALL_Global_Mqtt_CALLBACK is true");
    if (instance->onMessageCallback != nullptr) {
      instance->onMessageCallback(topic, payload, length);  // فراخوانی callback ثبت‌شده توسط کاربر
    }
    return;
  }
    instance->Debug_ConsolePrintln("CALL_Global_Mqtt_CALLBACK is false");


    String projectTopic = String(topic);
    int p = projectTopic.indexOf('/');

    if (projectTopic.substring(0, p) != instance->_ProjectCode)
            return;

  try
  {
    IOT__Call__Loop__MQTT();
    instance->Debug_ConsolePrintln("\r\nMqtt OnRecieve  : > length : " + String(length) + " > Project/Topic: " + projectTopic);
    //----------------------------------------------------------------------------- Cloud MqttBroker
    //.................................... Check Project

    String projectTopic = instance->CA_ToString(topic);
    int p = projectTopic.indexOf('/');
    if (projectTopic.substring(0, p) != instance->_ProjectCode)
      return;
    projectTopic = projectTopic.substring(p + 1);
  
     JsonDocument doc;
     doc.clear();
     deserializeJson(doc, payload);
     //.................................... Check Topic
     if (projectTopic == "DeviceSetting")
     {

        String opr = doc["operationName"].as<String>();      
        if (opr == "save_scenario" && instance->onSaveScenario != nullptr) {
            instance->onSaveScenario(doc);
        } else if (opr == "delete_scenario" && instance->onDeleteScenario != nullptr) {
            String vcode = doc["code"].as<String>();
            instance->Debug_ConsolePrintln(" Delete scenario with code : " + vcode);
            instance->onDeleteScenario(vcode);
        } else if (opr == "save_setting" && instance->onSaveSetting != nullptr && doc["deviceSerial"].as<String>() == instance->_SerialCloud) {
            String cmd = doc["value"].as<String>();
            instance->onSaveSetting(cmd);
        }

     }

     else if (projectTopic == "ServerToDevice")
     {
       String deviceSerial = doc["deviceSerial"].as<String>();
       instance->Debug_ConsolePrintln("json deviceSerial :: " + deviceSerial);
       String cmd = "";
       try
       {
         if (doc["operationName"]) cmd = doc["operationName"].as<String>();
       }
       catch (...) {}
       if (cmd != "")
       {
         if (cmd == "firmware_update"){
           String value = doc["value"].as<String>();
           if(value!=""){
             doc.clear();
             deserializeJson(doc, value);
             instance->IOT__FirmwareUpdate(doc["link"].as<String>(), doc["cert"].as<String>(), doc["utc"].as<String>());
           }
         }
         else if (cmd == "firmware_version"){
             instance->Mqtt_Send("FirmwareVersion", "{'deviceSerial':'" + instance->_SerialCloud + "','operationName':'firmware_version','value':'" + String(instance->brand_version) + "'}", false);
         }
         else if (cmd == "disable_scenario" || cmd == "enable_scenario" || cmd == "run_scenario")
         {
           String nameCode = doc["value"].as<String>() + "_"; // To Find Scenario By Name-StartWith
           for (int i = MAX_SCENARIO - 1; i >= 0; i--)
           {

           }
         }
         else if (cmd == "debugger")
         {
           if (doc["value"])
           {
             String value = doc["value"].as<String>();
             if (value == "reboot")
               instance->Tools__Reboot("Mqtt__OnRecieve reboot");
             else if (value == "clean")
             {
               instance->MemoClean(EP_MEMORYSTART2);
               instance->Tools__Reboot("Mqtt__OnRecieve clean");
             }
             else if (value == "reset_factory")
             {
               instance->MemoClean(EP_MEMORYSTART1);
               instance->Tools__Reboot("Mqtt__OnRecieve reset_factory");
             }
             else if (value == "config")
             {
               instance->Tools__SetMode("config_panel", true);
             }
             else if (value == "special")
            {
              instance->onDebugger(value);
            }
           }
         }
       }
       else if (doc["data"])
       {
         cmd = doc["data"].as<String>();
         instance->Debug_ConsolePrintln("json data :: " + cmd);
         instance->Message_On_Topic_ServerToDevice_Callback(cmd);
       }
     }
     else if (projectTopic == "DeviceToDevice")
     {
      instance->Debug_ConsolePrintln("command by Own-Serial > Dvc__AnalyzeData");
      instance->Message_On_Topic_DeviceToDevice_Callback(payload);
       //.................................... Extarct Data
       //  String type = doc["type"].as<String>();
       //  String data = doc["data"].as<String>();
       //   return data;
     }
     doc.clear();

  }
  catch (...)
  {
  }
  instance->_MqttRcv_IsWorking = false;
}

void AirNginClient::Tools__SetMode(String mode, bool restartNeed)
{
  instance->Debug_ConsolePrintln("Set Next Mode To : " + mode);
  //.............................................
  if (mode == "normal")
  {
  }
  //.............................................
  Tools__Memory_StrSet("_StartMode", mode);
  if (restartNeed)
  {
    Tools__Reboot("Tools__SetMode");
  }
}

void AirNginClient::Tools__Reboot(String Reason = "")
{
  instance->Debug_ConsolePrintln( "\r\n\r\n!!!!!!!! Rquested To REBOOT !!!!!!!!\r\n      Due to : " + Reason + "\r\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\r\n\r\n" );
  ESP.restart();
}



void AirNginClient::MemoClean(int p_start, int p_end, bool cleanFiles) {

  instance->Debug_ConsolePrintln( "MemoClean: " + String(p_start) + " To " + String(p_end) + (cleanFiles ? "  And Clean Files" : " "));
  
  try
  {
    for (int i = p_start; i <= p_end; i++)
    {
      EEPROM.write(i, 0xFF);
      if (i % 100 == 0)
      {
        EEPROM.commit();
        delay(1);
      }
    }
    EEPROM.commit();
    delay(100);
  }
  catch (...)
  {
  }
  if (cleanFiles)
    Tools__File_ClearAll();
}



void AirNginClient::MemoCheck()
{
  int sum = 0;
  try
  {
    sum = (MemoReadByte(EP_MEMORYPOINT1H) * 256) + MemoReadByte(EP_MEMORYPOINT1L);
  }
  catch (...)
  {
  }
  if (sum != EP_MEMORYPOINT1L)
  {
    MemoClean(EP_MEMORYSTART1, EP_MEMORYEND1, true);
    MemoWriteByte(EP_MEMORYPOINT1H, (EP_MEMORYPOINT1L & 0xFF00) >> 8);
    MemoWriteByte(EP_MEMORYPOINT1L, EP_MEMORYPOINT1L & 0xFF);
  }
  //................................................................................
  sum = 0;
  try
  {
    sum = (MemoReadByte(EP_MEMORYPOINT2H) * 256) + MemoReadByte(EP_MEMORYPOINT2L);
  }
  catch (...)
  {
  }
  if (sum != EP_MEMORYPOINT2L)
  {
    MemoClean(EP_MEMORYSTART2, EP_MEMORYEND2, true);
    MemoWriteByte(EP_MEMORYPOINT2H, (EP_MEMORYPOINT2L & 0xFF00) >> 8);
    MemoWriteByte(EP_MEMORYPOINT2L, EP_MEMORYPOINT2L & 0xFF);
  }
}



long AirNginClient::Tools__Random(long a, long b)
{
  long d;
  if (a > b)
  {
    d = a;
    a = b;
    b = d;
  }
  d = (b - a) + 1;
  try
  {
    return a + (esp_random() % d);
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("Random Error");
  }
  return a + (rand() % d);
}



uint32_t AirNginClient::Tools__GetChipID()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  return chipId;
}



void AirNginClient::IOT__WiFiStart()
{

    if(instance->isConfigMode)
    return;

    instance->_WiFi_ConnectWorking = true;
//   try
//   {

    instance->Debug_ConsolePrintln("Connecting to Wi-Fi...");
    instance->Tools__WifiPower(true);
    WiFi.mode(WIFI_STA);

    const char *ssid = _ModemSSID.c_str();
    const char *password = _ModemPass.c_str();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    instance->Debug_ConsolePrintln("Connecting...");
  }

    instance->Debug_ConsolePrintln("***After begin Wi-Fi...");

//   }
//   catch (...)
//   {
//   }
  delay(100);
  instance->_WiFi_ConnectWorking = false;
}




void AirNginClient::Tools__WifiPower(bool SetToOn)
{
  if (SetToOn)
  {
    //................................. Sets WiFi Power-Sleep to None
    WiFi.setSleep(WIFI_PS_NONE);
    //................................. Sets WiFi RF power output to highest level
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    // WiFi.setTxPower(WIFI_POWER_2dBm);  // Sets WiFi RF power output to low level
  }
  else
  {
    WiFi.mode(WIFI_OFF);
  }
}




void AirNginClient::Network_Reset()
{
  instance->Debug_ConsolePrintln("\r\n************* Network_Reset!! ***************\r\n");
  //....................................................... Prevent IOT_Checker
  instance->_MqttCon_Steps = 99;
  //.......................................................
  _IOT_PingTimeout = MIN_15;

  if (WiFi.isConnected())
  {
    WiFi.disconnect(true);
    delay(500);
  }
  Tools__WifiPower(false);
  delay(500);
  Tools__WifiPower(true);

  _MqttCon_IsConnected = false;
  _WiFi_IsConnected = false;
  // WiFi.onEvent(IOT__WiFiEvent);
  //....................................................... Activate IOT_Checker from start
  instance->_MqttCon_Steps = 0;
}




bool AirNginClient::Network_HelthCheck()
{
  // اگر mqtt تا 15 دقیقه اگر وصل نبود یک بار وی فای ریست شود
  // اگر کمتر از 10 ثانیه هست که ریکانکت میشه نیازی به پینگ نیست
  instance->Debug_ConsolePrintln("Network_HelthCheck (after " + String(MIN_15 - _IOT_PingTimeout) + " sec)");
  if (MIN_15 - _IOT_PingTimeout < SEC_30) return true;
  //................................................................
  if (Network_Ping(WiFi.gatewayIP().toString()))
  {
    instance->Debug_ConsolePrintln("\r\n************* SUCCESS PING FROM GATEWAY !! ***************\r\n");

    if (Network_Ping(_MqttBroker))
    {
    
    
      instance->Debug_ConsolePrintln("\r\n************* SUCCESS PING FROM BROKER WITH FIRST DOMAIN !! ***************\r\n");

      // اگر به بروکر وصل بود تلاش کنیم برای وصل
      _IOT_MqttTimeout = TIMER_NEED_RUN;
      return true;
    }
    else
    {
      //  may no internet
      //  یعنی در کل اینترنت نیست ، ولی ممکن است اینترانت شده باشد
      if (MIN_15 - _IOT_PingTimeout > MIN_10)
      {
        instance->Debug_ConsolePrintln("\r\n************* AFTER 10 MINUTES FOR CHECK AGAIN !! ***************\r\n");
        if (_MqttBroker == "mqtt.airngin.ir")
        {
          
          if (Network_Ping("mqtt.airngin.com"))
          {
            _MqttBroker = "mqtt.airngin.com";
            instance->Debug_ConsolePrintln("\r\n************* HAS PING FOR MQTT.AIRNGIN.COM !! ***************\r\n");

            return true;
          }
        }
        else if (Network_Ping("mqtt.airngin.ir"))
        {
            instance->Debug_ConsolePrintln("\r\n************* HAS PING FOR MQTT.AIRNGIN.IR !! ***************\r\n");

          _MqttBroker = "mqtt.airngin.ir";
          return true;
        }
        return false;
      }
      else
        return true;
    }

  }
  else
  {
    instance->Debug_ConsolePrintln("\r\n************* DOESN'T HAS PING FROM GATEWAY !! ***************\r\n");
    // اگر پینگ gateway نداریم نیاز است یک بار از مودم قطع شویم و دوباره وصل شویم
    Network_Reset();
    return false;
  }
  return false;
}




void AirNginClient::Send_RebootAndStatus()
{
  if (_Reboot)
  {
    //............................................ CLOUD BROKER
      if (_MqttCon_IsConnected)
      {
        _Reboot = false;
        Network_RSSI();//  Check Modem RSSI
        MessageCloud__ViaMqtt_NotifyTo_User(MESSAGE_CLOUD_REBOOT);
      }   
  }
}

void AirNginClient::Network_RSSI() 
{
  try{
    //...................................................................... Modem Out Of Reach
    if (!WiFi.isConnected())
    {
      instance->Debug_ConsolePrintln("Network_RSSI No Modem");
    }
    //...................................................................... Modem Is Valid
    else
    {
      int rssi = WiFi.RSSI();
      instance->Debug_ConsolePrintln("Network_RSSI of Modem : "+String(rssi));
      if(rssi<-80) {
        if (_MqttCon_IsConnected)
        {
          MessageCloud__ViaMqtt_NotifyTo_User(MESSAGE_CLOUD_MODEM_WAKE);
          // else for local_broker
        }
      }
    }
  }
  catch(...){}
}




bool AirNginClient::Network_Ping(String remote_host)
{
  instance->Debug_ConsolePrintln("Wifi_Ping To " + remote_host);
  if (!WiFi.isConnected())
  {
    instance->Debug_ConsolePrintln("Wifi_Ping No Modem");
    return false;
  }
  if (Ping.ping(remote_host.c_str()))
  {
    instance->Debug_ConsolePrintln("Wifi_Ping Success!!");
    return true;
  }
  else
  {
    instance->Debug_ConsolePrintln("Wifi_Ping Error :");
    return false;
  }
}




void AirNginClient::Mqtt_Send(String scope, String type, String data, String sender, bool offlineSupport)
{
  try
  {
    //................................. Reject If Mesh Not Connected
    if (data == "") return;
    if(data[0]=='{'){
      if( data.indexOf('"')<0 && data.indexOf('\'')>=0 ) data.replace('\'', '"');
    }
      
    String topic = _ProjectCode + "/" + scope;
    if (type == "cloud")
    {
      data = "{\"data\":\"" + data + "\"}";
    }
    instance->Debug_ConsolePrintln("Mqtt Send... " + topic + "  >  " + data);
    if (_MqttObj.connected())
      _MqttObj.publish(topic.c_str(), data.c_str());
    
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("MqttSend Error ");
  }
}




void AirNginClient::Mqtt_Send(String topic, String data, bool offlineSupport)
{
  try
  {
    //................................. Reject If Mesh Not Connected
    if (data == "")  return;
    if(data[0]=='{'){
      if( data.indexOf('"')<0 && data.indexOf('\'')>=0 ) data.replace('\'', '"');
    }
    topic = _ProjectCode + "/" + topic;
  
    //----------------------------------------------------------------------------- Cloud MqttBroker

    if (_MqttObj.connected())
      _MqttObj.publish(topic.c_str(), data.c_str());
    
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("MqttSend Error ");
  }
}


//===========================================================================================
//                                         Cloud Message
//===========================================================================================

//..................................................................... Mqtt Send Notfication To User
void AirNginClient::MessageCloud__ViaMqtt_NotifyTo_User(int messageCode)
{
  Mqtt_Send("SendNotificationToUser", "{\"code\":\"" + String(messageCode) + "\",\"type\":\"30\"}", true); // Reboot Popup-Notify To User of Project
}
void AirNginClient::MessageCloud__ViaMqtt_NotifyTo_User(String& messageCode)
{
  Mqtt_Send("SendNotificationToUser", "{\"code\":\"" + messageCode + "\",\"type\":\"30\"}", true); // Reboot Popup-Notify To User of Project
}


//..................................................................... Mqtt Send SMS To User
void AirNginClient::MessageCloud__ViaMqtt_SMSTo_User(int messageCode)
{
  Mqtt_Send("SendNotificationToUser", "{\"code\":\"" +  String(messageCode) + "\",\"type\":\"20\"}", true); // Reboot Popup-Notify To User of Project
}
void AirNginClient::MessageCloud__ViaMqtt_SMSTo_User(String& messageCode)
{
  Mqtt_Send("SendNotificationToUser", "{\"code\":\"" +  messageCode + "\",\"type\":\"20\"}", true); // Reboot Popup-Notify To User of Project
}


//..................................................................... Mqtt Send SMS To Center
void AirNginClient::MessageCloud__ViaMqtt_SMSTo_Center(int messageCode)
{
  Mqtt_Send("SendNotificationToCenter", "{\"code\":\"" +  String(messageCode) + "\",\"type\":\"20\"}", true); // Reboot Popup-Notify To User of Project
}
void AirNginClient::MessageCloud__ViaMqtt_SMSTo_Center(String& messageCode)
{
  Mqtt_Send("SendNotificationToCenter", "{\"code\":\"" +  messageCode + "\",\"type\":\"20\"}", true); // Reboot Popup-Notify To User of Project
}


//===========================================================================================
//                                          End
//===========================================================================================




void AirNginClient::IOT__Call__Loop__MQTT()
{
  if (instance->_MqttCon_IsConnected)
  {
    unsigned long currentMillis = millis();
    // بررسی فاصله زمانی 10 ثانیه
    if (currentMillis - instance->_TimerMqttLoopPrev >= instance->_MttLoopInterval)
    {
      instance->_TimerMqttLoopPrev = currentMillis; // به‌روزرسانی زمان آخرین اجرا
      instance->_MqttObj.loop();                    // اجرای mqtt.loop
    }
    delay(30);
  }
}



String AirNginClient::CA_ToString(char input[], int start, int len) {
  int i, end = strlen(input);
  String str = "";
  if (len == 0)
    len = end - start;
  end = start + len;
  for (i = start; i < end; i++)
  {
    if (input[i] == '\0')
      break;
    str += input[i];
  }
  return str;
}

void AirNginClient::MemoWriteByte(int p_start, byte val)
{
  EEPROM.write(p_start, val);
  EEPROM.commit();
  delay(1);
}


byte AirNginClient::MemoReadByte(int p_start)
{
  byte r = 0;
  try
  {
    r = 0 + EEPROM.read(p_start);
  }
  catch (...)
  {
  }
  return r;
}


bool AirNginClient::Tools__Memory_StrSet(String key, String val)
{
  instance->Debug_ConsolePrintln("Tools__Memory_StrSet > key : " + key + " > val : " + val );

  try
  {
    if (key == "_Configured")
    {
      instance->MemoWriteByte(EP_CONFIGURED, val == "1" ? 1 : 0);
    }
    else if (key == "_StartMode")
    {
      instance->MemoWriteByte(EP_STARTMODE,
                    val == "normal"           ? 0
                    : val == "reboot_sendall" ? 7
                    : val == "config_sendall" ? 8
                    : val == "config_panel"   ? 9
                                              : 0);
    }
    else if (key == "_Extender")
    {
      instance->MemoWriteByte(EP_EXTENDER, val == "2" ? 2 : 0);
    }
    else if (key == "_SerialNo")
    {
      val.toUpperCase();
      instance->MemoWriteString(EP_SERIAL_S, EP_SERIAL_E, val);
    }
    else if (key == "_Circuit")
    {
      instance->MemoWriteByte(EP_CIRCUIT, val == "wired"    ? 1
                                : val == "wifi"   ? 2
                                : val == "wirefi" ? 3
                                : val == "wiot"   ? 8
                                : val == "iot"    ? 9
                                               : 9);
    }
    else if (key == "_CloudClientId")
    {
      instance->MemoWriteString(EP_CLIENTID_S, EP_CLIENTID_E, val);
    }
    else if (key == "_ProjectCode")
    {
      val.toUpperCase();
      instance->MemoWriteString(EP_PROJECTCODE_S, EP_PROJECTCODE_E, val);
    }
    else if (key == "_EncryptionKey")
    {
      instance->MemoWriteString(EP_ENCRYPTIONKEY_S, EP_ENCRYPTIONKEY_E, val);
    }
    else if (key == "_MqttUser")
    {
      instance->MemoWriteString(EP_MQTTUSER_S, EP_MQTTUSER_E, val);
    }
    else if (key == "_MqttPass")
    {
      instance->MemoWriteString(EP_MQTTPASS_S, EP_MQTTPASS_E, val);
    }
    else if (key == "_MqttBroker")
    {
      instance->MemoWriteString(EP_MQTTBROKER_S, EP_MQTTBROKER_E, val);
    }
    else if (key == "_MqttBroker")
    {
      instance->MemoWriteString(EP_MQTTBROKER_S, EP_MQTTBROKER_E, val);
    }
    else if (key == "_MqttUser")
    {
      instance->MemoWriteString(EP_MQTTUSER_S, EP_MQTTUSER_E, val);
    }
    else if (key == "_MqttPass")
    {
      instance->MemoWriteString(EP_MQTTPASS_S, EP_MQTTPASS_E, val);
    }
    else if (key == "_ModemChannel")
    {
      byte b = instance->Tools__StringToByte(val);
      b = (b <= 14 && b > 0 ? b : 0);
      instance->MemoWriteByte(EP_MODEMCHANNEL, b);
    }
    else if (key == "_ModemSSID")
    {
      instance->MemoWriteString(EP_MODEMSSID_S, EP_MODEMSSID_E, val);
    }
    else if (key == "_ModemPass")
    {
      instance->MemoWriteString(EP_MODEMPASS_S,EP_MODEMPASS_E, val);
    }
    else if (key == "_ModemRSSI")
    {
      byte b = instance->Tools__StringToByte(val);
      b = (b < 255 && b >= 0 ? b : 255);
      instance->MemoWriteByte(EP_MODEMRSSI, b);
    }
    return true;
  }
  catch (...)
  {
  }
  return false;
}



String AirNginClient::Tools__Memory_StrGet(String key)
{


  bool checkNull = false;
  String res = "";
  int v = 0;
  try
  {
    if (key == "_Configured")
    {
      v = MemoReadByte(EP_CONFIGURED);
      res = (v == 1 ? "1" : "0");
    }
    else if (key == "_StartMode")
    {
      v = MemoReadByte(EP_STARTMODE);
      res = (v == 0   ? "normal"
             : v == 7 ? "reboot_sendall"
             : v == 8 ? "config_sendall"
             : v == 9 ? "config_panel"
                      : "normal");
    }
    else if (key == "_Extender")
    {
      v = MemoReadByte(EP_EXTENDER);

      res = (v == 2 ? "2" : "0");
    }
    else if (key == "_SerialNo")
    {
      res = MemoReadString(EP_SERIAL_S, EP_SERIAL_E);
      if (res == "" || res[0] == char(255))
        res = "0000000000";
    }
    else if (key == "_CloudClientId")
    {
      res = MemoReadString(EP_CLIENTID_S, EP_CLIENTID_E);
      if (res[0] == char(255))
        res = "";
    }
    else if (key == "_Circuit")
    {
      v = MemoReadByte(EP_CIRCUIT);

      res = (v == 2   ? "wifi"
             : v == 9 ? "iot"
                      : "iot");
    }
    else if (key == "_ProjectCode")
    {
      res = MemoReadString(EP_PROJECTCODE_S, EP_PROJECTCODE_E);
      // res.toUpperCase();
      checkNull = true;
    }
    else if (key == "_EncryptionKey")
    {
      res = MemoReadString(EP_ENCRYPTIONKEY_S, EP_ENCRYPTIONKEY_E);
      checkNull = true;
    }
    else if (key == "_MqttUser")
    {
      res = MemoReadString(EP_MQTTUSER_S, EP_MQTTUSER_E);
      checkNull = true;
    }
    else if (key == "_MqttPass")
    {
      res = MemoReadString(EP_MQTTPASS_S, EP_MQTTPASS_E);
      checkNull = true;
    }
    else if (key == "_MqttBroker")
    {
      res = MemoReadString(EP_MQTTBROKER_S, EP_MQTTBROKER_E);
      checkNull = true;
    }
    else if (key == "_ModemChannel")
    {
      v = MemoReadByte(EP_MODEMCHANNEL);
      res = "" + String(v);
    }
    else if (key == "_ModemSSID")
    {
      res = MemoReadString(EP_MODEMSSID_S, EP_MODEMSSID_E);
      checkNull = true;
    }
    else if (key == "_ModemPass")
    {
      res = MemoReadString(EP_MODEMPASS_S, EP_MODEMPASS_E);
      checkNull = true;
    }
    else if (key == "_ModemRSSI")
    {
      v = MemoReadByte(EP_MODEMRSSI);
      res = "" + String(v);
    }
  }
  catch (...)
  {
  }
  if (checkNull && res.charAt(0) == char(255))
    res = "";

  instance->Debug_ConsolePrintln("Tools__Memory_StrGet > key : " + key + " > val : " + res);

  return res;
}



void AirNginClient::MemoWriteString(int p_start, int p_end, String val)
{
  int max = val.length();
  byte v[max];
  val.getBytes(v, max + 1);
  for (int i = p_start, c = 0; i <= p_end; i++, c++)
  {
    EEPROM.write(i, c < max ? v[c] : 0);
  }
  EEPROM.commit();
  delay(1);
}


String AirNginClient::MemoReadString(int p_start, int p_end)
{
  String val = "";
  int i, empty = 0;
  char v, cFF = char(0xFF);
  for (i = p_start; i <= p_end; i++)
  {
    try
    {
      v = EEPROM.read(i);
      if (v == '\0')
        break;
      if (v == cFF)
        empty++;
      val += ("" + String(v));
    }
    catch (...)
    {
    }
  }
  if (val.length() == empty)
    val = "";
    
  return val;
}


int AirNginClient::Tools__StringSplitSize(String inp, char splitter)
{
  int j, i = 0, max = inp.length();
  for (j = 0; j < max; j++)
  {
    if (inp.charAt(j) == splitter)
      i++;
  }
  i++;
  return i;
}



void AirNginClient::Tools__StringSplit(String inp, char splitter, String outArray[], int sizeOfArray)
{
  int j, j_old = 0, i = 0, max = inp.length();
  for (j = 0; j < max && i < sizeOfArray; j++)
  {
    if (inp.charAt(j) == splitter)
    {
      outArray[i] = inp.substring(j_old, j);
      j_old = j + 1;
      i++;
    }
  }
  if (i < sizeOfArray)
    outArray[i] = inp.substring(j_old);
}



byte AirNginClient::Tools__StringToByte(String inp)
{
  byte r = 0;
  try
  {
    r = (byte)((inp).toInt() & 0xFF);
  }
  catch (...)
  {
  }
  return r;
}



int AirNginClient::Tools__StringToInt(String inp)
{
  int r = 0;
  try
  {
    inp.trim();
    r = (inp).toInt();
  }
  catch (...)
  {
  }
  return r;
}



void AirNginClient::Tools__CloudJSON_Reload(String typ = "all")
{
  instance->Debug_ConsolePrintln("Tools__CloudJSON_Reload ...");
  typ.toLowerCase();
  JsonVariant inp;
  //.........................................................  Scenario Operation Reload
  if (typ == "all" || typ == "scenariooperation")
  {
    Tools__File_JSONLoad("scenarioOperation", inp);
    inp = inp["scenarioOperation"].as<JsonVariant>();
    // Config__ReceiveJson_ScenarioOperation(inp, false, true);
  }
  //.........................................................  Command Operation Reload
  if (typ == "all" || typ == "commandoperation")
  {
    Tools__File_JSONLoad("commandOperation", inp);
    inp = inp["commandOperation"].as<JsonVariant>();
    // Config__ReceiveJson_CommandOperation(inp, false, true);
  }
}


bool AirNginClient::Tools__File_TextWrite(const String &filename, const String &data)
{
  bool res = false;
  try
  {
    File file = SPIFFS.open(filename.c_str(), FILE_WRITE); // Open the file for writing
    try
    {
      if (!file)
        return false;
      res = file.print(data);
    }
    catch (...)
    {
    }
    file.close();
  }
  catch (...)
  {
  }
  return res;
}


String AirNginClient::Tools__File_TextRead(const String &filename)
{
  String data = "";
  try
  {
    File file = SPIFFS.open(filename.c_str(), FILE_READ);
    try
    {
      if (!file)
        return "";
      try
      {
        data.reserve(8100);
      }
      catch (...)
      {
      }
      data = file.readString();
    }
    catch (...)
    {
    }
    file.close();
  }
  catch (...)
  {
  }
  return data;
}



bool AirNginClient::Tools__File_Clear(const String &filename)
{
  bool res = false;
  try
  {
    File file = SPIFFS.open(filename.c_str(), FILE_WRITE); // Open the file for writing
    try
    {
      if (!file)
        return false;
      res = file.print("");
    }
    catch (...)
    {
    }
    file.close();
  }
  catch (...)
  {
  }
  return res;
}



void AirNginClient::Tools__File_ClearAll()
{
  try
  {
    SPIFFS.format();
  }
  catch (...)
  {
  }
}



void AirNginClient::Tools__File_JSONLoad(String filename, JsonVariant &jsn, JsonDocument &doc)
{
  try
  {
    filename.toLowerCase();
    filename.replace('\\', '/');
    if (filename.substring(0, 1) != "/")
      filename = "/" + filename;
    if (filename.indexOf(".json") < 0)
      filename += ".json";
    String data = Tools__File_TextRead(filename);
    instance->Debug_ConsolePrintln( "Tools__File_JSONLoad " + filename + " => " + data);
    if (doc == NULL)
      doc = _JSONDoc;
    if (data != "")
    {
      doc.clear();
      data = "{\"root\":" + data + "}";
      deserializeJson(doc, data.c_str());
      jsn = doc["root"].as<JsonVariant>();
      // doc.clear();
    }
  }
  catch (...)
  {
  }
}



void AirNginClient::Tools__File_JSONLoad(String filename, JsonVariant &jsn)
{
  try
  {
    filename.toLowerCase();
    filename.replace('\\', '/');
    if (filename.substring(0, 1) != "/")
      filename = "/" + filename;
    if (filename.indexOf(".json") < 0)
      filename += ".json";
    String data = Tools__File_TextRead(filename);
    instance->Debug_ConsolePrintln( "Tools__File_JSONLoad " + filename + " => " + data );
    if (data != "")
    {
      _JSONDoc.clear();
      data = "{\"root\":" + data + "}";
      deserializeJson(_JSONDoc, data.c_str());
      jsn = _JSONDoc["root"].as<JsonVariant>();
      //_JSONDoc.clear();
    }
  }
  catch (...)
  {
  }
}



bool AirNginClient::Tools__File_JSONSave(String filename, JsonVariant &jsn)
{
  try
  {
    String data = "";
    try
    {
      data = jsn.as<String>();
    }
    catch (...)
    {
    }
    if (data == "")
      serializeJson(jsn, data);
    if (data.length() <= 8000)
    {
      instance->Debug_ConsolePrintln( "Tools__File_JSONSave " + filename + " => " + data );
      try
      {
        data.reserve(8100);
      }
      catch (...)
      {
      }
      filename.toLowerCase();
      filename.replace('\\', '/');
      if (filename.substring(0, 1) != "/")
        filename = "/" + filename;
      if (filename.indexOf(".json") < 0)
        filename += ".json";
      Tools__File_TextWrite(filename, data);
      return true;
    }
  }
  catch (...)
  {
  }
  return false;
}



bool AirNginClient::Tools__File_JSONSave(String filename, String &data)
{
  try
  {
    if (data.length() <= 8000)
    {
      instance->Debug_ConsolePrintln( "Tools__File_JSONSave " + filename + " => " + data );
      try
      {
        data.reserve(8100);
      }
      catch (...)
      {
      }
      filename.toLowerCase();
      filename.replace('\\', '/');
      if (filename.substring(0, 1) != "/")
        filename = "/" + filename;
      if (filename.indexOf(".json") < 0)
        filename += ".json";
      Tools__File_TextWrite(filename, data);
      return true;
    }
  }
  catch (...)
  {
  }
  return false;
}



void AirNginClient::Tools__SettingRead()
{
  instance->Debug_ConsolePrintln("Setting Read... ");
  //...................................... Load Important Data
  instance->_Banned = (instance->MemoReadByte(EP_BANNED) == 1 ? 1 : 0);
  instance->_Configured = (instance->MemoReadByte(EP_CONFIGURED) == 1 ? 1 : 0);
  // _TestMode = MemoReadByte(EP_TESTMODE);
  // _TestMode = (_TestMode == 1 || _TestMode == 0xFE ? _TestMode : 0xFF);
  instance->_StartMode = instance->Tools__Memory_StrGet("_StartMode");
  //...................................... Read Other Settings
  instance->_CloudClientId = instance->Tools__Memory_StrGet("_CloudClientId");
  instance->_CloudClientId.trim();
  byte b = instance->MemoReadByte(EP_EXTENDER);
  instance->_Extender = (b == 3 || b == 1 || b == 2 ? b : 0);
  instance->_Circuit = instance->Tools__Memory_StrGet("_Circuit");
  instance->_Circuit = (_Circuit == "wirefi" || _Circuit == "wifi" ? "wifi" : "iot");
  instance->_Extender = 2;
  instance->_ProjectCode = instance->Tools__Memory_StrGet("_ProjectCode");
  instance->_EncryptionKey = instance->Tools__Memory_StrGet("_EncryptionKey");

  _ModemRSSI = instance->MemoReadByte(EP_MODEMRSSI);
  if (instance->_ModemRSSI <= 0 || _ModemRSSI > 99)
    instance->_ModemRSSI = 99;
  instance->_ModemSSID = instance->Tools__Memory_StrGet("_ModemSSID");
  instance->_ModemPass = instance->Tools__Memory_StrGet("_ModemPass");
  
  instance->_ModemChannel = instance->Tools__StringToByte(instance->Tools__Memory_StrGet("_ModemChannel"));
  if (instance->_ModemChannel > 11 || instance->_ModemChannel < 0)
    instance->_ModemChannel = 0;


  instance->_MqttBroker = instance->Tools__Memory_StrGet("_MqttBroker");
  instance->_MqttUser = instance->Tools__Memory_StrGet("_MqttUser");
  instance->_MqttPass = instance->Tools__Memory_StrGet("_MqttPass");
  //........................................... Local-Broker or Cloud-Broker
  
}


AirNginClient::~AirNginClient() {
    // Clean-up code if needed
}







//===========================================================================================
//                                      Config (AP Mode)
//===========================================================================================

bool _ConfigPanelIsOpen = false;
bool _NeedReboot = false;

String AirNginClient::Query_Data()
{
  String data = "";
  try
  {
    if (_Server.hasArg("data"))
      data = String(_Server.arg("data"));
    else if (_Server.hasArg("Data"))
      data = String(_Server.arg("Data"));
    else if (_Server.hasArg("DATA"))
      data = String(_Server.arg("DATA"));
    if (data == "")
    {
      String noNeed = ",RANDOM,Random,random,Rnd,RND,rnd,TOKEN,Token,token,TKN,Tkn,tkn";
      for (int i = 0; i < _Server.args(); i++)
      {
        if (noNeed.indexOf(_Server.argName(i)) >= 0)
          continue;
        data = String(_Server.arg(i));
        break;
      }
    }
  }
  catch (...) {}
  return data;
}



void AirNginClient::Config__ResponseJSON(String json, bool needRestart)
{
  json.replace('\'', '"');
  _Server.send(200, "text/json", json);
  if (needRestart)
  {
    _ConfigPanelIsOpen = false;
    delay(500);
    Tools__Reboot("Config__ResponseJSON");
  }
}



void AirNginClient::Config__API_FirmwareUpdater()
{
  try {
    HTTPUpload& upload = instance->_Server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      instance->Debug_ConsolePrintln("Update: " + String(upload.filename.c_str()));
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  //start with max available size
        instance->Debug_ConsolePrintln("Update Error : Cant Begin");
        // Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        // Update.printError(Serial);
        instance->Debug_ConsolePrintln("Update Error : Cant Write");
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { 
        instance->Debug_ConsolePrintln("Update Success");
        instance->Tools__SetMode("config_panel", false);  // Set Powercut Restart-Point
      } else {
        instance->Debug_ConsolePrintln("Update Error : Ends With Bad Status");
        //Update.printError(Serial);
      }
    }
  } catch (...) {}
}



void AirNginClient::Config__API_FirmwareResult()
{
  try
  {
    instance->Debug_ConsolePrintln("***** Updating... ");
    //...................................................
    instance->Debug_ConsolePrintln("***** Updating... ");
    //...................................................
    instance->_Server.sendHeader("Connection", "close");
    instance->_Server.send(200, "text/plain", (Update.hasError() ? "fail" : "ok"));
    instance->Tools__SetMode("config_panel", false);  // Set Restart-Point
    instance->Debug_ConsolePrintln("Update Finish & Reboot ******");
    instance->Tools__Reboot("Config__API_FirmwareResult");
  }
  catch (...){}
}



void AirNginClient::Config__API_GetSerial()
{
  try
  {
    instance->Debug_ConsolePrintln("***** Get Serial... ");
    //............................................
    instance->_Server.sendHeader("Connection", "close");
    instance->_Server.send(200, "text/plain", instance->_SerialCloud);
    instance->Debug_ConsolePrintln("Get Serial OK ******");
  }
  catch (...)
  {
  }
}



void AirNginClient::Config__API_Alive()
{
  if (!_ConfigPanelIsOpen)
    return;
  //...................................................
  instance->_Server.sendHeader("Connection", "close");
  instance->Debug_ConsolePrintln("HTTP Alive...");
  instance->_Server.send(200, "text/html", "ok");
}



void AirNginClient::Config__SetJson_Network(JsonDocument doc)
{
  instance->Debug_ConsolePrintln("API-Config: Network-Save");
  String tmp;
  JsonObject jsn = doc.as<JsonObject>();
  try
  {
    if (jsn["network"])
      jsn = jsn["network"].as<JsonObject>();
  }
  catch (...)
  {
  }
  
    try
  {
    if (jsn["projectCode"])
      instance->_ProjectCode = jsn["projectCode"].as<String>();
  }
  catch (...)
  {
  }
  try
  {
    if (jsn["encryptionKey"])
      instance->_EncryptionKey = jsn["encryptionKey"].as<String>();
  }
  catch (...)
  {
  }
  try
  {
    if (jsn["modemSsid"])
      instance->_ModemSSID = jsn["modemSsid"].as<String>();
  }
  catch (...)
  {
  }
  try
  {
    if (jsn["modemPass"])
      instance->_ModemPass = jsn["modemPass"].as<String>();
  }
  catch (...)
  {
  }
  String ch = "";
  try
  {
    if (jsn["modemChannel"])
      ch = jsn["modemChannel"].as<String>();
  }
  catch (...)
  {
  }
  instance->_ModemChannel = Tools__StringToByte(ch);
  
  try
  {
    if (jsn["mqttBroker"])
      instance->_MqttBroker = jsn["mqttBroker"].as<String>();
  }
  catch (...)
  {
  }
  try
  {
    if (jsn["mqttUser"])
      instance->_MqttUser = jsn["mqttUser"].as<String>();
  }
  catch (...)
  {
  }
  try
  {
    if (jsn["mqttPass"])
      instance->_MqttPass = jsn["mqttPass"].as<String>();
  }
  catch (...)
  {
  }
  try
  { //       clientId
    if (jsn["clientId"])
    {
      String cid = jsn["clientId"].as<String>();
      cid.trim();
      if (cid.indexOf("-" + instance->_ProjectCode + "-") >= 3 && cid.indexOf(instance->_SerialCloud + "-") > 10)
        instance->_CloudClientId = cid;
      else
        instance->_CloudClientId = "";
    }
  }
  catch (...)
  {
  }
  instance->Tools__SettingSave();
}



String AirNginClient::Config__SetJson(String data)
{
  instance->Debug_ConsolePrintln("\r\n* **** SetJson Recieved Data ******");
  _NeedReboot = false;
  String res = "", tmp;
  try
  {
    if (data != "")
    {
      JsonDocument doc;
      doc.clear();
      deserializeJson(doc, data.c_str());
      JsonObject jsn;
      //............................................................................

      String mode = "", sec = "";
      jsn = doc.as<JsonObject>();
      try
      {
        if (data.indexOf("\"requestMode\"") > 0)
          mode = jsn["requestMode"].as<String>();
        else if (data.indexOf("\"mode\"") > 0)
          mode = jsn["mode"].as<String>();
        mode.toLowerCase();
      }
      catch (...)
      {
      }
      try
      {
        if (data.indexOf("\"closeConfig\"") > 0)
          _NeedReboot = jsn["closeConfig"].as<bool>() ? true : false;
      }
      catch (...)
      {
      }
      try
      {
        if (data.indexOf("\"section\"") > 0)
          sec = jsn["section"].as<String>();
        sec.toLowerCase();
      }
      catch (...)
      {
      }
      if (mode == "network")
      {
        sec = "network";
        mode = "";
      }
      if (mode == "reboot" || mode == "close")
      {
        sec = "general";
        mode = "exit";
      }
      //...................................................... UPDATE
      if (sec == "update")
      {
        if (mode == "clean")
        {
          Tools__SetMode("config_panel", false); // Set Powercut Restart-Point
          MemoClean(EP_MEMORYSTART2, EEPROM_SIZE, true);
          Tools__SetMode("config_panel", false); // Set Powercut Restart-Point
          _NeedReboot = true;
          res = "{'result':'ok'}";
          goto response_end;
        }
      }
      //...................................................... GENERAL
      else if (sec == "general")
      {
        instance->Debug_ConsolePrintln("SetJson: general");
        //..................................................
        if (mode == "reboot")
        {
          instance->Debug_ConsolePrintln("SetJson: Reboot All");
          Tools__SetMode("reboot_sendall", false);
          _NeedReboot = true;
          res = "{'result':'ok'}";
          goto response_end;
        }
        else if (mode == "exit")
        {
          instance->Debug_ConsolePrintln("SetJson: Exit (Closing Config)");
          Tools__SetMode("normal", false);
          _NeedReboot = true;
          res = "{'result':'ok'}";
          goto response_end;
        }
      }
      //...................................................... NETWORK
      else if (sec == "network")
      {
        instance->Debug_ConsolePrintln("SetJson: Network");
        bool sendtoall = false;
        try
        {
          if (data.indexOf("\"target\"") > 0)
            sendtoall = (jsn["target"].as<String>() == "all" ? true : false);
        }
        catch (...)
        {
        }
        //..................................................
        if (mode == "restore")
        {
          instance->Debug_ConsolePrintln("SetJson: Network-Restore");
          Tools__SettingDefault();
          Tools__SettingSave();
          Tools__SetMode(sendtoall ? "config_sendall" : "config_panel", false); // Set Restart-Point
          res = "{'result':'ok'}";
          goto response_end;
        }
        //..................................................
        else
        {
          Config__SetJson_Network(doc);
          res = "{'result':'ok'}";
          goto response_end;
        }
      }
      else
      {
        res = "{'result':'wrong'}";
      }
      doc.clear();
    }
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("SetJson Error ");
  }
response_end:
  res.replace('\'', '"');
  return res;
}



void AirNginClient::Config__API_Set()
{
  if (!_ConfigPanelIsOpen)
    return;
  instance->Debug_ConsolePrintln("\r\n* **** Config API-Set Recieved Data Start ******");
  try
  {
     instance->_Server.sendHeader("Connection", "close");
    //.......................................................................
    instance->_ConfigTimeout = millis() + (10 * 60000); // Reset timeout for next 10 min
    //.......................................................................
    String data = instance->Query_Data();
    if (data != "")
      instance->Config__ResponseJSON(instance->Config__SetJson(data));
  }
  catch (...)
  {
  }
   instance->Debug_ConsolePrintln("****** Config API-Set Recieved Data Finish *****");
  if (_NeedReboot)
    instance->Tools__Reboot("Config__API_Set");
}



String AirNginClient::Config__GetJson_Network()
{
  String res = "'network':{";
  res += "'circuit':'" + _Circuit + "'";
  res += ",'extender':'" + String(_Extender) + "'";
  res += ",'projectCode':'" + _ProjectCode + "'";
  res += ",'encryptionKey':'" + _EncryptionKey + "'";
  res += ",'modemChannel':'" + String(_ModemChannel) + "'";
  res += ",'modemSsid':'" + _ModemSSID + "'";
  res += ",'modemPass':'" + _ModemPass + "'";
  res += ",'mqttUser':'" + _MqttUser + "'";
  res += ",'mqttPass':'" + _MqttPass + "'";
  res += ",'mqttBroker':'" + _MqttBroker + "'";
  res += ",'clientId':'" + _CloudClientId + "'";
  res += "}";
  return res;
}



void AirNginClient::Config__API_Get()
{
  try
  {
    instance->Debug_ConsolePrintln("\r\n* **** API-Get ******");
    instance->_Server.sendHeader("Connection", "close");
    //.......................................................................
    instance->_ConfigTimeout = millis() + (10 * 60000); // Reset timeout for next 10 min
    //.......................................................................
    String res = "";
    try
    {
      String data = instance->Query_Data();
      //...................................................................
      if (data != "")
      {
        int max = 1024 + (2 * data.length());
        JsonDocument doc;
        doc.clear();
        deserializeJson(doc, data.c_str());
        JsonObject jsn;
        //............................................................................

        String mode = "";
        String sec = "";
        jsn = doc.as<JsonObject>();
        try
        {
          if (data.indexOf("\"requestMode\"") > 0)
            mode = jsn["requestMode"].as<String>();
          else if (data.indexOf("\"mode\"") > 0)
            mode = jsn["mode"].as<String>();
          mode.toLowerCase();
        }
        catch (...)
        {
        }
        try
        {
          if (data.indexOf("\"section\"") > 0)
            sec = jsn["section"].as<String>();
          sec.toLowerCase();
        }
        catch (...)
        {
        }
        //...................................................................
        if (mode == "network")
        {
          sec = "network";
          mode = "";
        }
        else if (mode == "serial")
        {
          sec = "serial";
          mode = "";
        }
        //...................................................... Serial
        if (sec == "serial")
        {
          if (res.length() > 0)
            res += ",";
          res += "'serial':'" + instance->_SerialCloud + "'";
        }
        //...................................................... Network
        else if (sec == "network")
        {
          if (res.length() > 0) res += ",";
          res += "'serial':'" + instance->_SerialCloud + "'";
          //res += ",'dviceType':'" + String(DEVICE_TYPE) + "'";
if(instance->isDebugEnabled)
          res += ",'firmware':'" + String(instance->brand_version) + " Debug'" ;
else
          res += ",'firmware':'" + String(instance->brand_version) + "'";

          res += "," + instance->Config__GetJson_Network();
        }
        //...................................................... Return Result
        res = "{'result':'ok'," + res + "}";
        instance->Config__ResponseJSON(res);
      }
    }
    catch (...)
    {
      instance->Debug_ConsolePrintln("API-Get Error");
      res = "{'result':'error'}";
    }
    instance->Config__ResponseJSON(res);
  }
  catch (...)
  {
  }
response_end:
    instance->Debug_ConsolePrintln("response_end");

//   Tools__LED_Warning(WARNINGLED_AUTO);
}



void AirNginClient::Config__File_Logo()
{
  try
  {
    instance->_Server.send(200, "image/png", FPSTR(PROGMEM_LOGO));
    instance->Debug_ConsolePrintln("Logo File Loaded");
  }
  catch (...)
  {
  }
}

void AirNginClient::Config__File_Html()
{
  try
  {
    // instance->Debug_ConsolePrintln("\r\n***** HTTP Page Config Show-Start...\r\ntemp degree: " + Tools__ESPGetTemprature());
    instance->Debug_ConsolePrintln("\r\n***** HTTP Page Config Show-Start...\r\ntemp degree: ");
    //...................................................
    instance->_ConfigTimeout = millis() + (10 * 60000); // Reset timeout for next 10 min
    //...................................................
    byte i;
    String data = "<!DOCTYPE html><html dir='ltr' lang='en'><head><meta charset='utf-8'><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>";
    data += "<title>" + String(instance->brand_name) + " Settings </title>";
    data += "<script>window.OPT={};";
    data += "OPT.serial='" + instance->_SerialCloud + "';";
    data += "OPT.circuit='" + instance->_Circuit + "';";
    data += "OPT.extender='" + String(instance->_Extender) + "';";
    data += "OPT.projectCode='" + instance->_ProjectCode + "';";
    data += "OPT.encryptionKey='" + instance->_EncryptionKey + "';";
    data += "OPT.modemChannel='" + String(instance->_ModemChannel) + "';";
    data += "OPT.modemSsid='" + instance->_ModemSSID + "';";
    data += "OPT.modemPass='" + instance->_ModemPass + "';";
    data += "OPT.mqttUser='" + instance->_MqttUser + "';";
    data += "OPT.mqttPass='" + instance->_MqttPass + "';";
    data += "OPT.mqttBroker='" + instance->_MqttBroker + "';";
if(instance->isDebugEnabled)
    data += "OPT.firmware='" + String(instance->brand_version) + " (" + String(SOFTWARE_DATE) + ")" + " Debug';";
else
    data += "OPT.firmware='" + String(instance->brand_version) + " (" + String(SOFTWARE_DATE) + ")';";

    data += "OPT.updateUrl='airngin.com/update/firmware';";
    data += "</script>";
    //------------------------------------------------- header --------------------------------------
    instance->_Server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    instance->_Server.sendHeader("Pragma", "no-cache");
    instance->_Server.sendHeader("Expires", "-1");
    instance->_Server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    instance->_Server.send(200, "text/html", "");
    instance->_Server.sendContent(data);
    //------------------------------------------------- logo ----------------------------------------
    instance->_Server.sendContent("<script>OPT.logo='");
    instance->_Server.sendContent(FPSTR(PROGMEM_LOGO));
    instance->_Server.sendContent("';</script>");
    //------------------------------------------------- js jquery1.2.3 ------------------------------
    instance->_Server.sendContent("<script>");
    for (i = 0; i < maxPROGMEM_JS_JQUERY; i++)
      instance->_Server.sendContent(instance->PROGMEM_JS_JQUERY[i]);
    instance->_Server.sendContent("</script>");
    //------------------------------------------------- js main/network -----------------------------
    instance->_Server.sendContent("<script>");
    for (i = 0; i < maxPROGMEM_JS_MAIN; i++)
      instance->_Server.sendContent(instance->PROGMEM_JS_MAIN[i]);
    instance->_Server.sendContent("</script>");
    //------------------------------------------------- css -----------------------------------------
    instance->_Server.sendContent("<style>");
    for (i = 0; i < maxPROGMEM_CSS; i++)
      instance->_Server.sendContent(instance->PROGMEM_CSS[i]);
    instance->_Server.sendContent("</style>");
    //------------------------------------------------- html ----------------------------------------
    instance->_Server.sendContent("</head><body onload='prepare()'>");
    for (i = 0; i < maxPROGMEM_HTML; i++)
      instance->_Server.sendContent(instance->PROGMEM_HTML[i]);
    instance->_Server.sendContent("</body></html>");
    //-----------------------------------------------------------------------------------------------
    instance->_Server.sendContent("");
    instance->_Server.client().stop();
    instance->Debug_ConsolePrintln("HTTP Config Loaded ******");
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("HTTP Config Error ******");
  }
}

void AirNginClient::Config__Loop()
{
  while (instance->isConfigMode)
  {
    try
    {
      _Server.handleClient();
      //........................ Panel Timeout
      int timer = (_ConfigTimeout - millis());
      if (timer < 0)
      {
        Tools__Reboot("Config__Loop");
      }
    }
    catch (...)
    {
      instance->Debug_ConsolePrintln("Config__Loop Error ");
    }
    delay(TASK_LOOP_DELAY);
  }
}

void AirNginClient::Config__Setup()
{
  instance->_MqttCon_Steps = 99;
  _Mqtt_TryConnecting = true;
  WiFi.disconnect(true);
  _WiFi_ConnectWorking = true;
  instance->Debug_ConsolePrintln("\r\n Config_Setup Start ... \r\n");
  try
  {
    //...................................... Set Powercut Restart-Point
    Tools__Memory_StrSet("_StartMode", "normal");

    //....................................
     instance->_ConfigTimeout = millis() + (10 * 60000); // Set timeout on next 5 min

    IPAddress local_IP(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    //............................................
    WiFi.softAPConfig(local_IP, local_IP, subnet);
    delay(2500);

    String wifiName = _SerialCloud + "_" + instance->brand_name;
    String wifiPass = DVC_DEFAULTPASS;
    char wN[wifiName.length()];
    wifiName.toCharArray(wN, wifiName.length() + 1);
    char wP[wifiPass.length()];
    wifiPass.toCharArray(wP, wifiPass.length() + 1);
    WiFi.softAP(wN, wP);
    instance->Debug_ConsolePrintln("WifiName : " + String(wN) + "  &   Pass : " + String(wP));

    delay(1000);
    WiFi.softAPConfig(local_IP, local_IP, subnet);

    instance->Debug_ConsolePrintln(WiFi.softAPIP().toString());

    //............................................
    while (!MDNS.begin(wifiName))
    {
      instance->Debug_ConsolePrintln("Error setting up MDNS responder!");
      delay(1000);
    }
    // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    // DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");
    _Server.enableCORS();

    _Server.on("/logo.png", Config__File_Logo);
    _Server.on("/alive", Config__API_Alive);
    _Server.on("/set", Config__API_Set);
    _Server.on("/get", Config__API_Get);
    _Server.on("/getSerial", Config__API_GetSerial);
    _Server.on("/config", Config__File_Html);
    _Server.on("/", Config__File_Html);
    _Server.on("/firmware", HTTP_POST, Config__API_FirmwareResult, Config__API_FirmwareUpdater);

    _Server.begin();
    MDNS.addService("http", "tcp", 80);
    _ConfigPanelIsOpen = true;
  }
  catch (...)
  {
    instance->Debug_ConsolePrintln("\r\n Config_Setup Error\r\n");
  }
  _WiFi_ConnectWorking = false;
  instance->Debug_ConsolePrintln("\r\n Config_Setup End \r\n");
  //........................................... Stay In Loop
  Config__Loop();
}



void AirNginClient::Tools__SettingDefault()
{
  instance->Debug_ConsolePrintln("Setting To Dafault >>> ");
  //..................

  _Extender = 2;

  _Circuit = "iot";

  _ProjectCode = "";
  _EncryptionKey = "";

  _ModemSSID = "";
  _ModemPass = "";
  _ModemChannel = 0;

  _MqttBroker = "";
  _MqttUser = "";
  _MqttPass = "";

}



void AirNginClient::Tools__SettingSave()
{
  instance->Debug_ConsolePrintln("Setting Save... ");
  try
  {
    instance->_CloudClientId.trim();
    Tools__Memory_StrSet("_CloudClientId", instance->_CloudClientId);
    Tools__Memory_StrSet("_Extender", instance->_Extender == 3 || instance->_Extender == 1 || instance->_Extender == 2 ? String(instance->_Extender) : "0");
    Tools__Memory_StrSet("_Circuit", instance->_Circuit);
    Tools__Memory_StrSet("_ProjectCode", instance->_ProjectCode);
    Tools__Memory_StrSet("_EncryptionKey", instance->_EncryptionKey);

    Tools__Memory_StrSet("_ModemChannel", String(instance->_ModemChannel));
    Tools__Memory_StrSet("_ModemSSID", instance->_ModemSSID);
    Tools__Memory_StrSet("_ModemPass",instance->_ModemPass);

    Tools__Memory_StrSet("_MqttBroker", instance->_MqttBroker);
    Tools__Memory_StrSet("_MqttUser", instance->_MqttUser);
    Tools__Memory_StrSet("_MqttPass", instance->_MqttPass);

  }
  catch (...)
  {
  }
}



void AirNginClient::Tools__SettingShowInfo()
{
    if(!instance->isDebugEnabled) return;

    instance->Debug_ConsolePrintln("\r\n -----------------------\r\nSetting Show Info\r\n-----------------------");
    instance->Debug_ConsolePrintln("_SerialNo: " + instance->_SerialCloud);
    instance->instance->Debug_ConsolePrintln("_StartMode: " + instance->_StartMode);

    instance->Debug_ConsolePrintln("\r\n _Circuit: " + instance->_Circuit);
    instance->Debug_ConsolePrintln("_Extender: " + String(instance->_Extender) + (instance->_Extender == 1 || instance->_Extender == 3 ? " / Yes" : " / No"));
    instance->Debug_ConsolePrintln("_ProjectCode: " + instance->_ProjectCode);
    instance->Debug_ConsolePrintln("_EncryptionKey: " + instance->_EncryptionKey);

    instance->Debug_ConsolePrintln("\r\n _ModemChannel: " + String(instance->_ModemChannel));
    instance->Debug_ConsolePrintln("_ModemSSID: " + instance->_ModemSSID);
    instance->Debug_ConsolePrintln("_ModemPass: " + instance->_ModemPass);
    instance->Debug_ConsolePrintln("_ModemRSSI: " + String(instance->_ModemRSSI));

    instance->Debug_ConsolePrintln("\r\n _MqttBroker: " + instance->_MqttBroker);
    instance->Debug_ConsolePrintln("_MqttUser: " + instance->_MqttUser);
    instance->Debug_ConsolePrintln("_MqttPass: " +instance-> _MqttPass);
    instance->Debug_ConsolePrintln("_CloudClientId: " + instance->_CloudClientId);
    instance->Debug_ConsolePrintln("");
}



void AirNginClient::IOT__FirmwareUpdate(String DownloadLink, String Certificate, String UTC){
  if(_Update_Trying) return;
  _Update_Trying = true;
  try{
    instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : Start");
    if (WiFi.isConnected()==true)
    {
      instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : Wifi ok");
      SetClock(UTC);
      instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : clock ok");
      _WiFiClientSecure.setCACert( (Certificate == "" ? instance->rootCACertificate : Certificate).c_str());
      _WiFiClientSecure.setTimeout(20); // timeout argument is defined in seconds for setTimeout
      t_httpUpdate_return ret = httpUpdate.update(
          _WiFiClientSecure, DownloadLink, "",
          [](HTTPClient *_WiFiClientSecure)
          {
              // client->setAuthorization("test", "password");
          }
      );
      switch (ret)
      {
        case HTTP_UPDATE_FAILED:
           instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : UPDATE_FAILED Error ("+String(httpUpdate.getLastError())+"): "+String(httpUpdate.getLastErrorString().c_str()));
            break;

        case HTTP_UPDATE_NO_UPDATES:
             instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
             instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : UPDATE_OK");
            break;
      }
      instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : IOT__FirmwareUpdate : End");
    }
  }
  catch(...){
      instance->Debug_ConsolePrintln("IOT__FirmwareUpdate : Err");
  }
  _Update_Trying = false;
}



void AirNginClient::SetClock(String UTC) {
  struct tm timeinfo = {};
  if(UTC!=""){
    instance->Debug_ConsolePrintln("SetClock : Try to sync the time from Mqtt...");
    const char* dateTimeStr = UTC.c_str();
    if (strptime(dateTimeStr, "%Y-%m-%dT%H:%M:%S", &timeinfo) != NULL) {
      timeinfo.tm_isdst = 0;
      time_t t = mktime(&timeinfo);
      if (t != -1) {
        struct timeval tv = { .tv_sec = t, .tv_usec = 0 };
        settimeofday(&tv, NULL); 
        instance->Debug_ConsolePrintln("SetClock : Time set : "+String(asctime(&timeinfo)));
        return;
      } else {
        instance->Debug_ConsolePrintln("SetClock : Error setting time");
      }
    } else {
      instance->Debug_ConsolePrintln("SetClock : Invalid date format. Use YYYY-MM-DDTHH:MM:SS");
    }
  }
  instance->Debug_ConsolePrintln("SetClock : Try to sync the time from web...");
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");  // UTC
  instance->Debug_ConsolePrintln("SetClock : Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    instance->Debug_ConsolePrintln(".");
    now = time(nullptr);
  }
  timeinfo={};
  gmtime_r(&now, &timeinfo);
  instance->Debug_ConsolePrintln("SetClock : Current time: "+String(asctime(&timeinfo)));
}



void AirNginClient::TimerSec_Refresh()
{
  //-------------------------------------------- TIMER
  try
  {
    _TimerSecCurrent = millis();
    _TimerSecDef = ((_TimerSecCurrent - _TimerSecOld) / 1000) & 0xFF;
    if (_TimerSecDef < 0)
      _TimerSecDef = 1;
    if (_TimerSecDef >= 1)
    {
      _TimerSecOld = _TimerSecCurrent;
      _TimerForActions += _TimerSecDef;
      // if (_Started_IotRun)
      // {
        if (_IOT_PingTimeout > 0)
        {
          _IOT_PingTimeout -= _TimerSecDef;
          if (_IOT_PingTimeout <= 0) _IOT_PingTimeout = TIMER_NEED_RUN;
        }
        if (_IOT_MqttTimeout > 0)
        {
          _IOT_MqttTimeout -= _TimerSecDef;
          if (_IOT_MqttTimeout <= 0) _IOT_MqttTimeout = TIMER_NEED_RUN;
        }
        if (_IOT_ModemTimeout > 0)
        {
          _IOT_ModemTimeout -= _TimerSecDef;
          if (_IOT_ModemTimeout <= 0) _IOT_ModemTimeout = TIMER_NEED_RUN;
        }
      // }
// #if HARDWARE_GSM
//       //.................................... Timer For GSM_AllStatus
//       if (_GSM_AllStatusTimeout > 0)
//       {
//         _GSM_AllStatusTimeout -= _TimerSecDef;
//         if (_GSM_AllStatusTimeout <= 0) _GSM_AllStatusTimeout = TIMER_NEED_RUN;
//       }
//       //.................................... Timer For GSM_RSSI
//       if (_GSM_RSSI_CheckupTimeout > 0)
//       {
//         _GSM_RSSI_CheckupTimeout -= _TimerSecDef;
//         if (_GSM_RSSI_CheckupTimeout <= 0) _GSM_RSSI_CheckupTimeout = TIMER_NEED_RUN;
//       }
//       //.................................... Timer For GSM_Money_Request
//       if (_GSM_Money_CheckupTimeout > 0)
//       {
//         _GSM_Money_CheckupTimeout -= _TimerSecDef;
//         if (_GSM_Money_CheckupTimeout <= 0)  _GSM_Money_CheckupTimeout = TIMER_NEED_RUN;
//       }
//       //.................................... Timer For GSM_Money_Response
//       if (_GSM_Money_ResponseTimeout > 0)
//       {
//         _GSM_Money_ResponseTimeout -= _TimerSecDef;
//         if (_GSM_Money_ResponseTimeout <= 0)  _GSM_Money_ResponseTimeout = TIMER_NEED_RUN;
//       }
//       //.................................... Timer For GSM_Operator_Autofind
//       if (_GSM_Operator_AutofindTimeout > 0)
//       {
//         _GSM_Operator_AutofindTimeout -= _TimerSecDef;
//         if (_GSM_Operator_AutofindTimeout <= 0)  _GSM_Operator_AutofindTimeout = TIMER_NEED_RUN;
//       }
// #endif
    }
  }
  catch (...)
  {
  }
}

const String serverURL = "https://app.airngin.com/api/v1.0/DeviceCloud/";
#include <ArduinoJson.h>  // برای پردازش JSON

bool AirNginClient::addDataToCloud(String key, String value) {
  if (WiFi.status() == WL_CONNECTED) {

    // بررسی اینکه هیچ‌یک از مقادیر ورودی خالی نباشد
    if (String(instance->_MqttUser).isEmpty() || 
        String(instance->_MqttPass).isEmpty() ||
        String(instance->_SerialCloud).isEmpty() ||
        String(instance->_ProjectCode).isEmpty() ||
        key.isEmpty()) {
      instance->Debug_ConsolePrintln("Error: One or more required parameters are empty!");
      return "";  // مقدار خالی برگردانده می‌شود
    }
    
    HTTPClient http;
    http.begin(serverURL + "AddNewData");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // ساخت داده ارسالی
    String requestBody = "Username=" + String(instance->_MqttUser) +
                         "&Password=" + String(instance->_MqttPass) +
                         "&DeviceSerial=" + String(instance->_SerialCloud) +
                         "&ProjectCode=" + String(instance->_ProjectCode) +
                         "&Key=" + key +
                         "&Value=" + value;

    int httpResponseCode = http.POST(requestBody);
    String response = "";

    if (httpResponseCode > 0) {
      response = http.getString();
      instance->Debug_ConsolePrintln("Response: " + response);

      // پردازش JSON برای بررسی مقدار `isSuccess`
      StaticJsonDocument<512> jsonDoc;  
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (!error) {
        bool isSuccess = jsonDoc["isSuccess"];
        if (isSuccess) {
          return true;
        } else {
          instance->Debug_ConsolePrintln("Error: Operation failed!");
          instance->Debug_ConsolePrintln("Response: " + response);

          return false;  // در صورت ناموفق بودن مقدار خالی برمی‌گرداند
        }
      } else {
        instance->Debug_ConsolePrintln("Error: Failed to parse JSON!");
        return false;
      }
    } else {
      instance->Debug_ConsolePrintln("Error sending request");
      return false;
    }

    http.end();
  }
  
  return "";  // مقدار خالی در صورتی که WiFi قطع باشد
}

String AirNginClient::getDataFromCloud(String key) {
  if (WiFi.status() == WL_CONNECTED) {

    // بررسی اینکه هیچ‌یک از مقادیر ورودی خالی نباشد
    if (String(instance->_MqttUser).isEmpty() || 
        String(instance->_MqttPass).isEmpty() ||
        String(instance->_SerialCloud).isEmpty() ||
        String(instance->_ProjectCode).isEmpty() ||
        key.isEmpty()) {
      instance->Debug_ConsolePrintln("Error: One or more required parameters are empty!");
      return "";
    }

    HTTPClient http;
    
    // ارسال پارامترها در QueryString و نه در Body
    String requestURL = serverURL + "GetDeviceCloud?"
                        "Username=" + String(instance->_MqttUser) +
                        "&Password=" + String(instance->_MqttPass) +
                        "&DeviceSerial=" + String(instance->_SerialCloud) +
                        "&ProjectCode=" + String(instance->_ProjectCode) +
                        "&Key=" + key;

    http.begin(requestURL);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // ارسال درخواست POST بدون body، زیرا داده‌ها در QueryString هستند
    int httpResponseCode = http.POST("");
    String receivedValue = ""; // مقدار بازگشتی

    if (httpResponseCode > 0) {
      String response = http.getString();
      instance->Debug_ConsolePrintln("Received Data: " + response);

      // **تجزیه‌ی JSON برای استخراج `isSuccess` و `data.value`**
      StaticJsonDocument<512> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (!error) {
        bool isSuccess = jsonDoc["isSuccess"];
        if (isSuccess) {
          receivedValue = jsonDoc["data"]["value"].as<String>(); // مقدار دریافت‌شده را دریافت کن
          instance->Debug_ConsolePrintln("Data Retrieved Successfully: " + receivedValue);
        } else {
          instance->Debug_ConsolePrintln("Error: Operation was not successful");
        }
      } else {
        instance->Debug_ConsolePrintln("JSON Parsing Error");
      }
    } else {
      instance->Debug_ConsolePrintln("Error receiving data");
    }

    http.end();
    return receivedValue; // مقدار دریافت‌شده را برمی‌گرداند
  }
  return "";
}

#include <ArduinoJson.h>

bool AirNginClient::deleteDataFromCloud(String key) {
  if (WiFi.status() == WL_CONNECTED) {

    // بررسی اینکه هیچ‌یک از مقادیر ورودی خالی نباشد
    if (String(instance->_MqttUser).isEmpty() || 
        String(instance->_MqttPass).isEmpty() ||
        String(instance->_SerialCloud).isEmpty() ||
        String(instance->_ProjectCode).isEmpty() ||
        key.isEmpty()) {
      instance->Debug_ConsolePrintln("Error: One or more required parameters are empty!");
      return false;
    }

    HTTPClient http;

    // ساخت URL برای حذف داده
    String requestURL = serverURL + "DeleteData?"
                        "Username=" + String(instance->_MqttUser) +
                        "&Password=" + String(instance->_MqttPass) +
                        "&DeviceSerial=" + String(instance->_SerialCloud) +
                        "&ProjectCode=" + String(instance->_ProjectCode) +
                        "&Key=" + key;

    http.begin(requestURL);
    
    // در صورتی که `sendRequest("DELETE")` مشکل داشت، از `GET` استفاده کنید
    int httpResponseCode = http.sendRequest("DELETE"); // در صورت خطا، می‌توان از http.GET() استفاده کرد.

    bool isSuccess = false;

    if (httpResponseCode > 0) {
      String response = http.getString();
      instance->Debug_ConsolePrintln("Delete Response: " + response);

      // **تجزیه‌ی JSON برای استخراج `isSuccess`**
      StaticJsonDocument<512> jsonDoc;
      DeserializationError error = deserializeJson(jsonDoc, response);

      if (!error) {
        isSuccess = jsonDoc["isSuccess"];
      } else {
        instance->Debug_ConsolePrintln("JSON Parsing Error");
      }
    } else {
      instance->Debug_ConsolePrintln("Error deleting data");
    }

    http.end();
    return isSuccess; // برگرداندن مقدار true در صورت موفقیت، false در غیر این صورت
  }
  return false;
}



//-----------------------------------------------------------------------------------------------
//-------------------------------- Set/Get Settings From EPPROM  --------------------------------
//-----------------------------------------------------------------------------------------------

  const String AirNginClient::rootCACertificate = "-----BEGIN CERTIFICATE-----\n" \
"MIIDuzCCAqOgAwIBAgIDBETAMA0GCSqGSIb3DQEBBQUAMH4xCzAJBgNVBAYTAlBM\n" \
"MSIwIAYDVQQKExlVbml6ZXRvIFRlY2hub2xvZ2llcyBTLkEuMScwJQYDVQQLEx5D\n" \
"ZXJ0dW0gQ2VydGlmaWNhdGlvbiBBdXRob3JpdHkxIjAgBgNVBAMTGUNlcnR1bSBU\n" \
"cnVzdGVkIE5ldHdvcmsgQ0EwHhcNMDgxMDIyMTIwNzM3WhcNMjkxMjMxMTIwNzM3\n" \
"WjB+MQswCQYDVQQGEwJQTDEiMCAGA1UEChMZVW5pemV0byBUZWNobm9sb2dpZXMg\n" \
"Uy5BLjEnMCUGA1UECxMeQ2VydHVtIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MSIw\n" \
"IAYDVQQDExlDZXJ0dW0gVHJ1c3RlZCBOZXR3b3JrIENBMIIBIjANBgkqhkiG9w0B\n" \
"AQEFAAOCAQ8AMIIBCgKCAQEA4/t9o3K6wvDJFIf1awFO4W5AB7ptJ11/91sts1rH\n" \
"UV+rpDKmYYe2bg+G0jACl/jXaVehGDldamR5xgFZrDwxSjh80gTSSyjoIF87B6LM\n" \
"TXPb865Px1bVWqeWifrzq2jUI4ZZJ88JJ7ysbnKDHDBy3+Ci6dLhdHUZvSqeexVU\n" \
"BBvXQzmtVSjF4hq79MDkrjhJM8x2hZ85RdKknvISjFH4fOQtf/WsX+sWn7Et0brM\n" \
"kUJ3TCXJkDhv2/DM+44el1k+1WBO5gUo7Ul5E0u6SNsv+XLTOcr+H9g0cvW0QM8x\n" \
"AcPs3hEtF10fuFDRXhmnad4HMyjKUJX5p1TLVIZQRan5SQIDAQABo0IwQDAPBgNV\n" \
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBQIds3LB/8k9sXN7buQvOKEN0Z19zAOBgNV\n" \
"HQ8BAf8EBAMCAQYwDQYJKoZIhvcNAQEFBQADggEBAKaorSLOAT2mo/9i0Eidi15y\n" \
"sHhE49wcrwn9I0j6vSrEuVUEtRCjjSfeC4Jj0O7eDDd5QVsisrCaQVymcODU0HfL\n" \
"I9MA4GxWL+FpDQ3Zqr8hgVDZBqWo/5U30Kr+4rP1mS1FhIrlQgnXdAIv94nYmem8\n" \
"J9RHjboNRhx3zxSkHLmkMcScKHQDNP8zGSal6Q10tz6XxnboJ5ajZt3hrvJBW8qY\n" \
"VoNzcOSGGtIxQbovvi0TWnZvTuhOgQ4/WwMioBK+ZlgRSssDxLQqKi2WF+A5VLxI\n" \
"03YnnZotBqbJ7DnSq9ufmgsnAjUpsUCV5/nonFWIGUbWtzT1fs45mtk48VH3Tyw=\n" \
"-----END CERTIFICATE-----\n";

    
    /*---- Logo ----*/
    const char  AirNginClient::PROGMEM_LOGO[] PROGMEM = "data:image/jpeg;base64,iVBORw0KGgoAAAANSUhEUgAAASgAAAEoCAYAAADrB2wZAAAgAElEQVR4nO2dB/QcZdXGn1QgCQFCSRCpIUCQIh1pUhMQMCgaaUovotKbdEKRIgoiCgICFjoWehBRRLqEEqRFUdAQSKGEBIHwT77z8j2r63jvzOzszO7M7vM7JyfJu7NT3tm5c/sLIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCFETvTKvJu7x+sedC8LAVgHwFoAlgewHICBAOYD8AGA2QD+DuAlAE8AeBzA290+aV3LqNGZr7xvt8+dSE0QSjsDGANgcwCDG/juTAC/A3ALgJv4fyES6a0pEgksCeA8akNXAPhsg8IJ3H4Mvx/2cy73K0QsElAijpMB/BXAUQCG5DRTiwI4GsBfAJyk2RdxSEAJi3UBPA3gNAALFDRDAwCMA/AUgLV1F4SFfFAiyv4AfpRiVp4D8ACAiQBeBPAmgPcBDKK/amUAqwP4FP/tsQad6PvRBBTi30hAiXrGJZhdQQhdCeDnACYkzNztdf9eH8CX+WchZ/vLASzFcxDiI2TiiRpnJginswAMB3BkCuEU5VEA3+D3z4nZLpiUp+qOiBoSUAIUHsc7M/EggFUBnEANqhlmADiOZt2jzn5OAXCg7oqABJSgj+h7zkT8AMDG9DflSfBbbUCzzuISJoKKLkc+qO6mD4A7nBk4nWkGaQjZ5EvXZZK/zEzyJPanVnWssd2dAIYCmNftN6mbkYDqboKmsrAxAxekEE7bABgL4NMARhifT2L2+PUA7o3ZTzD5FgRwcGR8cQAXAjik229SN6NavO5lBNMDotzHUhaPTahdxW0TJQioEwE8FLNN+GxDY3w5amSiqjRRiycfVPfyXePKg3m2Y8yMhAjb/Q0Kp8CWdLafELPN9gB6jPHvdPNN6nYkoLqT5SkQogRz6h1nRi5nhK0ZzgDwQ+f7b7CkJsrnmR8luhAJqO7kNOOqJwO41JmNbwPYN2ampgP4PYBf8+/pMdsexJwri+D7mmqMNysYRUWRgOo+FmNGdxQvSXMbJmda3M8WLMsC2ALATvw7/P+LNOssQs7VZs5nVqLmfjEZ6KKDkYDqPqyEzFoJi8U1znhNyPwCwLuRz95l36eNY6KB3n4vNczMEMw5pttvXDciAdVdhM4EXzeu+FvOLOxHjSvKoTHfiXI626tECX6l3YzxuTQpoxwOoF+338BuQwKquzjMeMjfj8kkt3w/42O29/g2c6KinOFs/x0joheE69e6+/Z1HxJQ3YWVsX0RhVSU4E/6uDG+X8YZ298YC9HEbY3xWU60z6sXFB2KBFT34DmaPVPN0m5uA/BPZ/vga/pVTK5T6Mx5jzHuRfSs44fs8t2d7UUHIgHVPVjm2lXMP4qyKYBPGOOe8LmMqQtjKFi+72xnaUBrs19UlNcBXGeMq19UFyEB1R145prXe8nSqh5kG+AoyxhmX/AVDTO2fYzdM6Oc5ZyHFQFcwTELRQciAdUdeOaaVeO2GtMDonjak5dEeaIzbu1nKwArGeOTGjQLRYchAdX5NGquWQ//c8wQj7IIgH2c/RzE/uRRxtMfFeV0Zz+NmIWiw5CA6nws8ynOXPusMe5pQ1ZUsEafmAx0y3Qbm5NZKDoICajOZjW2R4nihestc20ys8Wj9GfCZhxHUVBFCVnk04zxPMxC0UFIQHU2nrl2nzHumWte1Cz0MZ8/YfYGxfQXt84tL7NQdAgSUJ1LHubaTKYQWHwz5cx5Rcg/MGr4+jgtV9CgWSg6BAmozqURc20+x1w71+kJvieXMK8nFBxvZwidIEC+ZOxjjtM078gYs9BqxaLl0zsYCajOJA9zbU5MN0urn1ToBXUXgFsaOK4lAAfR1LOwHOMHOmah6AAkoDoTy1x7O8ZcO84YCybYv4zx7dnvKUpNeFg5VyvRqR3FMyE9M9QzC71ooag4ElCdh2euneeYa18xzDXEhPEtATSeSZWBPztOeG9/lnY1jP6lKJ5Z6EULRcWRgOo8PHPNerDhmGs/d/w9YZHPTxrj0TQAy4G+PhMsowS/2M3GuBehswRtnFkoKowEVOfhmWtR0wg015Yzxr3yFUsLshIpH3ISQb0SFcvR7ZmFnqnqmYWiwkhAdRaNmmuWwPByjlZxlpvykj6t5MptWewbxcvN8lrBNGIWigojAdVZNGqurWmMezV6lu/JK+YFi5Gt5c+9iJ5lFq6Xk1koKooEVOeQh7n2qFP3thRXb4mSlINktXPZnY3nogSz8CljvFGzcOuEcxIVQgKqc2jEXBvpmGue9mQJg9cAXJ8we1c7DfG841h+pEbNQq/PuaggElCdgWeuef6hRsy1wU4/8bQ9mSw/0sEABhjjnlnomW6WWbgBgBEpz02UHAmozsCLrk0wxpficuJRPHPtaON3EhY1uCTlzIVFGd6LjPXjCjMWllm4W4xZ+Cdj3Ir+iQoiAVV9PHPN054aMdeCIDnCGD8fwIcpZ85b1uoYLsgZJZiFM4xx73p+ZoytlvLcRMmRgKo+lvnjmWsLOeaa57f5qmGK9TgLa8ZxdgPnAmf7rzlm4UvGmGrzOgQJqGrzsQaja42aa5Yz+1J+pxG8pdW9ZdE9s/BwY1tLGHlLY4mKIQFVbawHPM5csx7w841VfEG/zxLGeNYomeVb8vxhnll4tGEWftrY7uGM5yhKhgRUdfGia3mZa1ZC5Q0ApmScsVecVixehC6NWRiWQ/9yZJsg3H6b8RxFyZCAqi5FmmujAQw3xj2TLC1WntOqjhaUxiz8lSF0L3XaxIgKIgFVTfo60bW9HHNtF8Ncm0rHs4WV4xRWC36hydmaCOByY9yrFdzHSPQMZuG+AC4GMMr4jpc5LypIX920SmIlOoYWJMvz4e3Hsbkct5zmU+lgX6zOr/Me97GOsf1ctvpdoIkJCx0VPjDGN6Jm9A/2s6pdz3Se55DI9paQA8to3mri/ETJsPJQ0nH3eN3L9vG648DuZs6PWXBBtJNRozMfXCZe9fCia93MeRJOnYkEVPXw2pV0I9Np1h2TcO392e1BVAz5oKrFNk50rdsIvqqf0Kx7M8W1XwBgdQC3d/vEVQ0JqGpxQeRsp7B528tcIWUm84B66Cgv2/3tTee3tXhD0vfeZ4Z4iAT+kX3W07Alc8Cuzv9yRNFIQFWHw5kzdDfzfx5kTyQrKib+n7F1WfVK3qwgElDVINTcbQpgJy6QKZI5O7I+oNXcTpQcCahqMJfRu2gBrfhfdmCTvPqWKw+y1EZUDAmoavBat09ACoJgOoSBhCheLylRciSgRFUZSi1pC3ZEGOlcx60y76qLBFS1CNGsDbm6b3CYL8mkzUG8l70yRMjKTC9GJOfRzJ3L6+zPkpzoCspRpmqtvGojAVUNFmGmdEhKXLbbJyMloe5vE/ntqo0EVPkZxb7b1qIBwia0PN6Rf8cxKEN3UNFCVOpSbrbk2nYSTukJ7YI/kaI1zPoAjivTiYv/RRpUeVkkJrlwHhM059FPE+5jny6dp1CP9zznKmiaf0n5vStZMiNKjARUeamt93Yf83he4MNXK2n5gE7jXixrma+ulKQb6M1yl9B6ZnaD13sigww/7pK5qiwSUOXkALatDasFP93tk5Ezp9Z13VT6QcmRgCof8zE8rsUn82UI+5V/gXud6qxKLEqEnOTlo4fFwCI/DqCJ/IW6PV6o+S0/0qDKR9olxUU8K7K4OvRoXyWy5ewMqyOLNiABVQ2GsVHdMN2zj+gxggF9OT9BMK3LjHuPL6pNTTXQj728rMYyje3oLO9X1QspGaFl8p3dPglVQQKqfCzD1YGjK+aK5gkRvNM0j9VBTvJyEWrtXpRwyp1X2PEgSTiFpdU3q+D1dSzSoMpDWJnknG6fhJx5EsBPuSpyGp/T2VzI9A+VusoORgKqHBxgCKfJXCSglin9Lh3DfbssYzwttUVopzLj/qEGk1zDqi8HATi3fZcgokhAtZ9tmEAY6sJ+yezm8GC9xFIWUTyhv9YDPMo9mu/yIAHVXpZgn6fj2UdbtJ5dAFzLo4YXwiO6B+VBAqq9hNSBfWjOidayOH1O+9Qd9RYWYouSIAHVXiSYWk/QWg+k5jo4cvRTu2ECqoQElOh0gvN8OQAbA9iW5S8DjWsO0b6n9GsoFxJQ5WEVlmeswQUBFufiAPPq+j4Jm566JdV7OFd9+GfJFH3c/w5gT81t+ZCAaj+7Ms1g826fiDYxBcCnlLZRTiSg2scKbDurzOX2cT+AnQFMizmDgcw7e6fTJ6OMqNSlPawFYKKEU9t4FcARnP844RRYFMAVHTwXpUYaVOsJiyE8TP+SaA3zmAj7EDsZ3MCWymnow/Ys26kLQuuRgGo9v6FwCs7cR5k1/lemHEzlg/MePwe13D7ykWSiFwMM01gwnIVN+J0jJaBajwRUawndCtYBcBiAm1lrJ8rNkTy7rdgML+2yViIHJKBaR9CCFuSiks92y0VXnP3ZLLDGKWqF01rkJG8dwUS7RMKpMowB8KPIye5Bp7loERJQrUOdCarBUgDOj1lZ59hun6BWIhOvHAxlXlStNkwO8dayAIARANan5hQXYf0GVybWogstQAKqfaxIf8YOXCBBaQfVYH4ABwO4oNsnohXIxGs9i9AXNQnAyQDWlnCqHMd3+wS0Cgmo1jKKK9we2E0XXWFOZhSvJ3IJoZD7S90+Oa1AAqp17AZgPH/cotzcTsF0OhNpbzTOVlpUC5APqjUEzennMUcK5t4TAEZz6aN6ZrCotf5lEiKCgwBsbezrQWak128/ixX7y0e2/ZAPI+rauczj+M7GvkOS4jMZX2xz2UpmBWP8Zpq59ecQ/r2j0WYmnMNjkZ5Oc9lWZQPjuOOZnR+dv0/T3K4nzPUXAPw+Mj6OrYHrCdeyqtJGikUCqniGxJRIXAPgItbmDaBgibKL08j/WENAvc7GbBaTjLFv0YyJspEjoDZKUVwbx8dZE1dPL7benWB8L6ywcnRkbA61UYsZnO967gDwPWPbEJy4NTLW3xBOgedYx/epyPhujOiJgpCJVzxXO/O8M0tfHub/rzA6PU5yhNMArj4c5XTnag5m1LCeuTFLLP3YGLuhSeEElvZEhUIQUFc521sLSYwEsHcD25/rBCFuA/ByZGxBdta0uMwYG+VsK3JCAqpY1uCbOkpoTveLurGhhgkROMk5u28a2u8sRgctTjHGLuF3ooSHbmVj3NK0smD5blanyRXlTfbMiuIJ4qApvR8Zm8/QwmpY87IHF++McocxtpbR11zkiARUsVgPxtlc+64eSziFnkXXG+NBMB1ujJ9vRJtAM2QJY9x7yM8yxn7L6GMePFO3Bl3SceEsZBCyvT9njIfkyQuN8aOdlslXO1rh7sbY6zz3evryJSQKQgKqOPqxQX89QWM5wTjiVsbY+c6ZHWyYgnNjth9njAVz7TVjfD12W4iSd8TK2t9GzsP+CpeDinKms++zjbGFWPhrYSVcbuNs+6QxNtzZVuSABFRxrMdIWz2/dmryVjXGfm2M9Xacspc4LWlHOQ+QZzpaWswT7FuVJ39wol+e0LGE+kinI6lnFlrmHLiacxTrfoCLK0QZ5mwrckACqjhWN/b8J2NskGGCTeXS51H2cvKoGjXXXjTGRzhpC0Xl+1hCZwdnBZY8zMKPOdG/0CzwrcjY4nSYR3nDGJMPqkAkoIpjqLFnS8sZREduPW85BcMbGmM3Oubaug2aa5YpGITkXc72zfIrp2Gft3imdd4bx5iFtxnj0TQB0G/1dmSsLwuIo8wxxvo45ytyQAKqOKzQtrVg5Cwj8jTUEFqB64yxbzpXYIXcPXPNiyLmFbnzOM0Y38vIZQLNwj8b455ZeIwxZgUdBhla6XvOy8QSWupqUCASUMVhhfCtaNosRojqWcjxg9zLB/gJmotjaKJEWbFBc83ya01PyH7Pg8sN7SVwXAPnGczCZYzx55gVPoHzFcy7PxrbfYJ5ZfVMcRZVsO7fm7nNhvgfJKCKI5oxjZiIz9PGmNda9mp2QFjPiW7BSeL0zLWg1X3VGPc0k7w5x9jfoY4G6ZmFlnkKltCsw/m61tnGWlHYitbBuX+TnW1FDkhAFYfliF7POZrlL/laRgfsCk6lvRfFOtLwowTt4eIMx87CBYZvJ5jHhzj7sszCPZkb1SiLOZ0lPMG/rjGWV36YMJCAKo6wMOe7kb0PNwp2Qd+S9ZDebmybhCXsQvTpZ8Z4uP9HGePfdRzCRRCE4feN/Xpm3uWOf8jK9E7iduMZ+Fcky7/GKgCWjozNVLFwsUhAFcd7dXV29USTN8GHwjJ1NqFZY5k7URZj14ORxmee7+lAJ5zu1egVhZUuMCSm5s7K41qDKRQLpzjHARRO6xufneE4vq3M9QdbKMi7EgmoYrHe6l6zulNZjR9lDM2IvQ1nLvggB3Pw+bpFJusJpualzjGtB/0yx3FdJNMdDc9LObjQKPQNbMm5OshoWwNG7PbjnHzG+Py1mNwqKxM9i4YrGsCqT0rH3eM1z8ks5Th1Nzfq8cA3+iMxe53GhMWX6TcaTqFkaUE1hjtJn2OdsPvSbVpQdAUnIjnG8QmtTKHs8Rbn6iXmlC3HvKm4ZaPWchzko50Aw1CnRY6oZ9TozNMhAVU89xi1ds84meZgPpIXcWqUrZiaYPEP9meqJ5R9fL6NcxX6Zm0bGfub0eSuxnYZfU8Wn3fKXkCtbKXIWDju9jkdu7NpQkDJxCsey0wJq7gc4Bz5OnaStPKo0jKFxbeecDrZEE4oQfM1K+l0eSfpEhRomzfZp+ptCkVPOB1qCCc40USRMxJQxROSAx83jnKpk2AIRuJGsutAo1zO7z7kfG9N5+G6swQRqWBe/c4YP4dRNIv7eL1e07s4rqGp6JkDw51uBw8UUEAtDGTitYbVmHYQ5W8s0rX6ONUI9Xf78i1vaT2gn+VWduW0jlNjIW5rlZIs4ySXthrPF/U6BcbsmPNZk07w7Z10DrBO707O1WMx+5qP/c+tOV/ZyXMTFvJBVYILneTDR1jEmrSacB/6rUbQ0TuXUb/nnRq1KIP5QFrmygkx0at2cLpjbj7DhRGi+WUWq1HrWpS/8xkUKhNTLEPfjykiaxufnRdjcgoLCajKMMnoDQ7Wi33GqMnLixUZhbJKNR5xuiS0mwmMqkV5ns5xqzdTHixFB7jVJeFZ1u6JRpCTvDJs7iT2rU0tqIio0G7UPCzh9EaJG/9v7WSMr8LrKWLhzJ14HyzhFBJvtyjgmCIGCajWMtlZHAA0RW6js9cLqzdCeMhuYkcCKxP9fZqWM0s6V28wEmn55wYy2nl9ThrNSkwU/aWT4AnmmynnqcVIQLWeh5jx7Pmc9qQpeCkf0EbZhg/bU87adqBQWq8Cjt6az8lzjI/lNlc7fd2T2JRRzxechRJAjXcTJxIrCkY+qPbxSUbevMhcjafYrG0CfSDTKWB6se5scWoR69IEGZGwv2fZQ+lvFZqrESwrSbq2F7jw5p9oqk2va+c7mPWKq7IFy2YxybI1XqbZnSYIITzkJK8sg2nSWYWocczmvbNq8+K4kjVlcWkNZaU/UwP2aPD8ahG/RufqRtY/xqU1iDTISV5ZZrLEYq8GG58NbPCBe55CcJ+KCieww0Bo4vdFZxl3jwENztXLNPfGSji1HwmocnA1zZeTcu7QGHxMX2em9a+qPEF13MREycOchM6svMIeVCsxw1yUAJl45SOYMruyn/bmxtp6SbzBcpFr2fK20xnLAuswV4s0eK3v0Gd1A6OCH3bBfLUe+aA6liGM5G3AN/tw+q1qq4vMYrHrJJpxjzFKaOUPdTqDmTaxPjWsEUwZqK2k8y5N6r9Qs3yUDee06EHRNCGg+lblGruUN5gbNYG5UStQSxjINIV3+IBNYo3d9C6eq5ks+q3N1QhGORfki3gWI3p/5VxNKcE5iwQkoKrBq/xjLZsk/ptp/BPX+E9UBDnJhRClRQJKCFFaJKCEEKVFAkoIUVokoIQQpUUCSghRWiSghBClRQJKCFFaJKCEEKVFAkoIUVokoIQQpUUCSghRWiSghBClRQJKCFFaJKCEEKVFAkoIUVokoIQQpUUCSghRWiSghBClRQJKCFFaJKCEEKVFAkoIUVokoIQQpUUCSghRWiSghBClRQJKCFFaJKCEEKWlr26NEP/FYAAfB7AUgKEABgAYCKAPgA8BzAYwE8DLAKbwb1EQ7RBQmwPYGsA8AHMb+F5v/vk9gN/keD6b8HzCXPRwrD+ANwBcBWBawveXB7A3gEUAfMDr6jG26+FnNXrx+qPb9uLf7wOYDmAGgBcBvARgVnOXmol+AA7kAzuH1xDm500AP+VDmjefArATgPkpFMDzCNd/LYCJOR5vSQCfBrARgLUBDAcwLOV3w/2eBOBpAA8DuB/AE02cS5jXAygga7+l2u9+AoCbU+xjMf4eh3Afvbjf1wB8v24+8+RzADbj/mrPdH/+fn/UzG+kV4ptbO4en+VbHwMwOfMx/0MQCn/PYT/rAPhTzOcvAFidD6bFIL5Bh+RwLknM4I/0XgC3AXimBccM/BzAbs5nQUiNBvBYjsdbifMex5J84LIyH4DdAXwBwJb8f16Ee3QTgOsA/K3BfQbhu0vM51/hS8EjaHlPAljN+fwZvoxfz/F6PwPg9pjPX8So0Stn3XmrfVBn5bSf83Laz5oJn4eJXTDm88VaJJwCiwLYBsC3qEGEH8VWLTju+jGfBa3xUf5I82JEiv0sm/FYQSM7gYLjCgDb5SycQC0s/M7/AuDKGGFhsUGTnw9IOF747CkAI7NcmEPSM7RSMztvpYBaAsCeOe0rvPmWyWE/SSbTOwlm6IcFqcxpCELhHr5RBxR4nBkptgnCcq+cjvduim3ez7DfXWkqn0ENrGh6c07Cy+TklNZK0ly/lfD5PGq1cQyllrdpTtc/M+HzpPOJpZUC6syc93d2zvurKntQdV+uzecftIVjSzqHVwO4BsDSTezDM/PTcFoBmlozBE3yDwB2LtE5mbTKSR7Mk/1y3md4Ix4F4NWc91sU7/GFkPQmncdt+jVwHsvTD7QigLfbeI1nU1M+so3nUM9A+uzizNQoE+nwfoIa1zTO6Rw6fheiBhZMl09y359I2OdZGbW+ogm+soMB/LCE5/YRrRJQ4wra71k5mhZF8SR/BC81KKAWphm7EaMkSb6M4A+7hRGpdnIEzYg92nwewWH8SArhAUa7LgPwMwqnJIIf5666bdah22G3iOshmD/7pIy+tYsf8H6dWsaTa4WAWpgPaBHsSbMiz6hE3pwE4KEM+wya4bN8EIIP46AUb7rNGFXLFGLNkd2pSW3bYCpJnoxPKZyu4vz+o4ljP84/JzMlYydGGY/NKWpdNKfwfhX1nGamFT6o01JsM4eqdLiZr/DvaXyzJZG3bytvsqdy/DeXMOqUxOElue5tmMKxSBuOfVTKCOeuzBlqRjjVM4e5RltTg6yCcKrx1TJqekULqBCi/4bz2ZuU2OvSh7Is1eNl6cxchuPrMnltqrOffWnelJU8naN3Mb8mjg1K5JBdi+bQ8BYec4mUaSibpZjLbuPzdJ6XxqFftICKC6/uT5Plcb5p/lVnDsyjU/lVfn4Zk9Q8ivJxlZFrEs5pYSbEloWl6XBuxFHdDKen+O5YZn2L/2VT3q+02fSFUqSAGhhjbvwzgzo5nslvFl/lg9kNvJfiGvMyK/NiQTqsdyj4OIPolI7jVgA3tuSqq8tIar5pfHiFUqSAOoGRFIsTM+7zmzGflTIKUQA7JuxyZkH1cXlwK30+RfHZFIGfr5d0bsrGEkzobGtUuKgo3vx0VFpMZ+JcFm6i2WeZMIfSpEzKbK0ym8b49Go8RXO5VfQwO/shRhCTnPQ/pvnwrQLOb8uEz+9nECYLtSLiLJUDfRjwiav7bBXh+TiOtaz7MD3Coz+L88e2S+ssSkAdF5NomCaqF8eJ/JFbhJD+0blcQbkIpsvXUmbPe3NTFNfXaa/BDH8ewKUJxzqLb+i8I45JuWJ3ZNxv0Louyvjdep5mhC+pQ0aRnFGXrnIngHMAHJNwvBs4Bxe3+mSLMPH6xZQ8vJPDRV4ZU7N0OH1fZST4YRZIcV79+PBuyAjlT5jkmUY4TWZeTyuJmpM/YjQoicMch38zGnCSY/f5jPs9LOP3oqzBxNt2Es0ZPDZlidL3nWBUoRZLERrUMTTxLM6O9ETKSojUXGB8N6jSx9P/VTYWobBZmep1rdC41l+pL4XrEKZN9M9w/mnypPLGKlT+Jft+3ZPwGwt5SIszobPWF+vjGc+vf4oXQFKxrUdTBa8R2lVcXmOQMXYuX/qXJ3z3JL48D6obWyH/U/wPeQuovhQQFsEGPz+n41xEVdWa7KMpwNJEu1rJKxScd1E7ypOQILh9zo3cmuU+th75HWsxPbbmeZ9HAZM12JGms4T34kwizwhxuzLrk7iCpuevE7Y7kDWfV1I4ec97LuRt4h0e0/rjuzkWTM6l7WzRLyHa104eYEvZW3I8h9/Q95Jnl9G8mMiC2pcS9jeSvrOLqVFlYW4KTSfr2/6ckhb75s0tDMQkVXBsxbrFcRk1/dTkKaB6UQW0mJdjs7oa345pgXFsg90AWkmYizEU2M1wB8Pqo1h1X1b+yYzyx1twfpMSPh+dcb+X11U1rGn8WYNjXyxxikda/siXSjsd+f8mTxPv0Jjuk5cU4Ex7j34oK2o3H31hZa7TO4KJp2mDBjcyxeJJhn7zaHncKmby4R5PgVoUD7JQ12MH+gKz+JSmpBA+T7N3u+UfrRLPUeDel7LDaWHkqUHF+Q6KSqKM08qOr8CqNT9gZ8w0jtOJjCZdVTHhVM/oguvfkqoTenPOi2R2weZutUoAAASaSURBVPtvFVOo+T7azpPIS0AdzEZeFtfGFPo2y1sxkYcB1FLKzp1cmCFJ6IxjJnZZTde07JpTTpHFSyx2jWOXmEUg8iCrI76MzGbx+Z3tOre8BFRcgWbRIf9TYj47sSKLkz5PP0bSw7UD2/u2Ve3OgUMS7lszpHkphZVqvlTQ8d8oaL/t5DN0irecPB7e/WNWNrkrw9I7jfJqjGq/IB+GKvAO656SEi1Xoq+jSF9OKxhXUF3c4ymTVa9j/k/eGunWOe+vLHw5h8BOw+QhoOIc0a1qoh+Xi1HUm7oo9mZNYRzz0+Fc9cLXiwvSZPZOsbYeGGB5kfWNzS4fthXD9EnF0FU20Y9ootA/E80KqL1i8lbu55u+FbwYkwe0MOvYqsTpbJubxEUF+nNaxQ11K03nycZMcUgirIbzPWr617F1z4YpOoEOoxZ7JgMY96ToNIGUXWLLzJmco5bQbJQrriK91a1nj4wRiOPaUejYJNfQ6XtHwsPydfqkxlQ4mfC3TEO4NybY0igzGIW6i4saJDGY2lxNo5vNvuLT2B2ih6VUC1A4LZVBG7qNofuqcwnn5aair6MZDeorMcWZD7QoMa+eicwPshgSqR+qCg8zwpekiY6m8zzzEtMlYAITBLO2Q7GYTsGXRcscyBYrQZvaglreFvz/cg0Kpx62H9qxxa1wiuRmzkdPkQdpRkDFtU05tIn9NkNcBOcU40eVpEH2SZijPjFN+eq3aYbJTJpLqpFakYIsqSdSoyTNUZ65Zn+nkHoqYbtG5/QQBiDubeLcsnIdXxxp6lCbneveKeYmz/v1e9ZbxiW+NnW8ZgSUt5Ltw23Qnmo8EbOu2bAMkzUgYUXZD1K0180rzWGnFBnK/QvwScUV+qKAZdffpEn2u5htsnQE+AMd2aPYw6pIX9Bs1hauz7yvv6b8XtJcJ+VYfUhTNY5mX5hRnuYL1Mvj86pLUtHMw3OqUZn9ZgErCDfK/oZEn8dWL1H1+m7WHlnMYZHoOzHHf42Zyd4Dc3/ORbyHMynWy7WZVkDU8pSYNiWvsE9Q3vRQE/xpZG57qJE828TxfsNkzeUYcftpTK/7RvgnBd++rNvbl6s9N8JJMStDT2LHgTjeZZcPT/g+UZDf6B8UUvdGgh0fpFzEwiV7c/27P1obcmm+tedSMk8tScvdwexb00MhPCfBt7EMzb+awO3Dt2Daws+PUZOo2eNpjtkMC/L66l8QvSkwiyi1WIjR2tr11X43Lxftg6ibW/AFU9Rac8OZYzayzgm+KLWW+Tm/c3kO7zL/bjIjyM/RLM2jlYo11/MazCccxlZE9b/HnhaVSNVkQjjvWR/9JkdlrdEWQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghhBBCCCGEEEIIIYQQQgghuhsA/wdmkdu+jbZz+gAAAABJRU5ErkJggg==";
    
    
    
    /*---- jQ v3.2.1 ----*/
    const char PROGMEM_JS_JQUERY1[] PROGMEM = "!function(a,b){\"use strict\";\"object\"==typeof module&&\"object\"==typeof module.exports?module.exports=a.document?b(a,!0):function(a){if(!a.document)throw new Error(\"jQuery requires a window with a document\");return b(a)}:b(a)}(\"undefined\"!=typeof window?window:this,function(a,b){\"use strict\";var c=[],d=a.document,e=Object.getPrototypeOf,f=c.slice,g=c.concat,h=c.push,i=c.indexOf,j={},k=j.toString,l=j.hasOwnProperty,m=l.toString,n=m.call(Object),o={};function p(a,b){b=b||d;var c=b.createElement(\"script\");c.text=a,b.head.appendChild(c).parentNode.removeChild(c)}var q=\"3.2.1\",r=function(a,b){return new r.fn.init(a,b)},s=/^[\\s\\uFEFF\\xA0]+|[\\s\\uFEFF\\xA0]+$/g,t=/^-ms-/,u=/-([a-z])/g,v=function(a,b){return b.toUpperCase()};r.fn=r.prototype={jquery:q,constructor:r,length:0,toArray:function(){return f.call(this)},get:function(a){return null==a?f.call(this):a<0?this[a+this.length]:this[a]},pushStack:function(a){var b=r.merge(this.constructor(),a);return b.prevObject=this,b},each:function(a){return r.each(this,a)},map:function(a){return this.pushStack(r.map(this,function(b,c){return a.call(b,c,b)}))},slice:function(){return this.pushStack(f.apply(this,arguments))},first:function(){return this.eq(0)},last:function(){return this.eq(-1)},eq:function(a){var b=this.length,c=+a+(a<0?b:0);return this.pushStack(c>=0&&c<b?[this[c]]:[])},end:function(){return this.prevObject||this.constructor()},push:h,sort:c.sort,splice:c.splice},r.extend=r.fn.extend=function(){var a,b,c,d,e,f,g=arguments[0]||{},h=1,i=arguments.length,j=!1;for(\"boolean\"==typeof g&&(j=g,g=arguments[h]||{},h++),\"object\"==typeof g||r.isFunction(g)||(g={}),h===i&&(g=this,h--);h<i;h++)if(null!=(a=arguments[h]))for(b in a)c=g[b],d=a[b],g!==d&&(j&&d&&(r.isPlainObject(d)||(e=Array.isArray(d)))?(e?(e=!1,f=c&&Array.isArray(c)?c:[]):f=c&&r.isPlainObject(c)?c:{},g[b]=r.extend(j,f,d)):void 0!==d&&(g[b]=d));return g},r.extend({expando:\"jQuery\"+(q+Math.random()).replace(/\\D/g,\"\"),isReady:!0,error:function(a){throw new Error(a)},noop:function(){},isFunction:function(a){return\"function\"===r.type(a)},isWindow:function(a){return null!=a&&a===a.window},isNumeric:function(a){var b=r.type(a);return(\"number\"===b||\"string\"===b)&&!isNaN(a-parseFloat(a))},isPlainObject:function(a){var b,c;return!(!a||\"[object Object]\"!==k.call(a))&&(!(b=e(a))||(c=l.call(b,\"constructor\")&&b.constructor,\"function\"==typeof c&&m.call(c)===n))},isEmptyObject:function(a){var b;for(b in a)return!1;return!0},type:function(a){return null==a?a+\"\":\"object\"==typeof a||\"function\"==typeof a?j[k.call(a)]||\"object\":typeof a},globalEval:function(a){p(a)},camelCase:function(a){return a.replace(t,\"ms-\").replace(u,v)},each:function(a,b){var c,d=0;if(w(a)){for(c=a.length;d<c;d++)if(b.call(a[d],d,a[d])===!1)break}else for(d in a)if(b.call(a[d],d,a[d])===!1)break;return a},trim:function(a){return null==a?\"\":(a+\"\").replace(s,\"\")},makeArray:function(a,b){var c=b||[];return null!=a&&(w(Object(a))?r.merge(c,\"string\"==typeof a?[a]:a):h.call(c,a)),c},inArray:function(a,b,c){return null==b?-1:i.call(b,a,c)},merge:function(a,b){for(var c=+b.length,d=0,e=a.length;d<c;d++)a[e++]=b[d];return a.length=e,a},grep:function(a,b,c){for(var d,e=[],f=0,g=a.length,h=!c;f<g;f++)d=!b(a[f],f),d!==h&&e.push(a[f]);return e},map:function(a,b,c){var d,e,f=0,h=[];if(w(a))for(d=a.length;f<d;f++)e=b(a[f],f,c),null!=e&&h.push(e);else for(f in a)e=b(a[f],f,c),null!=e&&h.push(e);return g.apply([],h)},guid:1,proxy:function(a,b){var c,d,e;if(\"string\"==typeof b&&(c=a[b],b=a,a=c),r.isFunction(a))return d=f.call(arguments,2),e=function(){return a.apply(b||this,d.concat(f.call(arguments)))},e.guid=a.guid=a.guid||r.guid++,e},now:Date.now,support:o}),\"function\"==typeof Symbol&&(r.fn[Symbol.iterator]=c[Symbol.iterator]),r.each(\"Boolean Number String Function Array Date RegExp Object Error Symbol\".split(\" \"),function(a,b){j[\"[object \"+b+\"]\"]=b.toLowerCase()});function w(a){var b=!!a&&\"length\"in a&&a.length,c=r.type(a);return\"function\"!==c&&!r.isWindow(a)&&(\"array\"===c||0===b||\"number\"==typeof b&&b>0&&b-1 in a)}var x=function(a){var b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u=\"sizzle\"+1*new Date,v=a.document,w=0,x=0,y=ha(),z=ha(),A=ha(),B=function(a,b){return a===b&&(l=!0),0},C={}.hasOwnProperty,D=[],E=D.pop,F=D.push,G=D.push,H=D.slice,I=function(a,b){for(var c=0,d=a.length;c<d;c++)if(a[c]===b)return c;return-1},J=\"checked|selected|async|autofocus|autoplay|controls|defer|disabled|hidden|ismap|loop|multiple|open|readonly|required|scoped\",K=\"[\\\\x20\\\\t\\\\r\\\\n\\\\f]\",L=\"(?:\\\\\\\\.|[\\\\w-]|[^\\0-\\\\xa0])+\",M=\"\\\\[\"+K+\"*(\"+L+\")(?:\"+K+\"*([*^$|!~]?=)\"+K+\"*(?:'((?:\\\\\\\\.|[^\\\\\\\\'])*)'|\\\"((?:\\\\\\\\.|[^\\\\\\\\\\\"])*)\\\"|(\"+L+\"))|)\"+K+\"*\\\\]\",N=\":(\"+L+\")(?:\\\\((('((?:\\\\\\\\.|[^\\\\\\\\'])*)'|\\\"((?:\\\\\\\\.|[^\\\\\\\\\\\"])*)\\\")|((?:\\\\\\\\.|[^\\\\\\\\()[\\\\]]|\"+M+\")*)|.*)\\\\)|)\",O=new RegExp(K+\"+\",\"g\"),P=new RegExp(\"^\"+K+\"+|((?:^|[^\\\\\\\\])(?:\\\\\\\\.)*)\"+K+\"+$\",\"g\"),Q=new RegExp(\"^\"+K+\"*,\"+K+\"*\"),R=new RegExp(\"^\"+K+\"*([>+~]|\"+K+\")\"+K+\"*\"),S=new RegExp(\"=\"+K+\"*([^\\\\]'\\\"]*?)\"+K+\"*\\\\]\",\"g\"),T=new RegExp(N),U=new RegExp(\"^\"+L+\"$\"),V={ID:new RegExp(\"^#(\"+L+\")\"),CLASS:new RegExp(\"^\\\\.(\"+L+\")\"),TAG:new RegExp(\"^(\"+L+\"|[*])\"),ATTR:new RegExp(\"^\"+M),PSEUDO:new RegExp(\"^\"+N),CHILD:new RegExp(\"^:(only|first|last|nth|nth-last)-(child|of-type)(?:\\\\(\"+K+\"*(even|odd|(([+-]|)(\\\\d*)n|)\"+K+\"*(?:([+-]|)\"+K+\"*(\\\\d+)|))\"+K+\"*\\\\)|)\",\"i\"),bool:new RegExp(\"^(?:\"+J+\")$\",\"i\"),needsContext:new RegExp(\"^\"+K+\"*[>+~]|:(even|odd|eq|gt|lt|nth|first|last)(?:\\\\(\"+K+\"*((?:-\\\\d)?\\\\d*)\"+K+\"*\\\\)|)(?=[^-]|$)\",\"i\")},W=/^(?:input|select|textarea|button)$/i,X=/^h\\d$/i,Y=/^[^{]+\\{\\s*\\[native \\w/,Z=/^(?:#([\\w-]+)|(\\w+)|\\.([\\w-]+))$/,$=/[+~]/,_=new RegExp(\"\\\\\\\\([\\\\da-f]{1,6}\"+K+\"?|(\"+K+\")|.)\",\"ig\"),aa=function(a,b,c){var d=\"0x\"+b-65536;return d!==d||c?b:d<0?String.fromCharCode(d+65536):String.fromCharCode(d>>10|55296,1023&d|56320)},ba=/([\\0-\\x1f\\x7f]|^-?\\d)|^-$|[^\\0-\\x1f\\x7f-\\uFFFF\\w-]/g,ca=function(a,b){return b?\"\\0\"===a?\"\\ufffd\":a.slice(0,-1)+\"\\\\\"+a.charCodeAt(a.length-1).toString(16)+\" \":\"\\\\\"+a},da=function(){m()},ea=ta(function(a){return a.disabled===!0&&(\"form\"in a||\"label\"in a)},{dir:\"parentNode\",next:\"legend\"});try{G.apply(D=H.call(v.childNodes),v.childNodes),D[v.childNodes.length].nodeType}catch(fa){G={apply:D.length?function(a,b){F.apply(a,H.call(b))}:function(a,b){var c=a.length,d=0;while(a[c++]=b[d++]);a.length=c-1}}}function ga(a,b,d,e){var f,h,j,k,l,o,r,s=b&&b.ownerDocument,w=b?b.nodeType:9;if(d=d||[],\"string\"!=typeof a||!a||1!==w&&9!==w&&11!==w)return d;if(!e&&((b?b.ownerDocument||b:v)!==n&&m(b),b=b||n,p)){if(11!==w&&(l=Z.exec(a)))if(f=l[1]){if(9===w){if(!(j=b.getElementById(f)))return d;if(j.id===f)return d.push(j),d}else if(s&&(j=s.getElementById(f))&&t(b,j)&&j.id===f)return d.push(j),d}else{if(l[2])return G.apply(d,b.getElementsByTagName(a)),d;if((f=l[3])&&c.getElementsByClassName&&b.getElementsByClassName)return G.apply(d,b.getElementsByClassName(f)),d}if(c.qsa&&!A[a+\" \"]&&(!q||!q.test(a))){if(1!==w)s=b,r=a;else if(\"object\"!==b.nodeName.toLowerCase()){(k=b.getAttribute(\"id\"))?k=k.replace(ba,ca):b.setAttribute(\"id\",k=u),o=g(a),h=o.length;while(h--)o[h]=\"#\"+k+\" \"+sa(o[h]);r=o.join(\",\"),s=$.test(a)&&qa(b.parentNode)||b}if(r)try{return G.apply(d,s.querySelectorAll(r)),d}catch(x){}finally{k===u&&b.removeAttribute(\"id\")}}}return i(a.replace(P,\"$1\"),b,d,e)}function ha(){var a=[];function b(c,e){return a.push(c+\" \")>d.cacheLength&&delete b[a.shift()],b[c+\" \"]=e}return b}function ia(a){return a[u]=!0,a}function ja(a){var b=n.createElement(\"fieldset\");try{return!!a(b)}catch(c){return!1}";
    const char PROGMEM_JS_JQUERY2[] PROGMEM = "finally{b.parentNode&&b.parentNode.removeChild(b),b=null}}function ka(a,b){var c=a.split(\"|\"),e=c.length;while(e--)d.attrHandle[c[e]]=b}function la(a,b){var c=b&&a,d=c&&1===a.nodeType&&1===b.nodeType&&a.sourceIndex-b.sourceIndex;if(d)return d;if(c)while(c=c.nextSibling)if(c===b)return-1;return a?1:-1}function ma(a){return function(b){var c=b.nodeName.toLowerCase();return\"input\"===c&&b.type===a}}function na(a){return function(b){var c=b.nodeName.toLowerCase();return(\"input\"===c||\"button\"===c)&&b.type===a}}function oa(a){return function(b){return\"form\"in b?b.parentNode&&b.disabled===!1?\"label\"in b?\"label\"in b.parentNode?b.parentNode.disabled===a:b.disabled===a:b.isDisabled===a||b.isDisabled!==!a&&ea(b)===a:b.disabled===a:\"label\"in b&&b.disabled===a}}function pa(a){return ia(function(b){return b=+b,ia(function(c,d){var e,f=a([],c.length,b),g=f.length;while(g--)c[e=f[g]]&&(c[e]=!(d[e]=c[e]))})})}function qa(a){return a&&\"undefined\"!=typeof a.getElementsByTagName&&a}c=ga.support={},f=ga.isXML=function(a){var b=a&&(a.ownerDocument||a).documentElement;return!!b&&\"HTML\"!==b.nodeName},m=ga.setDocument=function(a){var b,e,g=a?a.ownerDocument||a:v;return g!==n&&9===g.nodeType&&g.documentElement?(n=g,o=n.documentElement,p=!f(n),v!==n&&(e=n.defaultView)&&e.top!==e&&(e.addEventListener?e.addEventListener(\"unload\",da,!1):e.attachEvent&&e.attachEvent(\"onunload\",da)),c.attributes=ja(function(a){return a.className=\"i\",!a.getAttribute(\"className\")}),c.getElementsByTagName=ja(function(a){return a.appendChild(n.createComment(\"\")),!a.getElementsByTagName(\"*\").length}),c.getElementsByClassName=Y.test(n.getElementsByClassName),c.getById=ja(function(a){return o.appendChild(a).id=u,!n.getElementsByName||!n.getElementsByName(u).length}),c.getById?(d.filter.ID=function(a){var b=a.replace(_,aa);return function(a){return a.getAttribute(\"id\")===b}},d.find.ID=function(a,b){if(\"undefined\"!=typeof b.getElementById&&p){var c=b.getElementById(a);return c?[c]:[]}}):(d.filter.ID=function(a){var b=a.replace(_,aa);return function(a){var c=\"undefined\"!=typeof a.getAttributeNode&&a.getAttributeNode(\"id\");return c&&c.value===b}},d.find.ID=function(a,b){if(\"undefined\"!=typeof b.getElementById&&p){var c,d,e,f=b.getElementById(a);if(f){if(c=f.getAttributeNode(\"id\"),c&&c.value===a)return[f];e=b.getElementsByName(a),d=0;while(f=e[d++])if(c=f.getAttributeNode(\"id\"),c&&c.value===a)return[f]}return[]}}),d.find.TAG=c.getElementsByTagName?function(a,b){return\"undefined\"!=typeof b.getElementsByTagName?b.getElementsByTagName(a):c.qsa?b.querySelectorAll(a):void 0}:function(a,b){var c,d=[],e=0,f=b.getElementsByTagName(a);if(\"*\"===a){while(c=f[e++])1===c.nodeType&&d.push(c);return d}return f},d.find.CLASS=c.getElementsByClassName&&function(a,b){if(\"undefined\"!=typeof b.getElementsByClassName&&p)return b.getElementsByClassName(a)},r=[],q=[],(c.qsa=Y.test(n.querySelectorAll))&&(ja(function(a){o.appendChild(a).innerHTML=\"<a id='\"+u+\"'></a><select id='\"+u+\"-\\r\\\\' msallowcapture=''><option selected=''></option></select>\",a.querySelectorAll(\"[msallowcapture^='']\").length&&q.push(\"[*^$]=\"+K+\"*(?:''|\\\"\\\")\"),a.querySelectorAll(\"[selected]\").length||q.push(\"\\\\[\"+K+\"*(?:value|\"+J+\")\"),a.querySelectorAll(\"[id~=\"+u+\"-]\").length||q.push(\"~=\"),a.querySelectorAll(\":checked\").length||q.push(\":checked\"),a.querySelectorAll(\"a#\"+u+\"+*\").length||q.push(\".#.+[+~]\")}),ja(function(a){a.innerHTML=\"<a href='' disabled='disabled'></a><select disabled='disabled'><option/></select>\";var b=n.createElement(\"input\");b.setAttribute(\"type\",\"hidden\"),a.appendChild(b).setAttribute(\"name\",\"D\"),a.querySelectorAll(\"[name=d]\").length&&q.push(\"name\"+K+\"*[*^$|!~]?=\"),2!==a.querySelectorAll(\":enabled\").length&&q.push(\":enabled\",\":disabled\"),o.appendChild(a).disabled=!0,2!==a.querySelectorAll(\":disabled\").length&&q.push(\":enabled\",\":disabled\"),a.querySelectorAll(\"*,:x\"),q.push(\",.*:\")})),(c.matchesSelector=Y.test(s=o.matches||o.webkitMatchesSelector||o.mozMatchesSelector||o.oMatchesSelector||o.msMatchesSelector))&&ja(function(a){c.disconnectedMatch=s.call(a,\"*\"),s.call(a,\"[s!='']:x\"),r.push(\"!=\",N)}),q=q.length&&new RegExp(q.join(\"|\")),r=r.length&&new RegExp(r.join(\"|\")),b=Y.test(o.compareDocumentPosition),t=b||Y.test(o.contains)?function(a,b){var c=9===a.nodeType?a.documentElement:a,d=b&&b.parentNode;return a===d||!(!d||1!==d.nodeType||!(c.contains?c.contains(d):a.compareDocumentPosition&&16&a.compareDocumentPosition(d)))}:function(a,b){if(b)while(b=b.parentNode)if(b===a)return!0;return!1},B=b?function(a,b){if(a===b)return l=!0,0;var d=!a.compareDocumentPosition-!b.compareDocumentPosition;return d?d:(d=(a.ownerDocument||a)===(b.ownerDocument||b)?a.compareDocumentPosition(b):1,1&d||!c.sortDetached&&b.compareDocumentPosition(a)===d?a===n||a.ownerDocument===v&&t(v,a)?-1:b===n||b.ownerDocument===v&&t(v,b)?1:k?I(k,a)-I(k,b):0:4&d?-1:1)}:function(a,b){if(a===b)return l=!0,0;var c,d=0,e=a.parentNode,f=b.parentNode,g=[a],h=[b];if(!e||!f)return a===n?-1:b===n?1:e?-1:f?1:k?I(k,a)-I(k,b):0;if(e===f)return la(a,b);c=a;while(c=c.parentNode)g.unshift(c);c=b;while(c=c.parentNode)h.unshift(c);while(g[d]===h[d])d++;return d?la(g[d],h[d]):g[d]===v?-1:h[d]===v?1:0},n):n},ga.matches=function(a,b){return ga(a,null,null,b)},ga.matchesSelector=function(a,b){if((a.ownerDocument||a)!==n&&m(a),b=b.replace(S,\"='$1']\"),c.matchesSelector&&p&&!A[b+\" \"]&&(!r||!r.test(b))&&(!q||!q.test(b)))try{var d=s.call(a,b);if(d||c.disconnectedMatch||a.document&&11!==a.document.nodeType)return d}catch(e){}return ga(b,n,null,[a]).length>0},ga.contains=function(a,b){return(a.ownerDocument||a)!==n&&m(a),t(a,b)},ga.attr=function(a,b){(a.ownerDocument||a)!==n&&m(a);var e=d.attrHandle[b.toLowerCase()],f=e&&C.call(d.attrHandle,b.toLowerCase())?e(a,b,!p):void 0;return void 0!==f?f:c.attributes||!p?a.getAttribute(b):(f=a.getAttributeNode(b))&&f.specified?f.value:null},ga.escape=function(a){return(a+\"\").replace(ba,ca)},ga.error=function(a){throw new Error(\"Syntax error, unrecognized expression: \"+a)},ga.uniqueSort=function(a){var b,d=[],e=0,f=0;if(l=!c.detectDuplicates,k=!c.sortStable&&a.slice(0),a.sort(B),l){while(b=a[f++])b===a[f]&&(e=d.push(f));while(e--)a.splice(d[e],1)}return k=null,a},e=ga.getText=function(a){var b,c=\"\",d=0,f=a.nodeType;if(f){if(1===f||9===f||11===f){if(\"string\"==typeof a.textContent)return a.textContent;for(a=a.firstChild;a;a=a.nextSibling)c+=e(a)}else if(3===f||4===f)return a.nodeValue}else while(b=a[d++])c+=e(b);return c},d=ga.selectors={cacheLength:50,createPseudo:ia,match:V,attrHandle:{},find:{},relative:{\">\":{dir:\"parentNode\",first:!0},\" \":{dir:\"parentNode\"},\"+\":{dir:\"previousSibling\",first:!0},\"~\":{dir:\"previousSibling\"}},preFilter:{ATTR:function(a){return a[1]=a[1].replace(_,aa),a[3]=(a[3]||a[4]||a[5]||\"\").replace(_,aa),\"~=\"===a[2]&&(a[3]=\" \"+a[3]+\" \"),a.slice(0,4)},CHILD:function(a){return a[1]=a[1].toLowerCase(),\"nth\"===a[1].slice(0,3)?(a[3]||ga.error(a[0]),a[4]=+(a[4]?a[5]+(a[6]||1):2*(\"even\"===a[3]||\"odd\"===a[3])),a[5]=+(a[7]+a[8]||\"odd\"===a[3])):a[3]&&ga.error(a[0]),a},PSEUDO:function(a){var b,c=!a[6]&&a[2];return V.CHILD.test(a[0])?null:(a[3]?a[2]=a[4]||a[5]||\"\":c&&T.test(c)&&(b=g(c,!0))&&(b=c.indexOf(\")\",c.length-b)-c.length)&&(a[0]=a[0].slice(0,b),a[2]=c.slice(0,b)),a.slice(0,3))}},filter:{TAG:function(a){var b=a.replace(_,aa).toLowerCase();return\"*\"===a?function(){return!0}:function(a){return a.nodeName&&a.nodeName.toLowerCase()===b}},CLASS:function(a){var b=y[a+\" \"];return b||(b=new RegExp(\"(^|\"+K+\")\"+a+\"(\"+K+\"|$)\"))&&y(a,function(a){return b.test(\"string\"==typeof a.className&&a.className||\"undefined\"!=typeof a.getAttribute&&a.getAttribute(\"class\")||\"\")})},ATTR:function(a,b,c){return function(d){var e=ga.attr(d,a);return null==e?\"!=\"===b:!b||";
    const char PROGMEM_JS_JQUERY3[] PROGMEM = "(e+=\"\",\"=\"===b?e===c:\"!=\"===b?e!==c:\"^=\"===b?c&&0===e.indexOf(c):\"*=\"===b?c&&e.indexOf(c)>-1:\"$=\"===b?c&&e.slice(-c.length)===c:\"~=\"===b?(\" \"+e.replace(O,\" \")+\" \").indexOf(c)>-1:\"|=\"===b&&(e===c||e.slice(0,c.length+1)===c+\"-\"))}},CHILD:function(a,b,c,d,e){var f=\"nth\"!==a.slice(0,3),g=\"last\"!==a.slice(-4),h=\"of-type\"===b;return 1===d&&0===e?function(a){return!!a.parentNode}:function(b,c,i){var j,k,l,m,n,o,p=f!==g?\"nextSibling\":\"previousSibling\",q=b.parentNode,r=h&&b.nodeName.toLowerCase(),s=!i&&!h,t=!1;if(q){if(f){while(p){m=b;while(m=m[p])if(h?m.nodeName.toLowerCase()===r:1===m.nodeType)return!1;o=p=\"only\"===a&&!o&&\"nextSibling\"}return!0}if(o=[g?q.firstChild:q.lastChild],g&&s){m=q,l=m[u]||(m[u]={}),k=l[m.uniqueID]||(l[m.uniqueID]={}),j=k[a]||[],n=j[0]===w&&j[1],t=n&&j[2],m=n&&q.childNodes[n];while(m=++n&&m&&m[p]||(t=n=0)||o.pop())if(1===m.nodeType&&++t&&m===b){k[a]=[w,n,t];break}}else if(s&&(m=b,l=m[u]||(m[u]={}),k=l[m.uniqueID]||(l[m.uniqueID]={}),j=k[a]||[],n=j[0]===w&&j[1],t=n),t===!1)while(m=++n&&m&&m[p]||(t=n=0)||o.pop())if((h?m.nodeName.toLowerCase()===r:1===m.nodeType)&&++t&&(s&&(l=m[u]||(m[u]={}),k=l[m.uniqueID]||(l[m.uniqueID]={}),k[a]=[w,t]),m===b))break;return t-=e,t===d||t%d===0&&t/d>=0}}},PSEUDO:function(a,b){var c,e=d.pseudos[a]||d.setFilters[a.toLowerCase()]||ga.error(\"unsupported pseudo: \"+a);return e[u]?e(b):e.length>1?(c=[a,a,\"\",b],d.setFilters.hasOwnProperty(a.toLowerCase())?ia(function(a,c){var d,f=e(a,b),g=f.length;while(g--)d=I(a,f[g]),a[d]=!(c[d]=f[g])}):function(a){return e(a,0,c)}):e}},pseudos:{not:ia(function(a){var b=[],c=[],d=h(a.replace(P,\"$1\"));return d[u]?ia(function(a,b,c,e){var f,g=d(a,null,e,[]),h=a.length;while(h--)(f=g[h])&&(a[h]=!(b[h]=f))}):function(a,e,f){return b[0]=a,d(b,null,f,c),b[0]=null,!c.pop()}}),has:ia(function(a){return function(b){return ga(a,b).length>0}}),contains:ia(function(a){return a=a.replace(_,aa),function(b){return(b.textContent||b.innerText||e(b)).indexOf(a)>-1}}),lang:ia(function(a){return U.test(a||\"\")||ga.error(\"unsupported lang: \"+a),a=a.replace(_,aa).toLowerCase(),function(b){var c;do if(c=p?b.lang:b.getAttribute(\"xml:lang\")||b.getAttribute(\"lang\"))return c=c.toLowerCase(),c===a||0===c.indexOf(a+\"-\");while((b=b.parentNode)&&1===b.nodeType);return!1}}),target:function(b){var c=a.location&&a.location.hash;return c&&c.slice(1)===b.id},root:function(a){return a===o},focus:function(a){return a===n.activeElement&&(!n.hasFocus||n.hasFocus())&&!!(a.type||a.href||~a.tabIndex)},enabled:oa(!1),disabled:oa(!0),checked:function(a){var b=a.nodeName.toLowerCase();return\"input\"===b&&!!a.checked||\"option\"===b&&!!a.selected},selected:function(a){return a.parentNode&&a.parentNode.selectedIndex,a.selected===!0},empty:function(a){for(a=a.firstChild;a;a=a.nextSibling)if(a.nodeType<6)return!1;return!0},parent:function(a){return!d.pseudos.empty(a)},header:function(a){return X.test(a.nodeName)},input:function(a){return W.test(a.nodeName)},button:function(a){var b=a.nodeName.toLowerCase();return\"input\"===b&&\"button\"===a.type||\"button\"===b},text:function(a){var b;return\"input\"===a.nodeName.toLowerCase()&&\"text\"===a.type&&(null==(b=a.getAttribute(\"type\"))||\"text\"===b.toLowerCase())},first:pa(function(){return[0]}),last:pa(function(a,b){return[b-1]}),eq:pa(function(a,b,c){return[c<0?c+b:c]}),even:pa(function(a,b){for(var c=0;c<b;c+=2)a.push(c);return a}),odd:pa(function(a,b){for(var c=1;c<b;c+=2)a.push(c);return a}),lt:pa(function(a,b,c){for(var d=c<0?c+b:c;--d>=0;)a.push(d);return a}),gt:pa(function(a,b,c){for(var d=c<0?c+b:c;++d<b;)a.push(d);return a})}},d.pseudos.nth=d.pseudos.eq;for(b in{radio:!0,checkbox:!0,file:!0,password:!0,image:!0})d.pseudos[b]=ma(b);for(b in{submit:!0,reset:!0})d.pseudos[b]=na(b);function ra(){}ra.prototype=d.filters=d.pseudos,d.setFilters=new ra,g=ga.tokenize=function(a,b){var c,e,f,g,h,i,j,k=z[a+\" \"];if(k)return b?0:k.slice(0);h=a,i=[],j=d.preFilter;while(h){c&&!(e=Q.exec(h))||(e&&(h=h.slice(e[0].length)||h),i.push(f=[])),c=!1,(e=R.exec(h))&&(c=e.shift(),f.push({value:c,type:e[0].replace(P,\" \")}),h=h.slice(c.length));for(g in d.filter)!(e=V[g].exec(h))||j[g]&&!(e=j[g](e))||(c=e.shift(),f.push({value:c,type:g,matches:e}),h=h.slice(c.length));if(!c)break}return b?h.length:h?ga.error(a):z(a,i).slice(0)};function sa(a){for(var b=0,c=a.length,d=\"\";b<c;b++)d+=a[b].value;return d}function ta(a,b,c){var d=b.dir,e=b.next,f=e||d,g=c&&\"parentNode\"===f,h=x++;return b.first?function(b,c,e){while(b=b[d])if(1===b.nodeType||g)return a(b,c,e);return!1}:function(b,c,i){var j,k,l,m=[w,h];if(i){while(b=b[d])if((1===b.nodeType||g)&&a(b,c,i))return!0}else while(b=b[d])if(1===b.nodeType||g)if(l=b[u]||(b[u]={}),k=l[b.uniqueID]||(l[b.uniqueID]={}),e&&e===b.nodeName.toLowerCase())b=b[d]||b;else{if((j=k[f])&&j[0]===w&&j[1]===h)return m[2]=j[2];if(k[f]=m,m[2]=a(b,c,i))return!0}return!1}}function ua(a){return a.length>1?function(b,c,d){var e=a.length;while(e--)if(!a[e](b,c,d))return!1;return!0}:a[0]}function va(a,b,c){for(var d=0,e=b.length;d<e;d++)ga(a,b[d],c);return c}function wa(a,b,c,d,e){for(var f,g=[],h=0,i=a.length,j=null!=b;h<i;h++)(f=a[h])&&(c&&!c(f,d,e)||(g.push(f),j&&b.push(h)));return g}function xa(a,b,c,d,e,f){return d&&!d[u]&&(d=xa(d)),e&&!e[u]&&(e=xa(e,f)),ia(function(f,g,h,i){var j,k,l,m=[],n=[],o=g.length,p=f||va(b||\"*\",h.nodeType?[h]:h,[]),q=!a||!f&&b?p:wa(p,m,a,h,i),r=c?e||(f?a:o||d)?[]:g:q;if(c&&c(q,r,h,i),d){j=wa(r,n),d(j,[],h,i),k=j.length;while(k--)(l=j[k])&&(r[n[k]]=!(q[n[k]]=l))}if(f){if(e||a){if(e){j=[],k=r.length;while(k--)(l=r[k])&&j.push(q[k]=l);e(null,r=[],j,i)}k=r.length;while(k--)(l=r[k])&&(j=e?I(f,l):m[k])>-1&&(f[j]=!(g[j]=l))}}else r=wa(r===g?r.splice(o,r.length):r),e?e(null,g,r,i):G.apply(g,r)})}function ya(a){for(var b,c,e,f=a.length,g=d.relative[a[0].type],h=g||d.relative[\" \"],i=g?1:0,k=ta(function(a){return a===b},h,!0),l=ta(function(a){return I(b,a)>-1},h,!0),m=[function(a,c,d){var e=!g&&(d||c!==j)||((b=c).nodeType?k(a,c,d):l(a,c,d));return b=null,e}];i<f;i++)if(c=d.relative[a[i].type])m=[ta(ua(m),c)];else{if(c=d.filter[a[i].type].apply(null,a[i].matches),c[u]){for(e=++i;e<f;e++)if(d.relative[a[e].type])break;return xa(i>1&&ua(m),i>1&&sa(a.slice(0,i-1).concat({value:\" \"===a[i-2].type?\"*\":\"\"})).replace(P,\"$1\"),c,i<e&&ya(a.slice(i,e)),e<f&&ya(a=a.slice(e)),e<f&&sa(a))}m.push(c)}return ua(m)}function za(a,b){var c=b.length>0,e=a.length>0,f=function(f,g,h,i,k){var l,o,q,r=0,s=\"0\",t=f&&[],u=[],v=j,x=f||e&&d.find.TAG(\"*\",k),y=w+=null==v?1:Math.random()||.1,z=x.length;for(k&&(j=g===n||g||k);s!==z&&null!=(l=x[s]);s++){if(e&&l){o=0,g||l.ownerDocument===n||(m(l),h=!p);while(q=a[o++])if(q(l,g||n,h)){i.push(l);break}k&&(w=y)}c&&((l=!q&&l)&&r--,f&&t.push(l))}if(r+=s,c&&s!==r){o=0;while(q=b[o++])q(t,u,g,h);if(f){if(r>0)while(s--)t[s]||u[s]||(u[s]=E.call(i));u=wa(u)}G.apply(i,u),k&&!f&&u.length>0&&r+b.length>1&&ga.uniqueSort(i)}return k&&(w=y,j=v),t};return c?ia(f):f}return h=ga.compile=function(a,b){var c,d=[],e=[],f=A[a+\" \"];if(!f){b||(b=g(a)),c=b.length;while(c--)f=ya(b[c]),f[u]?d.push(f):e.push(f);f=A(a,za(e,d)),f.selector=a}return f},i=ga.select=function(a,b,c,e){var f,i,j,k,l,m=\"function\"==typeof a&&a,n=!e&&g(a=m.selector||a);if(c=c||[],1===n.length){if(i=n[0]=n[0].slice(0),i.length>2&&\"ID\"===(j=i[0]).type&&9===b.nodeType&&p&&d.relative[i[1].type]){if(b=(d.find.ID(j.matches[0].replace(_,aa),b)||[])[0],!b)return c;m&&(b=b.parentNode),a=a.slice(i.shift().value.length)}f=V.needsContext.test(a)?0:i.length;while(f--){if(j=i[f],d.relative[k=j.type])break;if((l=d.find[k])&&(e=l(j.matches[0].replace(_,aa),$.test(i[0].type)&&qa(b.parentNode)||b))){if(i.splice(f,1),a=e.length&&sa(i),!a)return G.apply(c,e),c;break}}}return(m||h(a,n))(e,b,!p,c,!b||$.test(a)&&qa(b.parentNode)||b),c},c.sortStable=u.split(\"\").sort(B).join(\"\")===u,c.detectDuplicates=!!l,m(),c.sortDetached=ja(";
    const char PROGMEM_JS_JQUERY4[] PROGMEM = "function(a){return 1&a.compareDocumentPosition(n.createElement(\"fieldset\"))}),ja(function(a){return a.innerHTML=\"<a href='#'></a>\",\"#\"===a.firstChild.getAttribute(\"href\")})||ka(\"type|href|height|width\",function(a,b,c){if(!c)return a.getAttribute(b,\"type\"===b.toLowerCase()?1:2)}),c.attributes&&ja(function(a){return a.innerHTML=\"<input/>\",a.firstChild.setAttribute(\"value\",\"\"),\"\"===a.firstChild.getAttribute(\"value\")})||ka(\"value\",function(a,b,c){if(!c&&\"input\"===a.nodeName.toLowerCase())return a.defaultValue}),ja(function(a){return null==a.getAttribute(\"disabled\")})||ka(J,function(a,b,c){var d;if(!c)return a[b]===!0?b.toLowerCase():(d=a.getAttributeNode(b))&&d.specified?d.value:null}),ga}(a);r.find=x,r.expr=x.selectors,r.expr[\":\"]=r.expr.pseudos,r.uniqueSort=r.unique=x.uniqueSort,r.text=x.getText,r.isXMLDoc=x.isXML,r.contains=x.contains,r.escapeSelector=x.escape;var y=function(a,b,c){var d=[],e=void 0!==c;while((a=a[b])&&9!==a.nodeType)if(1===a.nodeType){if(e&&r(a).is(c))break;d.push(a)}return d},z=function(a,b){for(var c=[];a;a=a.nextSibling)1===a.nodeType&&a!==b&&c.push(a);return c},A=r.expr.match.needsContext;function B(a,b){return a.nodeName&&a.nodeName.toLowerCase()===b.toLowerCase()}var C=/^<([a-z][^\\/\\0>:\\x20\\t\\r\\n\\f]*)[\\x20\\t\\r\\n\\f]*\\/?>(?:<\\/\\1>|)$/i,D=/^.[^:#\\[\\.,]*$/;function E(a,b,c){return r.isFunction(b)?r.grep(a,function(a,d){return!!b.call(a,d,a)!==c}):b.nodeType?r.grep(a,function(a){return a===b!==c}):\"string\"!=typeof b?r.grep(a,function(a){return i.call(b,a)>-1!==c}):D.test(b)?r.filter(b,a,c):(b=r.filter(b,a),r.grep(a,function(a){return i.call(b,a)>-1!==c&&1===a.nodeType}))}r.filter=function(a,b,c){var d=b[0];return c&&(a=\":not(\"+a+\")\"),1===b.length&&1===d.nodeType?r.find.matchesSelector(d,a)?[d]:[]:r.find.matches(a,r.grep(b,function(a){return 1===a.nodeType}))},r.fn.extend({find:function(a){var b,c,d=this.length,e=this;if(\"string\"!=typeof a)return this.pushStack(r(a).filter(function(){for(b=0;b<d;b++)if(r.contains(e[b],this))return!0}));for(c=this.pushStack([]),b=0;b<d;b++)r.find(a,e[b],c);return d>1?r.uniqueSort(c):c},filter:function(a){return this.pushStack(E(this,a||[],!1))},not:function(a){return this.pushStack(E(this,a||[],!0))},is:function(a){return!!E(this,\"string\"==typeof a&&A.test(a)?r(a):a||[],!1).length}});var F,G=/^(?:\\s*(<[\\w\\W]+>)[^>]*|#([\\w-]+))$/,H=r.fn.init=function(a,b,c){var e,f;if(!a)return this;if(c=c||F,\"string\"==typeof a){if(e=\"<\"===a[0]&&\">\"===a[a.length-1]&&a.length>=3?[null,a,null]:G.exec(a),!e||!e[1]&&b)return!b||b.jquery?(b||c).find(a):this.constructor(b).find(a);if(e[1]){if(b=b instanceof r?b[0]:b,r.merge(this,r.parseHTML(e[1],b&&b.nodeType?b.ownerDocument||b:d,!0)),C.test(e[1])&&r.isPlainObject(b))for(e in b)r.isFunction(this[e])?this[e](b[e]):this.attr(e,b[e]);return this}return f=d.getElementById(e[2]),f&&(this[0]=f,this.length=1),this}return a.nodeType?(this[0]=a,this.length=1,this):r.isFunction(a)?void 0!==c.ready?c.ready(a):a(r):r.makeArray(a,this)};H.prototype=r.fn,F=r(d);var I=/^(?:parents|prev(?:Until|All))/,J={children:!0,contents:!0,next:!0,prev:!0};r.fn.extend({has:function(a){var b=r(a,this),c=b.length;return this.filter(function(){for(var a=0;a<c;a++)if(r.contains(this,b[a]))return!0})},closest:function(a,b){var c,d=0,e=this.length,f=[],g=\"string\"!=typeof a&&r(a);if(!A.test(a))for(;d<e;d++)for(c=this[d];c&&c!==b;c=c.parentNode)if(c.nodeType<11&&(g?g.index(c)>-1:1===c.nodeType&&r.find.matchesSelector(c,a))){f.push(c);break}return this.pushStack(f.length>1?r.uniqueSort(f):f)},index:function(a){return a?\"string\"==typeof a?i.call(r(a),this[0]):i.call(this,a.jquery?a[0]:a):this[0]&&this[0].parentNode?this.first().prevAll().length:-1},add:function(a,b){return this.pushStack(r.uniqueSort(r.merge(this.get(),r(a,b))))},addBack:function(a){return this.add(null==a?this.prevObject:this.prevObject.filter(a))}});function K(a,b){while((a=a[b])&&1!==a.nodeType);return a}r.each({parent:function(a){var b=a.parentNode;return b&&11!==b.nodeType?b:null},parents:function(a){return y(a,\"parentNode\")},parentsUntil:function(a,b,c){return y(a,\"parentNode\",c)},next:function(a){return K(a,\"nextSibling\")},prev:function(a){return K(a,\"previousSibling\")},nextAll:function(a){return y(a,\"nextSibling\")},prevAll:function(a){return y(a,\"previousSibling\")},nextUntil:function(a,b,c){return y(a,\"nextSibling\",c)},prevUntil:function(a,b,c){return y(a,\"previousSibling\",c)},siblings:function(a){return z((a.parentNode||{}).firstChild,a)},children:function(a){return z(a.firstChild)},contents:function(a){return B(a,\"iframe\")?a.contentDocument:(B(a,\"template\")&&(a=a.content||a),r.merge([],a.childNodes))}},function(a,b){r.fn[a]=function(c,d){var e=r.map(this,b,c);return\"Until\"!==a.slice(-5)&&(d=c),d&&\"string\"==typeof d&&(e=r.filter(d,e)),this.length>1&&(J[a]||r.uniqueSort(e),I.test(a)&&e.reverse()),this.pushStack(e)}});var L=/[^\\x20\\t\\r\\n\\f]+/g;function M(a){var b={};return r.each(a.match(L)||[],function(a,c){b[c]=!0}),b}r.Callbacks=function(a){a=\"string\"==typeof a?M(a):r.extend({},a);var b,c,d,e,f=[],g=[],h=-1,i=function(){for(e=e||a.once,d=b=!0;g.length;h=-1){c=g.shift();while(++h<f.length)f[h].apply(c[0],c[1])===!1&&a.stopOnFalse&&(h=f.length,c=!1)}a.memory||(c=!1),b=!1,e&&(f=c?[]:\"\")},j={add:function(){return f&&(c&&!b&&(h=f.length-1,g.push(c)),function d(b){r.each(b,function(b,c){r.isFunction(c)?a.unique&&j.has(c)||f.push(c):c&&c.length&&\"string\"!==r.type(c)&&d(c)})}(arguments),c&&!b&&i()),this},remove:function(){return r.each(arguments,function(a,b){var c;while((c=r.inArray(b,f,c))>-1)f.splice(c,1),c<=h&&h--}),this},has:function(a){return a?r.inArray(a,f)>-1:f.length>0},empty:function(){return f&&(f=[]),this},disable:function(){return e=g=[],f=c=\"\",this},disabled:function(){return!f},lock:function(){return e=g=[],c||b||(f=c=\"\"),this},locked:function(){return!!e},fireWith:function(a,c){return e||(c=c||[],c=[a,c.slice?c.slice():c],g.push(c),b||i()),this},fire:function(){return j.fireWith(this,arguments),this},fired:function(){return!!d}};return j};function N(a){return a}function O(a){throw a}function P(a,b,c,d){var e;try{a&&r.isFunction(e=a.promise)?e.call(a).done(b).fail(c):a&&r.isFunction(e=a.then)?e.call(a,b,c):b.apply(void 0,[a].slice(d))}catch(a){c.apply(void 0,[a])}}r.extend({Deferred:function(b){var c=[[\"notify\",\"progress\",r.Callbacks(\"memory\"),r.Callbacks(\"memory\"),2],[\"resolve\",\"done\",r.Callbacks(\"once memory\"),r.Callbacks(\"once memory\"),0,\"resolved\"],[\"reject\",\"fail\",r.Callbacks(\"once memory\"),r.Callbacks(\"once memory\"),1,\"rejected\"]],d=\"pending\",e={state:function(){return d},always:function(){return f.done(arguments).fail(arguments),this},\"catch\":function(a){return e.then(null,a)},pipe:function(){var a=arguments;return r.Deferred(function(b){r.each(c,function(c,d){var e=r.isFunction(a[d[4]])&&a[d[4]];f[d[1]](function(){var a=e&&e.apply(this,arguments);a&&r.isFunction(a.promise)?a.promise().progress(b.notify).done(b.resolve).fail(b.reject):b[d[0]+\"With\"](this,e?[a]:arguments)})}),a=null}).promise()},then:function(b,d,e){var f=0;function g(b,c,d,e){return function(){var h=this,i=arguments,j=function(){var a,j;if(!(b<f)){if(a=d.apply(h,i),a===c.promise())throw new TypeError(\"Thenable self-resolution\");j=a&&(\"object\"==typeof a||\"function\"==typeof a)&&a.then,r.isFunction(j)?e?j.call(a,g(f,c,N,e),g(f,c,O,e)):(f++,j.call(a,g(f,c,N,e),g(f,c,O,e),g(f,c,N,c.notifyWith))):(d!==N&&(h=void 0,i=[a]),(e||c.resolveWith)(h,i))}},k=e?j:function(){try{j()}catch(a){r.Deferred.exceptionHook&&r.Deferred.exceptionHook(a,k.stackTrace),b+1>=f&&(d!==O&&(h=void 0,i=[a]),c.rejectWith(h,i))}};b?k():(r.Deferred.getStackHook&&(k.stackTrace=r.Deferred.getStackHook()),a.setTimeout(k))}}return r.Deferred(function(a){c[0][3].add(g(0,a,r.isFunction(e)?e:N,a.notifyWith)),c[1][3].add(";
    const char PROGMEM_JS_JQUERY5[] PROGMEM = "g(0,a,r.isFunction(b)?b:N)),c[2][3].add(g(0,a,r.isFunction(d)?d:O))}).promise()},promise:function(a){return null!=a?r.extend(a,e):e}},f={};return r.each(c,function(a,b){var g=b[2],h=b[5];e[b[1]]=g.add,h&&g.add(function(){d=h},c[3-a][2].disable,c[0][2].lock),g.add(b[3].fire),f[b[0]]=function(){return f[b[0]+\"With\"](this===f?void 0:this,arguments),this},f[b[0]+\"With\"]=g.fireWith}),e.promise(f),b&&b.call(f,f),f},when:function(a){var b=arguments.length,c=b,d=Array(c),e=f.call(arguments),g=r.Deferred(),h=function(a){return function(c){d[a]=this,e[a]=arguments.length>1?f.call(arguments):c,--b||g.resolveWith(d,e)}};if(b<=1&&(P(a,g.done(h(c)).resolve,g.reject,!b),\"pending\"===g.state()||r.isFunction(e[c]&&e[c].then)))return g.then();while(c--)P(e[c],h(c),g.reject);return g.promise()}});var Q=/^(Eval|Internal|Range|Reference|Syntax|Type|URI)Error$/;r.Deferred.exceptionHook=function(b,c){a.console&&a.console.warn&&b&&Q.test(b.name)&&a.console.warn(\"jQuery.Deferred exception: \"+b.message,b.stack,c)},r.readyException=function(b){a.setTimeout(function(){throw b})};var R=r.Deferred();r.fn.ready=function(a){return R.then(a)[\"catch\"](function(a){r.readyException(a)}),this},r.extend({isReady:!1,readyWait:1,ready:function(a){(a===!0?--r.readyWait:r.isReady)||(r.isReady=!0,a!==!0&&--r.readyWait>0||R.resolveWith(d,[r]))}}),r.ready.then=R.then;function S(){d.removeEventListener(\"DOMContentLoaded\",S),\na.removeEventListener(\"load\",S),r.ready()}\"complete\"===d.readyState||\"loading\"!==d.readyState&&!d.documentElement.doScroll?a.setTimeout(r.ready):(d.addEventListener(\"DOMContentLoaded\",S),a.addEventListener(\"load\",S));var T=function(a,b,c,d,e,f,g){var h=0,i=a.length,j=null==c;if(\"object\"===r.type(c)){e=!0;for(h in c)T(a,b,h,c[h],!0,f,g)}else if(void 0!==d&&(e=!0,r.isFunction(d)||(g=!0),j&&(g?(b.call(a,d),b=null):(j=b,b=function(a,b,c){return j.call(r(a),c)})),b))for(;h<i;h++)b(a[h],c,g?d:d.call(a[h],h,b(a[h],c)));return e?a:j?b.call(a):i?b(a[0],c):f},U=function(a){return 1===a.nodeType||9===a.nodeType||!+a.nodeType};function V(){this.expando=r.expando+V.uid++}V.uid=1,V.prototype={cache:function(a){var b=a[this.expando];return b||(b={},U(a)&&(a.nodeType?a[this.expando]=b:Object.defineProperty(a,this.expando,{value:b,configurable:!0}))),b},set:function(a,b,c){var d,e=this.cache(a);if(\"string\"==typeof b)e[r.camelCase(b)]=c;else for(d in b)e[r.camelCase(d)]=b[d];return e},get:function(a,b){return void 0===b?this.cache(a):a[this.expando]&&a[this.expando][r.camelCase(b)]},access:function(a,b,c){return void 0===b||b&&\"string\"==typeof b&&void 0===c?this.get(a,b):(this.set(a,b,c),void 0!==c?c:b)},remove:function(a,b){var c,d=a[this.expando];if(void 0!==d){if(void 0!==b){Array.isArray(b)?b=b.map(r.camelCase):(b=r.camelCase(b),b=b in d?[b]:b.match(L)||[]),c=b.length;while(c--)delete d[b[c]]}(void 0===b||r.isEmptyObject(d))&&(a.nodeType?a[this.expando]=void 0:delete a[this.expando])}},hasData:function(a){var b=a[this.expando];return void 0!==b&&!r.isEmptyObject(b)}};var W=new V,X=new V,Y=/^(?:\\{[\\w\\W]*\\}|\\[[\\w\\W]*\\])$/,Z=/[A-Z]/g;function $(a){return\"true\"===a||\"false\"!==a&&(\"null\"===a?null:a===+a+\"\"?+a:Y.test(a)?JSON.parse(a):a)}function _(a,b,c){var d;if(void 0===c&&1===a.nodeType)if(d=\"data-\"+b.replace(Z,\"-$&\").toLowerCase(),c=a.getAttribute(d),\"string\"==typeof c){try{c=$(c)}catch(e){}X.set(a,b,c)}else c=void 0;return c}r.extend({hasData:function(a){return X.hasData(a)||W.hasData(a)},data:function(a,b,c){return X.access(a,b,c)},removeData:function(a,b){X.remove(a,b)},_data:function(a,b,c){return W.access(a,b,c)},_removeData:function(a,b){W.remove(a,b)}}),r.fn.extend({data:function(a,b){var c,d,e,f=this[0],g=f&&f.attributes;if(void 0===a){if(this.length&&(e=X.get(f),1===f.nodeType&&!W.get(f,\"hasDataAttrs\"))){c=g.length;while(c--)g[c]&&(d=g[c].name,0===d.indexOf(\"data-\")&&(d=r.camelCase(d.slice(5)),_(f,d,e[d])));W.set(f,\"hasDataAttrs\",!0)}return e}return\"object\"==typeof a?this.each(function(){X.set(this,a)}):T(this,function(b){var c;if(f&&void 0===b){if(c=X.get(f,a),void 0!==c)return c;if(c=_(f,a),void 0!==c)return c}else this.each(function(){X.set(this,a,b)})},null,b,arguments.length>1,null,!0)},removeData:function(a){return this.each(function(){X.remove(this,a)})}}),r.extend({queue:function(a,b,c){var d;if(a)return b=(b||\"fx\")+\"queue\",d=W.get(a,b),c&&(!d||Array.isArray(c)?d=W.access(a,b,r.makeArray(c)):d.push(c)),d||[]},dequeue:function(a,b){b=b||\"fx\";var c=r.queue(a,b),d=c.length,e=c.shift(),f=r._queueHooks(a,b),g=function(){r.dequeue(a,b)};\"inprogress\"===e&&(e=c.shift(),d--),e&&(\"fx\"===b&&c.unshift(\"inprogress\"),delete f.stop,e.call(a,g,f)),!d&&f&&f.empty.fire()},_queueHooks:function(a,b){var c=b+\"queueHooks\";return W.get(a,c)||W.access(a,c,{empty:r.Callbacks(\"once memory\").add(function(){W.remove(a,[b+\"queue\",c])})})}}),r.fn.extend({queue:function(a,b){var c=2;return\"string\"!=typeof a&&(b=a,a=\"fx\",c--),arguments.length<c?r.queue(this[0],a):void 0===b?this:this.each(function(){var c=r.queue(this,a,b);r._queueHooks(this,a),\"fx\"===a&&\"inprogress\"!==c[0]&&r.dequeue(this,a)})},dequeue:function(a){return this.each(function(){r.dequeue(this,a)})},clearQueue:function(a){return this.queue(a||\"fx\",[])},promise:function(a,b){var c,d=1,e=r.Deferred(),f=this,g=this.length,h=function(){--d||e.resolveWith(f,[f])};\"string\"!=typeof a&&(b=a,a=void 0),a=a||\"fx\";while(g--)c=W.get(f[g],a+\"queueHooks\"),c&&c.empty&&(d++,c.empty.add(h));return h(),e.promise(b)}});var aa=/[+-]?(?:\\d*\\.|)\\d+(?:[eE][+-]?\\d+|)/.source,ba=new RegExp(\"^(?:([+-])=|)(\"+aa+\")([a-z%]*)$\",\"i\"),ca=[\"Top\",\"Right\",\"Bottom\",\"Left\"],da=function(a,b){return a=b||a,\"none\"===a.style.display||\"\"===a.style.display&&r.contains(a.ownerDocument,a)&&\"none\"===r.css(a,\"display\")},ea=function(a,b,c,d){var e,f,g={};for(f in b)g[f]=a.style[f],a.style[f]=b[f];e=c.apply(a,d||[]);for(f in b)a.style[f]=g[f];return e};function fa(a,b,c,d){var e,f=1,g=20,h=d?function(){return d.cur()}:function(){return r.css(a,b,\"\")},i=h(),j=c&&c[3]||(r.cssNumber[b]?\"\":\"px\"),k=(r.cssNumber[b]||\"px\"!==j&&+i)&&ba.exec(r.css(a,b));if(k&&k[3]!==j){j=j||k[3],c=c||[],k=+i||1;do f=f||\".5\",k/=f,r.style(a,b,k+j);while(f!==(f=h()/i)&&1!==f&&--g)}return c&&(k=+k||+i||0,e=c[1]?k+(c[1]+1)*c[2]:+c[2],d&&(d.unit=j,d.start=k,d.end=e)),e}var ga={};function ha(a){var b,c=a.ownerDocument,d=a.nodeName,e=ga[d];return e?e:(b=c.body.appendChild(c.createElement(d)),e=r.css(b,\"display\"),b.parentNode.removeChild(b),\"none\"===e&&(e=\"block\"),ga[d]=e,e)}function ia(a,b){for(var c,d,e=[],f=0,g=a.length;f<g;f++)d=a[f],d.style&&(c=d.style.display,b?(\"none\"===c&&(e[f]=W.get(d,\"display\")||null,e[f]||(d.style.display=\"\")),\"\"===d.style.display&&da(d)&&(e[f]=ha(d))):\"none\"!==c&&(e[f]=\"none\",W.set(d,\"display\",c)));for(f=0;f<g;f++)null!=e[f]&&(a[f].style.display=e[f]);return a}r.fn.extend({show:function(){return ia(this,!0)},hide:function(){return ia(this)},toggle:function(a){return\"boolean\"==typeof a?a?this.show():this.hide():this.each(function(){da(this)?r(this).show():r(this).hide()})}});var ja=/^(?:checkbox|radio)$/i,ka=/<([a-z][^\\/\\0>\\x20\\t\\r\\n\\f]+)/i,la=/^$|\\/(?:java|ecma)script/i,ma={option:[1,\"<select multiple='multiple'>\",\"</select>\"],thead:[1,\"<table>\",\"</table>\"],col:[2,\"<table><colgroup>\",\"</colgroup></table>\"],tr:[2,\"<table><tbody>\",\"</tbody></table>\"],td:[3,\"<table><tbody><tr>\",\"</tr></tbody></table>\"],_default:[0,\"\",\"\"]};ma.optgroup=ma.option,ma.tbody=ma.tfoot=ma.colgroup=ma.caption=ma.thead,ma.th=ma.td;function na(a,b){var c;return c=\"undefined\"!=typeof a.getElementsByTagName?a.getElementsByTagName(b||\"*\"):\"undefined\"!=typeof a.querySelectorAll?a.querySelectorAll(b||\"*\"):[],void 0===b||b&&B(a,b)?r.merge([a],c):c}function oa(a,b){for(var c=0,d=a.length;c<d;c++)W.set(";
    const char PROGMEM_JS_JQUERY6[] PROGMEM = "a[c],\"globalEval\",!b||W.get(b[c],\"globalEval\"))}var pa=/<|&#?\\w+;/;function qa(a,b,c,d,e){for(var f,g,h,i,j,k,l=b.createDocumentFragment(),m=[],n=0,o=a.length;n<o;n++)if(f=a[n],f||0===f)if(\"object\"===r.type(f))r.merge(m,f.nodeType?[f]:f);else if(pa.test(f)){g=g||l.appendChild(b.createElement(\"div\")),h=(ka.exec(f)||[\"\",\"\"])[1].toLowerCase(),i=ma[h]||ma._default,g.innerHTML=i[1]+r.htmlPrefilter(f)+i[2],k=i[0];while(k--)g=g.lastChild;r.merge(m,g.childNodes),g=l.firstChild,g.textContent=\"\"}else m.push(b.createTextNode(f));l.textContent=\"\",n=0;while(f=m[n++])if(d&&r.inArray(f,d)>-1)e&&e.push(f);else if(j=r.contains(f.ownerDocument,f),g=na(l.appendChild(f),\"script\"),j&&oa(g),c){k=0;while(f=g[k++])la.test(f.type||\"\")&&c.push(f)}return l}!function(){var a=d.createDocumentFragment(),b=a.appendChild(d.createElement(\"div\")),c=d.createElement(\"input\");c.setAttribute(\"type\",\"radio\"),c.setAttribute(\"checked\",\"checked\"),c.setAttribute(\"name\",\"t\"),b.appendChild(c),o.checkClone=b.cloneNode(!0).cloneNode(!0).lastChild.checked,b.innerHTML=\"<textarea>x</textarea>\",o.noCloneChecked=!!b.cloneNode(!0).lastChild.defaultValue}();var ra=d.documentElement,sa=/^key/,ta=/^(?:mouse|pointer|contextmenu|drag|drop)|click/,ua=/^([^.]*)(?:\\.(.+)|)/;function va(){return!0}function wa(){return!1}function xa(){try{return d.activeElement}catch(a){}}function ya(a,b,c,d,e,f){var g,h;if(\"object\"==typeof b){\"string\"!=typeof c&&(d=d||c,c=void 0);for(h in b)ya(a,h,c,d,b[h],f);return a}if(null==d&&null==e?(e=c,d=c=void 0):null==e&&(\"string\"==typeof c?(e=d,d=void 0):(e=d,d=c,c=void 0)),e===!1)e=wa;else if(!e)return a;return 1===f&&(g=e,e=function(a){return r().off(a),g.apply(this,arguments)},e.guid=g.guid||(g.guid=r.guid++)),a.each(function(){r.event.add(this,b,e,d,c)})}r.event={global:{},add:function(a,b,c,d,e){var f,g,h,i,j,k,l,m,n,o,p,q=W.get(a);if(q){c.handler&&(f=c,c=f.handler,e=f.selector),e&&r.find.matchesSelector(ra,e),c.guid||(c.guid=r.guid++),(i=q.events)||(i=q.events={}),(g=q.handle)||(g=q.handle=function(b){return\"undefined\"!=typeof r&&r.event.triggered!==b.type?r.event.dispatch.apply(a,arguments):void 0}),b=(b||\"\").match(L)||[\"\"],j=b.length;while(j--)h=ua.exec(b[j])||[],n=p=h[1],o=(h[2]||\"\").split(\".\").sort(),n&&(l=r.event.special[n]||{},n=(e?l.delegateType:l.bindType)||n,l=r.event.special[n]||{},k=r.extend({type:n,origType:p,data:d,handler:c,guid:c.guid,selector:e,needsContext:e&&r.expr.match.needsContext.test(e),namespace:o.join(\".\")},f),(m=i[n])||(m=i[n]=[],m.delegateCount=0,l.setup&&l.setup.call(a,d,o,g)!==!1||a.addEventListener&&a.addEventListener(n,g)),l.add&&(l.add.call(a,k),k.handler.guid||(k.handler.guid=c.guid)),e?m.splice(m.delegateCount++,0,k):m.push(k),r.event.global[n]=!0)}},remove:function(a,b,c,d,e){var f,g,h,i,j,k,l,m,n,o,p,q=W.hasData(a)&&W.get(a);if(q&&(i=q.events)){b=(b||\"\").match(L)||[\"\"],j=b.length;while(j--)if(h=ua.exec(b[j])||[],n=p=h[1],o=(h[2]||\"\").split(\".\").sort(),n){l=r.event.special[n]||{},n=(d?l.delegateType:l.bindType)||n,m=i[n]||[],h=h[2]&&new RegExp(\"(^|\\\\.)\"+o.join(\"\\\\.(?:.*\\\\.|)\")+\"(\\\\.|$)\"),g=f=m.length;while(f--)k=m[f],!e&&p!==k.origType||c&&c.guid!==k.guid||h&&!h.test(k.namespace)||d&&d!==k.selector&&(\"**\"!==d||!k.selector)||(m.splice(f,1),k.selector&&m.delegateCount--,l.remove&&l.remove.call(a,k));g&&!m.length&&(l.teardown&&l.teardown.call(a,o,q.handle)!==!1||r.removeEvent(a,n,q.handle),delete i[n])}else for(n in i)r.event.remove(a,n+b[j],c,d,!0);r.isEmptyObject(i)&&W.remove(a,\"handle events\")}},dispatch:function(a){var b=r.event.fix(a),c,d,e,f,g,h,i=new Array(arguments.length),j=(W.get(this,\"events\")||{})[b.type]||[],k=r.event.special[b.type]||{};for(i[0]=b,c=1;c<arguments.length;c++)i[c]=arguments[c];if(b.delegateTarget=this,!k.preDispatch||k.preDispatch.call(this,b)!==!1){h=r.event.handlers.call(this,b,j),c=0;while((f=h[c++])&&!b.isPropagationStopped()){b.currentTarget=f.elem,d=0;while((g=f.handlers[d++])&&!b.isImmediatePropagationStopped())b.rnamespace&&!b.rnamespace.test(g.namespace)||(b.handleObj=g,b.data=g.data,e=((r.event.special[g.origType]||{}).handle||g.handler).apply(f.elem,i),void 0!==e&&(b.result=e)===!1&&(b.preventDefault(),b.stopPropagation()))}return k.postDispatch&&k.postDispatch.call(this,b),b.result}},handlers:function(a,b){var c,d,e,f,g,h=[],i=b.delegateCount,j=a.target;if(i&&j.nodeType&&!(\"click\"===a.type&&a.button>=1))for(;j!==this;j=j.parentNode||this)if(1===j.nodeType&&(\"click\"!==a.type||j.disabled!==!0)){for(f=[],g={},c=0;c<i;c++)d=b[c],e=d.selector+\" \",void 0===g[e]&&(g[e]=d.needsContext?r(e,this).index(j)>-1:r.find(e,this,null,[j]).length),g[e]&&f.push(d);f.length&&h.push({elem:j,handlers:f})}return j=this,i<b.length&&h.push({elem:j,handlers:b.slice(i)}),h},addProp:function(a,b){Object.defineProperty(r.Event.prototype,a,{enumerable:!0,configurable:!0,get:r.isFunction(b)?function(){if(this.originalEvent)return b(this.originalEvent)}:function(){if(this.originalEvent)return this.originalEvent[a]},set:function(b){Object.defineProperty(this,a,{enumerable:!0,configurable:!0,writable:!0,value:b})}})},fix:function(a){return a[r.expando]?a:new r.Event(a)},special:{load:{noBubble:!0},focus:{trigger:function(){if(this!==xa()&&this.focus)return this.focus(),!1},delegateType:\"focusin\"},blur:{trigger:function(){if(this===xa()&&this.blur)return this.blur(),!1},delegateType:\"focusout\"},click:{trigger:function(){if(\"checkbox\"===this.type&&this.click&&B(this,\"input\"))return this.click(),!1},_default:function(a){return B(a.target,\"a\")}},beforeunload:{postDispatch:function(a){void 0!==a.result&&a.originalEvent&&(a.originalEvent.returnValue=a.result)}}}},r.removeEvent=function(a,b,c){a.removeEventListener&&a.removeEventListener(b,c)},r.Event=function(a,b){return this instanceof r.Event?(a&&a.type?(this.originalEvent=a,this.type=a.type,this.isDefaultPrevented=a.defaultPrevented||void 0===a.defaultPrevented&&a.returnValue===!1?va:wa,this.target=a.target&&3===a.target.nodeType?a.target.parentNode:a.target,this.currentTarget=a.currentTarget,this.relatedTarget=a.relatedTarget):this.type=a,b&&r.extend(this,b),this.timeStamp=a&&a.timeStamp||r.now(),void(this[r.expando]=!0)):new r.Event(a,b)},r.Event.prototype={constructor:r.Event,isDefaultPrevented:wa,isPropagationStopped:wa,isImmediatePropagationStopped:wa,isSimulated:!1,preventDefault:function(){var a=this.originalEvent;this.isDefaultPrevented=va,a&&!this.isSimulated&&a.preventDefault()},stopPropagation:function(){var a=this.originalEvent;this.isPropagationStopped=va,a&&!this.isSimulated&&a.stopPropagation()},stopImmediatePropagation:function(){var a=this.originalEvent;this.isImmediatePropagationStopped=va,a&&!this.isSimulated&&a.stopImmediatePropagation(),this.stopPropagation()}},r.each({altKey:!0,bubbles:!0,cancelable:!0,changedTouches:!0,ctrlKey:!0,detail:!0,eventPhase:!0,metaKey:!0,pageX:!0,pageY:!0,shiftKey:!0,view:!0,\"char\":!0,charCode:!0,key:!0,keyCode:!0,button:!0,buttons:!0,clientX:!0,clientY:!0,offsetX:!0,offsetY:!0,pointerId:!0,pointerType:!0,screenX:!0,screenY:!0,targetTouches:!0,toElement:!0,touches:!0,which:function(a){var b=a.button;return null==a.which&&sa.test(a.type)?null!=a.charCode?a.charCode:a.keyCode:!a.which&&void 0!==b&&ta.test(a.type)?1&b?1:2&b?3:4&b?2:0:a.which}},r.event.addProp),r.each({mouseenter:\"mouseover\",mouseleave:\"mouseout\",pointerenter:\"pointerover\",pointerleave:\"pointerout\"},function(a,b){r.event.special[a]={delegateType:b,bindType:b,handle:function(a){var c,d=this,e=a.relatedTarget,f=a.handleObj;return e&&(e===d||r.contains(d,e))||(a.type=f.origType,c=f.handler.apply(this,arguments),a.type=b),c}}}),r.fn.extend({on:function(a,b,c,d){return ya(this,a,b,c,d)},one:function(a,b,c,d){return ya(this,a,b,c,d,1)},off:function(a,b,c){var d,e;if(a&&a.preventDefault&&a.handleObj)return d=a.handleObj,r(";
    const char PROGMEM_JS_JQUERY7[] PROGMEM = "a.delegateTarget).off(d.namespace?d.origType+\".\"+d.namespace:d.origType,d.selector,d.handler),this;if(\"object\"==typeof a){for(e in a)this.off(e,b,a[e]);return this}return b!==!1&&\"function\"!=typeof b||(c=b,b=void 0),c===!1&&(c=wa),this.each(function(){r.event.remove(this,a,c,b)})}});var za=/<(?!area|br|col|embed|hr|img|input|link|meta|param)(([a-z][^\\/\\0>\\x20\\t\\r\\n\\f]*)[^>]*)\\/>/gi,Aa=/<script|<style|<link/i,Ba=/checked\\s*(?:[^=]|=\\s*.checked.)/i,Ca=/^true\\/(.*)/,Da=/^\\s*<!(?:\\[CDATA\\[|--)|(?:\\]\\]|--)>\\s*$/g;function Ea(a,b){return B(a,\"table\")&&B(11!==b.nodeType?b:b.firstChild,\"tr\")?r(\">tbody\",a)[0]||a:a}function Fa(a){return a.type=(null!==a.getAttribute(\"type\"))+\"/\"+a.type,a}function Ga(a){var b=Ca.exec(a.type);return b?a.type=b[1]:a.removeAttribute(\"type\"),a}function Ha(a,b){var c,d,e,f,g,h,i,j;if(1===b.nodeType){if(W.hasData(a)&&(f=W.access(a),g=W.set(b,f),j=f.events)){delete g.handle,g.events={};for(e in j)for(c=0,d=j[e].length;c<d;c++)r.event.add(b,e,j[e][c])}X.hasData(a)&&(h=X.access(a),i=r.extend({},h),X.set(b,i))}}function Ia(a,b){var c=b.nodeName.toLowerCase();\"input\"===c&&ja.test(a.type)?b.checked=a.checked:\"input\"!==c&&\"textarea\"!==c||(b.defaultValue=a.defaultValue)}function Ja(a,b,c,d){b=g.apply([],b);var e,f,h,i,j,k,l=0,m=a.length,n=m-1,q=b[0],s=r.isFunction(q);if(s||m>1&&\"string\"==typeof q&&!o.checkClone&&Ba.test(q))return a.each(function(e){var f=a.eq(e);s&&(b[0]=q.call(this,e,f.html())),Ja(f,b,c,d)});if(m&&(e=qa(b,a[0].ownerDocument,!1,a,d),f=e.firstChild,1===e.childNodes.length&&(e=f),f||d)){for(h=r.map(na(e,\"script\"),Fa),i=h.length;l<m;l++)j=e,l!==n&&(j=r.clone(j,!0,!0),i&&r.merge(h,na(j,\"script\"))),c.call(a[l],j,l);if(i)for(k=h[h.length-1].ownerDocument,r.map(h,Ga),l=0;l<i;l++)j=h[l],la.test(j.type||\"\")&&!W.access(j,\"globalEval\")&&r.contains(k,j)&&(j.src?r._evalUrl&&r._evalUrl(j.src):p(j.textContent.replace(Da,\"\"),k))}return a}function Ka(a,b,c){for(var d,e=b?r.filter(b,a):a,f=0;null!=(d=e[f]);f++)c||1!==d.nodeType||r.cleanData(na(d)),d.parentNode&&(c&&r.contains(d.ownerDocument,d)&&oa(na(d,\"script\")),d.parentNode.removeChild(d));return a}r.extend({htmlPrefilter:function(a){return a.replace(za,\"<$1></$2>\")},clone:function(a,b,c){var d,e,f,g,h=a.cloneNode(!0),i=r.contains(a.ownerDocument,a);if(!(o.noCloneChecked||1!==a.nodeType&&11!==a.nodeType||r.isXMLDoc(a)))for(g=na(h),f=na(a),d=0,e=f.length;d<e;d++)Ia(f[d],g[d]);if(b)if(c)for(f=f||na(a),g=g||na(h),d=0,e=f.length;d<e;d++)Ha(f[d],g[d]);else Ha(a,h);return g=na(h,\"script\"),g.length>0&&oa(g,!i&&na(a,\"script\")),h},cleanData:function(a){for(var b,c,d,e=r.event.special,f=0;void 0!==(c=a[f]);f++)if(U(c)){if(b=c[W.expando]){if(b.events)for(d in b.events)e[d]?r.event.remove(c,d):r.removeEvent(c,d,b.handle);c[W.expando]=void 0}c[X.expando]&&(c[X.expando]=void 0)}}}),r.fn.extend({detach:function(a){return Ka(this,a,!0)},remove:function(a){return Ka(this,a)},text:function(a){return T(this,function(a){return void 0===a?r.text(this):this.empty().each(function(){1!==this.nodeType&&11!==this.nodeType&&9!==this.nodeType||(this.textContent=a)})},null,a,arguments.length)},append:function(){return Ja(this,arguments,function(a){if(1===this.nodeType||11===this.nodeType||9===this.nodeType){var b=Ea(this,a);b.appendChild(a)}})},prepend:function(){return Ja(this,arguments,function(a){if(1===this.nodeType||11===this.nodeType||9===this.nodeType){var b=Ea(this,a);b.insertBefore(a,b.firstChild)}})},before:function(){return Ja(this,arguments,function(a){this.parentNode&&this.parentNode.insertBefore(a,this)})},after:function(){return Ja(this,arguments,function(a){this.parentNode&&this.parentNode.insertBefore(a,this.nextSibling)})},empty:function(){for(var a,b=0;null!=(a=this[b]);b++)1===a.nodeType&&(r.cleanData(na(a,!1)),a.textContent=\"\");return this},clone:function(a,b){return a=null!=a&&a,b=null==b?a:b,this.map(function(){return r.clone(this,a,b)})},html:function(a){return T(this,function(a){var b=this[0]||{},c=0,d=this.length;if(void 0===a&&1===b.nodeType)return b.innerHTML;if(\"string\"==typeof a&&!Aa.test(a)&&!ma[(ka.exec(a)||[\"\",\"\"])[1].toLowerCase()]){a=r.htmlPrefilter(a);try{for(;c<d;c++)b=this[c]||{},1===b.nodeType&&(r.cleanData(na(b,!1)),b.innerHTML=a);b=0}catch(e){}}b&&this.empty().append(a)},null,a,arguments.length)},replaceWith:function(){var a=[];return Ja(this,arguments,function(b){var c=this.parentNode;r.inArray(this,a)<0&&(r.cleanData(na(this)),c&&c.replaceChild(b,this))},a)}}),r.each({appendTo:\"append\",prependTo:\"prepend\",insertBefore:\"before\",insertAfter:\"after\",replaceAll:\"replaceWith\"},function(a,b){r.fn[a]=function(a){for(var c,d=[],e=r(a),f=e.length-1,g=0;g<=f;g++)c=g===f?this:this.clone(!0),r(e[g])[b](c),h.apply(d,c.get());return this.pushStack(d)}});var La=/^margin/,Ma=new RegExp(\"^(\"+aa+\")(?!px)[a-z%]+$\",\"i\"),Na=function(b){var c=b.ownerDocument.defaultView;return c&&c.opener||(c=a),c.getComputedStyle(b)};!function(){function b(){if(i){i.style.cssText=\"box-sizing:border-box;position:relative;display:block;margin:auto;border:1px;padding:1px;top:1%;width:50%\",i.innerHTML=\"\",ra.appendChild(h);var b=a.getComputedStyle(i);c=\"1%\"!==b.top,g=\"2px\"===b.marginLeft,e=\"4px\"===b.width,i.style.marginRight=\"50%\",f=\"4px\"===b.marginRight,ra.removeChild(h),i=null}}var c,e,f,g,h=d.createElement(\"div\"),i=d.createElement(\"div\");i.style&&(i.style.backgroundClip=\"content-box\",i.cloneNode(!0).style.backgroundClip=\"\",o.clearCloneStyle=\"content-box\"===i.style.backgroundClip,h.style.cssText=\"border:0;width:8px;height:0;top:0;left:-9999px;padding:0;margin-top:1px;position:absolute\",h.appendChild(i),r.extend(o,{pixelPosition:function(){return b(),c},boxSizingReliable:function(){return b(),e},pixelMarginRight:function(){return b(),f},reliableMarginLeft:function(){return b(),g}}))}();function Oa(a,b,c){var d,e,f,g,h=a.style;return c=c||Na(a),c&&(g=c.getPropertyValue(b)||c[b],\"\"!==g||r.contains(a.ownerDocument,a)||(g=r.style(a,b)),!o.pixelMarginRight()&&Ma.test(g)&&La.test(b)&&(d=h.width,e=h.minWidth,f=h.maxWidth,h.minWidth=h.maxWidth=h.width=g,g=c.width,h.width=d,h.minWidth=e,h.maxWidth=f)),void 0!==g?g+\"\":g}function Pa(a,b){return{get:function(){return a()?void delete this.get:(this.get=b).apply(this,arguments)}}}var Qa=/^(none|table(?!-c[ea]).+)/,Ra=/^--/,Sa={position:\"absolute\",visibility:\"hidden\",display:\"block\"},Ta={letterSpacing:\"0\",fontWeight:\"400\"},Ua=[\"Webkit\",\"Moz\",\"ms\"],Va=d.createElement(\"div\").style;function Wa(a){if(a in Va)return a;var b=a[0].toUpperCase()+a.slice(1),c=Ua.length;while(c--)if(a=Ua[c]+b,a in Va)return a}function Xa(a){var b=r.cssProps[a];return b||(b=r.cssProps[a]=Wa(a)||a),b}function Ya(a,b,c){var d=ba.exec(b);return d?Math.max(0,d[2]-(c||0))+(d[3]||\"px\"):b}function Za(a,b,c,d,e){var f,g=0;for(f=c===(d?\"border\":\"content\")?4:\"width\"===b?1:0;f<4;f+=2)\"margin\"===c&&(g+=r.css(a,c+ca[f],!0,e)),d?(\"content\"===c&&(g-=r.css(a,\"padding\"+ca[f],!0,e)),\"margin\"!==c&&(g-=r.css(a,\"border\"+ca[f]+\"Width\",!0,e))):(g+=r.css(a,\"padding\"+ca[f],!0,e),\"padding\"!==c&&(g+=r.css(a,\"border\"+ca[f]+\"Width\",!0,e)));return g}function $a(a,b,c){var d,e=Na(a),f=Oa(a,b,e),g=\"border-box\"===r.css(a,\"boxSizing\",!1,e);return Ma.test(f)?f:(d=g&&(o.boxSizingReliable()||f===a.style[b]),\"auto\"===f&&(f=a[\"offset\"+b[0].toUpperCase()+b.slice(1)]),f=parseFloat(f)||0,f+Za(a,b,c||(g?\"border\":\"content\"),d,e)+\"px\")}r.extend({cssHooks:{opacity:{get:function(a,b){if(b){var c=Oa(a,\"opacity\");return\"\"===c?\"1\":c}}}},cssNumber:{animationIterationCount:!0,columnCount:!0,fillOpacity:!0,flexGrow:!0,flexShrink:!0,fontWeight:!0,lineHeight:!0,opacity:!0,order:!0,orphans:!0,widows:!0,zIndex:!0,zoom:!0},cssProps:{\"float\":\"cssFloat\"},style:function(a,b,c,d){if(a&&3!==a.nodeType&&8!==a.nodeType&&a.style)";
    const char PROGMEM_JS_JQUERY8[] PROGMEM = "{var e,f,g,h=r.camelCase(b),i=Ra.test(b),j=a.style;return i||(b=Xa(h)),g=r.cssHooks[b]||r.cssHooks[h],void 0===c?g&&\"get\"in g&&void 0!==(e=g.get(a,!1,d))?e:j[b]:(f=typeof c,\"string\"===f&&(e=ba.exec(c))&&e[1]&&(c=fa(a,b,e),f=\"number\"),null!=c&&c===c&&(\"number\"===f&&(c+=e&&e[3]||(r.cssNumber[h]?\"\":\"px\")),o.clearCloneStyle||\"\"!==c||0!==b.indexOf(\"background\")||(j[b]=\"inherit\"),g&&\"set\"in g&&void 0===(c=g.set(a,c,d))||(i?j.setProperty(b,c):j[b]=c)),void 0)}},css:function(a,b,c,d){var e,f,g,h=r.camelCase(b),i=Ra.test(b);return i||(b=Xa(h)),g=r.cssHooks[b]||r.cssHooks[h],g&&\"get\"in g&&(e=g.get(a,!0,c)),void 0===e&&(e=Oa(a,b,d)),\"normal\"===e&&b in Ta&&(e=Ta[b]),\"\"===c||c?(f=parseFloat(e),c===!0||isFinite(f)?f||0:e):e}}),r.each([\"height\",\"width\"],function(a,b){r.cssHooks[b]={get:function(a,c,d){if(c)return!Qa.test(r.css(a,\"display\"))||a.getClientRects().length&&a.getBoundingClientRect().width?$a(a,b,d):ea(a,Sa,function(){return $a(a,b,d)})},set:function(a,c,d){var e,f=d&&Na(a),g=d&&Za(a,b,d,\"border-box\"===r.css(a,\"boxSizing\",!1,f),f);return g&&(e=ba.exec(c))&&\"px\"!==(e[3]||\"px\")&&(a.style[b]=c,c=r.css(a,b)),Ya(a,c,g)}}}),r.cssHooks.marginLeft=Pa(o.reliableMarginLeft,function(a,b){if(b)return(parseFloat(Oa(a,\"marginLeft\"))||a.getBoundingClientRect().left-ea(a,{marginLeft:0},function(){return a.getBoundingClientRect().left}))+\"px\"}),r.each({margin:\"\",padding:\"\",border:\"Width\"},function(a,b){r.cssHooks[a+b]={expand:function(c){for(var d=0,e={},f=\"string\"==typeof c?c.split(\" \"):[c];d<4;d++)e[a+ca[d]+b]=f[d]||f[d-2]||f[0];return e}},La.test(a)||(r.cssHooks[a+b].set=Ya)}),r.fn.extend({css:function(a,b){return T(this,function(a,b,c){var d,e,f={},g=0;if(Array.isArray(b)){for(d=Na(a),e=b.length;g<e;g++)f[b[g]]=r.css(a,b[g],!1,d);return f}return void 0!==c?r.style(a,b,c):r.css(a,b)},a,b,arguments.length>1)}});function _a(a,b,c,d,e){return new _a.prototype.init(a,b,c,d,e)}r.Tween=_a,_a.prototype={constructor:_a,init:function(a,b,c,d,e,f){this.elem=a,this.prop=c,this.easing=e||r.easing._default,this.options=b,this.start=this.now=this.cur(),this.end=d,this.unit=f||(r.cssNumber[c]?\"\":\"px\")},cur:function(){var a=_a.propHooks[this.prop];return a&&a.get?a.get(this):_a.propHooks._default.get(this)},run:function(a){var b,c=_a.propHooks[this.prop];return this.options.duration?this.pos=b=r.easing[this.easing](a,this.options.duration*a,0,1,this.options.duration):this.pos=b=a,this.now=(this.end-this.start)*b+this.start,this.options.step&&this.options.step.call(this.elem,this.now,this),c&&c.set?c.set(this):_a.propHooks._default.set(this),this}},_a.prototype.init.prototype=_a.prototype,_a.propHooks={_default:{get:function(a){var b;return 1!==a.elem.nodeType||null!=a.elem[a.prop]&&null==a.elem.style[a.prop]?a.elem[a.prop]:(b=r.css(a.elem,a.prop,\"\"),b&&\"auto\"!==b?b:0)},set:function(a){r.fx.step[a.prop]?r.fx.step[a.prop](a):1!==a.elem.nodeType||null==a.elem.style[r.cssProps[a.prop]]&&!r.cssHooks[a.prop]?a.elem[a.prop]=a.now:r.style(a.elem,a.prop,a.now+a.unit)}}},_a.propHooks.scrollTop=_a.propHooks.scrollLeft={set:function(a){a.elem.nodeType&&a.elem.parentNode&&(a.elem[a.prop]=a.now)}},r.easing={linear:function(a){return a},swing:function(a){return.5-Math.cos(a*Math.PI)/2},_default:\"swing\"},r.fx=_a.prototype.init,r.fx.step={};var ab,bb,cb=/^(?:toggle|show|hide)$/,db=/queueHooks$/;function eb(){bb&&(d.hidden===!1&&a.requestAnimationFrame?a.requestAnimationFrame(eb):a.setTimeout(eb,r.fx.interval),r.fx.tick())}function fb(){return a.setTimeout(function(){ab=void 0}),ab=r.now()}function gb(a,b){var c,d=0,e={height:a};for(b=b?1:0;d<4;d+=2-b)c=ca[d],e[\"margin\"+c]=e[\"padding\"+c]=a;return b&&(e.opacity=e.width=a),e}function hb(a,b,c){for(var d,e=(kb.tweeners[b]||[]).concat(kb.tweeners[\"*\"]),f=0,g=e.length;f<g;f++)if(d=e[f].call(c,b,a))return d}function ib(a,b,c){var d,e,f,g,h,i,j,k,l=\"width\"in b||\"height\"in b,m=this,n={},o=a.style,p=a.nodeType&&da(a),q=W.get(a,\"fxshow\");c.queue||(g=r._queueHooks(a,\"fx\"),null==g.unqueued&&(g.unqueued=0,h=g.empty.fire,g.empty.fire=function(){g.unqueued||h()}),g.unqueued++,m.always(function(){m.always(function(){g.unqueued--,r.queue(a,\"fx\").length||g.empty.fire()})}));for(d in b)if(e=b[d],cb.test(e)){if(delete b[d],f=f||\"toggle\"===e,e===(p?\"hide\":\"show\")){if(\"show\"!==e||!q||void 0===q[d])continue;p=!0}n[d]=q&&q[d]||r.style(a,d)}if(i=!r.isEmptyObject(b),i||!r.isEmptyObject(n)){l&&1===a.nodeType&&(c.overflow=[o.overflow,o.overflowX,o.overflowY],j=q&&q.display,null==j&&(j=W.get(a,\"display\")),k=r.css(a,\"display\"),\"none\"===k&&(j?k=j:(ia([a],!0),j=a.style.display||j,k=r.css(a,\"display\"),ia([a]))),(\"inline\"===k||\"inline-block\"===k&&null!=j)&&\"none\"===r.css(a,\"float\")&&(i||(m.done(function(){o.display=j}),null==j&&(k=o.display,j=\"none\"===k?\"\":k)),o.display=\"inline-block\")),c.overflow&&(o.overflow=\"hidden\",m.always(function(){o.overflow=c.overflow[0],o.overflowX=c.overflow[1],o.overflowY=c.overflow[2]})),i=!1;for(d in n)i||(q?\"hidden\"in q&&(p=q.hidden):q=W.access(a,\"fxshow\",{display:j}),f&&(q.hidden=!p),p&&ia([a],!0),m.done(function(){p||ia([a]),W.remove(a,\"fxshow\");for(d in n)r.style(a,d,n[d])})),i=hb(p?q[d]:0,d,m),d in q||(q[d]=i.start,p&&(i.end=i.start,i.start=0))}}function jb(a,b){var c,d,e,f,g;for(c in a)if(d=r.camelCase(c),e=b[d],f=a[c],Array.isArray(f)&&(e=f[1],f=a[c]=f[0]),c!==d&&(a[d]=f,delete a[c]),g=r.cssHooks[d],g&&\"expand\"in g){f=g.expand(f),delete a[d];for(c in f)c in a||(a[c]=f[c],b[c]=e)}else b[d]=e}function kb(a,b,c){var d,e,f=0,g=kb.prefilters.length,h=r.Deferred().always(function(){delete i.elem}),i=function(){if(e)return!1;for(var b=ab||fb(),c=Math.max(0,j.startTime+j.duration-b),d=c/j.duration||0,f=1-d,g=0,i=j.tweens.length;g<i;g++)j.tweens[g].run(f);return h.notifyWith(a,[j,f,c]),f<1&&i?c:(i||h.notifyWith(a,[j,1,0]),h.resolveWith(a,[j]),!1)},j=h.promise({elem:a,props:r.extend({},b),opts:r.extend(!0,{specialEasing:{},easing:r.easing._default},c),originalProperties:b,originalOptions:c,startTime:ab||fb(),duration:c.duration,tweens:[],createTween:function(b,c){var d=r.Tween(a,j.opts,b,c,j.opts.specialEasing[b]||j.opts.easing);return j.tweens.push(d),d},stop:function(b){var c=0,d=b?j.tweens.length:0;if(e)return this;for(e=!0;c<d;c++)j.tweens[c].run(1);return b?(h.notifyWith(a,[j,1,0]),h.resolveWith(a,[j,b])):h.rejectWith(a,[j,b]),this}}),k=j.props;for(jb(k,j.opts.specialEasing);f<g;f++)if(d=kb.prefilters[f].call(j,a,k,j.opts))return r.isFunction(d.stop)&&(r._queueHooks(j.elem,j.opts.queue).stop=r.proxy(d.stop,d)),d;return r.map(k,hb,j),r.isFunction(j.opts.start)&&j.opts.start.call(a,j),j.progress(j.opts.progress).done(j.opts.done,j.opts.complete).fail(j.opts.fail).always(j.opts.always),r.fx.timer(r.extend(i,{elem:a,anim:j,queue:j.opts.queue})),j}r.Animation=r.extend(kb,{tweeners:{\"*\":[function(a,b){var c=this.createTween(a,b);return fa(c.elem,a,ba.exec(b),c),c}]},tweener:function(a,b){r.isFunction(a)?(b=a,a=[\"*\"]):a=a.match(L);for(var c,d=0,e=a.length;d<e;d++)c=a[d],kb.tweeners[c]=kb.tweeners[c]||[],kb.tweeners[c].unshift(b)},prefilters:[ib],prefilter:function(a,b){b?kb.prefilters.unshift(a):kb.prefilters.push(a)}}),r.speed=function(a,b,c){var d=a&&\"object\"==typeof a?r.extend({},a):{complete:c||!c&&b||r.isFunction(a)&&a,duration:a,easing:c&&b||b&&!r.isFunction(b)&&b};return r.fx.off?d.duration=0:\"number\"!=typeof d.duration&&(d.duration in r.fx.speeds?d.duration=r.fx.speeds[d.duration]:d.duration=r.fx.speeds._default),null!=d.queue&&d.queue!==!0||(d.queue=\"fx\"),d.old=d.complete,d.complete=function(){r.isFunction(d.old)&&d.old.call(this),d.queue&&r.dequeue(this,d.queue)},d},r.fn.extend({fadeTo:function(a,b,c,d){return this.filter(da).css(\"opacity\",0).show().end().animate({opacity:b},a,c,d)},animate:function(a,b,c,d){var e=r.isEmptyObject(a),f=r.speed(b,c,d),";
    const char PROGMEM_JS_JQUERY9[] PROGMEM = "g=function(){var b=kb(this,r.extend({},a),f);(e||W.get(this,\"finish\"))&&b.stop(!0)};return g.finish=g,e||f.queue===!1?this.each(g):this.queue(f.queue,g)},stop:function(a,b,c){var d=function(a){var b=a.stop;delete a.stop,b(c)};return\"string\"!=typeof a&&(c=b,b=a,a=void 0),b&&a!==!1&&this.queue(a||\"fx\",[]),this.each(function(){var b=!0,e=null!=a&&a+\"queueHooks\",f=r.timers,g=W.get(this);if(e)g[e]&&g[e].stop&&d(g[e]);else for(e in g)g[e]&&g[e].stop&&db.test(e)&&d(g[e]);for(e=f.length;e--;)f[e].elem!==this||null!=a&&f[e].queue!==a||(f[e].anim.stop(c),b=!1,f.splice(e,1));!b&&c||r.dequeue(this,a)})},finish:function(a){return a!==!1&&(a=a||\"fx\"),this.each(function(){var b,c=W.get(this),d=c[a+\"queue\"],e=c[a+\"queueHooks\"],f=r.timers,g=d?d.length:0;for(c.finish=!0,r.queue(this,a,[]),e&&e.stop&&e.stop.call(this,!0),b=f.length;b--;)f[b].elem===this&&f[b].queue===a&&(f[b].anim.stop(!0),f.splice(b,1));for(b=0;b<g;b++)d[b]&&d[b].finish&&d[b].finish.call(this);delete c.finish})}}),r.each([\"toggle\",\"show\",\"hide\"],function(a,b){var c=r.fn[b];r.fn[b]=function(a,d,e){return null==a||\"boolean\"==typeof a?c.apply(this,arguments):this.animate(gb(b,!0),a,d,e)}}),r.each({slideDown:gb(\"show\"),slideUp:gb(\"hide\"),slideToggle:gb(\"toggle\"),fadeIn:{opacity:\"show\"},fadeOut:{opacity:\"hide\"},fadeToggle:{opacity:\"toggle\"}},function(a,b){r.fn[a]=function(a,c,d){return this.animate(b,a,c,d)}}),r.timers=[],r.fx.tick=function(){var a,b=0,c=r.timers;for(ab=r.now();b<c.length;b++)a=c[b],a()||c[b]!==a||c.splice(b--,1);c.length||r.fx.stop(),ab=void 0},r.fx.timer=function(a){r.timers.push(a),r.fx.start()},r.fx.interval=13,r.fx.start=function(){bb||(bb=!0,eb())},r.fx.stop=function(){bb=null},r.fx.speeds={slow:600,fast:200,_default:400},r.fn.delay=function(b,c){return b=r.fx?r.fx.speeds[b]||b:b,c=c||\"fx\",this.queue(c,function(c,d){var e=a.setTimeout(c,b);d.stop=function(){a.clearTimeout(e)}})},function(){var a=d.createElement(\"input\"),b=d.createElement(\"select\"),c=b.appendChild(d.createElement(\"option\"));a.type=\"checkbox\",o.checkOn=\"\"!==a.value,o.optSelected=c.selected,a=d.createElement(\"input\"),a.value=\"t\",a.type=\"radio\",o.radioValue=\"t\"===a.value}();var lb,mb=r.expr.attrHandle;r.fn.extend({attr:function(a,b){return T(this,r.attr,a,b,arguments.length>1)},removeAttr:function(a){return this.each(function(){r.removeAttr(this,a)})}}),r.extend({attr:function(a,b,c){var d,e,f=a.nodeType;if(3!==f&&8!==f&&2!==f)return\"undefined\"==typeof a.getAttribute?r.prop(a,b,c):(1===f&&r.isXMLDoc(a)||(e=r.attrHooks[b.toLowerCase()]||(r.expr.match.bool.test(b)?lb:void 0)),void 0!==c?null===c?void r.removeAttr(a,b):e&&\"set\"in e&&void 0!==(d=e.set(a,c,b))?d:(a.setAttribute(b,c+\"\"),c):e&&\"get\"in e&&null!==(d=e.get(a,b))?d:(d=r.find.attr(a,b),\nnull==d?void 0:d))},attrHooks:{type:{set:function(a,b){if(!o.radioValue&&\"radio\"===b&&B(a,\"input\")){var c=a.value;return a.setAttribute(\"type\",b),c&&(a.value=c),b}}}},removeAttr:function(a,b){var c,d=0,e=b&&b.match(L);if(e&&1===a.nodeType)while(c=e[d++])a.removeAttribute(c)}}),lb={set:function(a,b,c){return b===!1?r.removeAttr(a,c):a.setAttribute(c,c),c}},r.each(r.expr.match.bool.source.match(/\\w+/g),function(a,b){var c=mb[b]||r.find.attr;mb[b]=function(a,b,d){var e,f,g=b.toLowerCase();return d||(f=mb[g],mb[g]=e,e=null!=c(a,b,d)?g:null,mb[g]=f),e}});var nb=/^(?:input|select|textarea|button)$/i,ob=/^(?:a|area)$/i;r.fn.extend({prop:function(a,b){return T(this,r.prop,a,b,arguments.length>1)},removeProp:function(a){return this.each(function(){delete this[r.propFix[a]||a]})}}),r.extend({prop:function(a,b,c){var d,e,f=a.nodeType;if(3!==f&&8!==f&&2!==f)return 1===f&&r.isXMLDoc(a)||(b=r.propFix[b]||b,e=r.propHooks[b]),void 0!==c?e&&\"set\"in e&&void 0!==(d=e.set(a,c,b))?d:a[b]=c:e&&\"get\"in e&&null!==(d=e.get(a,b))?d:a[b]},propHooks:{tabIndex:{get:function(a){var b=r.find.attr(a,\"tabindex\");return b?parseInt(b,10):nb.test(a.nodeName)||ob.test(a.nodeName)&&a.href?0:-1}}},propFix:{\"for\":\"htmlFor\",\"class\":\"className\"}}),o.optSelected||(r.propHooks.selected={get:function(a){var b=a.parentNode;return b&&b.parentNode&&b.parentNode.selectedIndex,null},set:function(a){var b=a.parentNode;b&&(b.selectedIndex,b.parentNode&&b.parentNode.selectedIndex)}}),r.each([\"tabIndex\",\"readOnly\",\"maxLength\",\"cellSpacing\",\"cellPadding\",\"rowSpan\",\"colSpan\",\"useMap\",\"frameBorder\",\"contentEditable\"],function(){r.propFix[this.toLowerCase()]=this});function pb(a){var b=a.match(L)||[];return b.join(\" \")}function qb(a){return a.getAttribute&&a.getAttribute(\"class\")||\"\"}r.fn.extend({addClass:function(a){var b,c,d,e,f,g,h,i=0;if(r.isFunction(a))return this.each(function(b){r(this).addClass(a.call(this,b,qb(this)))});if(\"string\"==typeof a&&a){b=a.match(L)||[];while(c=this[i++])if(e=qb(c),d=1===c.nodeType&&\" \"+pb(e)+\" \"){g=0;while(f=b[g++])d.indexOf(\" \"+f+\" \")<0&&(d+=f+\" \");h=pb(d),e!==h&&c.setAttribute(\"class\",h)}}return this},removeClass:function(a){var b,c,d,e,f,g,h,i=0;if(r.isFunction(a))return this.each(function(b){r(this).removeClass(a.call(this,b,qb(this)))});if(!arguments.length)return this.attr(\"class\",\"\");if(\"string\"==typeof a&&a){b=a.match(L)||[];while(c=this[i++])if(e=qb(c),d=1===c.nodeType&&\" \"+pb(e)+\" \"){g=0;while(f=b[g++])while(d.indexOf(\" \"+f+\" \")>-1)d=d.replace(\" \"+f+\" \",\" \");h=pb(d),e!==h&&c.setAttribute(\"class\",h)}}return this},toggleClass:function(a,b){var c=typeof a;return\"boolean\"==typeof b&&\"string\"===c?b?this.addClass(a):this.removeClass(a):r.isFunction(a)?this.each(function(c){r(this).toggleClass(a.call(this,c,qb(this),b),b)}):this.each(function(){var b,d,e,f;if(\"string\"===c){d=0,e=r(this),f=a.match(L)||[];while(b=f[d++])e.hasClass(b)?e.removeClass(b):e.addClass(b)}else void 0!==a&&\"boolean\"!==c||(b=qb(this),b&&W.set(this,\"__className__\",b),this.setAttribute&&this.setAttribute(\"class\",b||a===!1?\"\":W.get(this,\"__className__\")||\"\"))})},hasClass:function(a){var b,c,d=0;b=\" \"+a+\" \";while(c=this[d++])if(1===c.nodeType&&(\" \"+pb(qb(c))+\" \").indexOf(b)>-1)return!0;return!1}});var rb=/\\r/g;r.fn.extend({val:function(a){var b,c,d,e=this[0];{if(arguments.length)return d=r.isFunction(a),this.each(function(c){var e;1===this.nodeType&&(e=d?a.call(this,c,r(this).val()):a,null==e?e=\"\":\"number\"==typeof e?e+=\"\":Array.isArray(e)&&(e=r.map(e,function(a){return null==a?\"\":a+\"\"})),b=r.valHooks[this.type]||r.valHooks[this.nodeName.toLowerCase()],b&&\"set\"in b&&void 0!==b.set(this,e,\"value\")||(this.value=e))});if(e)return b=r.valHooks[e.type]||r.valHooks[e.nodeName.toLowerCase()],b&&\"get\"in b&&void 0!==(c=b.get(e,\"value\"))?c:(c=e.value,\"string\"==typeof c?c.replace(rb,\"\"):null==c?\"\":c)}}}),r.extend({valHooks:{option:{get:function(a){var b=r.find.attr(a,\"value\");return null!=b?b:pb(r.text(a))}},select:{get:function(a){var b,c,d,e=a.options,f=a.selectedIndex,g=\"select-one\"===a.type,h=g?null:[],i=g?f+1:e.length;for(d=f<0?i:g?f:0;d<i;d++)if(c=e[d],(c.selected||d===f)&&!c.disabled&&(!c.parentNode.disabled||!B(c.parentNode,\"optgroup\"))){if(b=r(c).val(),g)return b;h.push(b)}return h},set:function(a,b){var c,d,e=a.options,f=r.makeArray(b),g=e.length;while(g--)d=e[g],(d.selected=r.inArray(r.valHooks.option.get(d),f)>-1)&&(c=!0);return c||(a.selectedIndex=-1),f}}}}),r.each([\"radio\",\"checkbox\"],function(){r.valHooks[this]={set:function(a,b){if(Array.isArray(b))return a.checked=r.inArray(r(a).val(),b)>-1}},o.checkOn||(r.valHooks[this].get=function(a){return null===a.getAttribute(\"value\")?\"on\":a.value})});var sb=/^(?:focusinfocus|focusoutblur)$/;r.extend(r.event,{trigger:function(b,c,e,f){var g,h,i,j,k,m,n,o=[e||d],p=l.call(b,\"type\")?b.type:b,q=l.call(b,\"namespace\")?b.namespace.split(\".\"):[];if(h=i=e=e||d,3!==e.nodeType&&8!==e.nodeType&&!sb.test(p+r.event.triggered)";
    const char PROGMEM_JS_JQUERY10[] PROGMEM = "&&(p.indexOf(\".\")>-1&&(q=p.split(\".\"),p=q.shift(),q.sort()),k=p.indexOf(\":\")<0&&\"on\"+p,b=b[r.expando]?b:new r.Event(p,\"object\"==typeof b&&b),b.isTrigger=f?2:3,b.namespace=q.join(\".\"),b.rnamespace=b.namespace?new RegExp(\"(^|\\\\.)\"+q.join(\"\\\\.(?:.*\\\\.|)\")+\"(\\\\.|$)\"):null,b.result=void 0,b.target||(b.target=e),c=null==c?[b]:r.makeArray(c,[b]),n=r.event.special[p]||{},f||!n.trigger||n.trigger.apply(e,c)!==!1)){if(!f&&!n.noBubble&&!r.isWindow(e)){for(j=n.delegateType||p,sb.test(j+p)||(h=h.parentNode);h;h=h.parentNode)o.push(h),i=h;i===(e.ownerDocument||d)&&o.push(i.defaultView||i.parentWindow||a)}g=0;while((h=o[g++])&&!b.isPropagationStopped())b.type=g>1?j:n.bindType||p,m=(W.get(h,\"events\")||{})[b.type]&&W.get(h,\"handle\"),m&&m.apply(h,c),m=k&&h[k],m&&m.apply&&U(h)&&(b.result=m.apply(h,c),b.result===!1&&b.preventDefault());return b.type=p,f||b.isDefaultPrevented()||n._default&&n._default.apply(o.pop(),c)!==!1||!U(e)||k&&r.isFunction(e[p])&&!r.isWindow(e)&&(i=e[k],i&&(e[k]=null),r.event.triggered=p,e[p](),r.event.triggered=void 0,i&&(e[k]=i)),b.result}},simulate:function(a,b,c){var d=r.extend(new r.Event,c,{type:a,isSimulated:!0});r.event.trigger(d,null,b)}}),r.fn.extend({trigger:function(a,b){return this.each(function(){r.event.trigger(a,b,this)})},triggerHandler:function(a,b){var c=this[0];if(c)return r.event.trigger(a,b,c,!0)}}),r.each(\"blur focus focusin focusout resize scroll click dblclick mousedown mouseup mousemove mouseover mouseout mouseenter mouseleave change select submit keydown keypress keyup contextmenu\".split(\" \"),function(a,b){r.fn[b]=function(a,c){return arguments.length>0?this.on(b,null,a,c):this.trigger(b)}}),r.fn.extend({hover:function(a,b){return this.mouseenter(a).mouseleave(b||a)}}),o.focusin=\"onfocusin\"in a,o.focusin||r.each({focus:\"focusin\",blur:\"focusout\"},function(a,b){var c=function(a){r.event.simulate(b,a.target,r.event.fix(a))};r.event.special[b]={setup:function(){var d=this.ownerDocument||this,e=W.access(d,b);e||d.addEventListener(a,c,!0),W.access(d,b,(e||0)+1)},teardown:function(){var d=this.ownerDocument||this,e=W.access(d,b)-1;e?W.access(d,b,e):(d.removeEventListener(a,c,!0),W.remove(d,b))}}});var tb=a.location,ub=r.now(),vb=/\\?/;r.parseXML=function(b){var c;if(!b||\"string\"!=typeof b)return null;try{c=(new a.DOMParser).parseFromString(b,\"text/xml\")}catch(d){c=void 0}return c&&!c.getElementsByTagName(\"parsererror\").length||r.error(\"Invalid XML: \"+b),c};var wb=/\\[\\]$/,xb=/\\r?\\n/g,yb=/^(?:submit|button|image|reset|file)$/i,zb=/^(?:input|select|textarea|keygen)/i;function Ab(a,b,c,d){var e;if(Array.isArray(b))r.each(b,function(b,e){c||wb.test(a)?d(a,e):Ab(a+\"[\"+(\"object\"==typeof e&&null!=e?b:\"\")+\"]\",e,c,d)});else if(c||\"object\"!==r.type(b))d(a,b);else for(e in b)Ab(a+\"[\"+e+\"]\",b[e],c,d)}r.param=function(a,b){var c,d=[],e=function(a,b){var c=r.isFunction(b)?b():b;d[d.length]=encodeURIComponent(a)+\"=\"+encodeURIComponent(null==c?\"\":c)};if(Array.isArray(a)||a.jquery&&!r.isPlainObject(a))r.each(a,function(){e(this.name,this.value)});else for(c in a)Ab(c,a[c],b,e);return d.join(\"&\")},r.fn.extend({serialize:function(){return r.param(this.serializeArray())},serializeArray:function(){return this.map(function(){var a=r.prop(this,\"elements\");return a?r.makeArray(a):this}).filter(function(){var a=this.type;return this.name&&!r(this).is(\":disabled\")&&zb.test(this.nodeName)&&!yb.test(a)&&(this.checked||!ja.test(a))}).map(function(a,b){var c=r(this).val();return null==c?null:Array.isArray(c)?r.map(c,function(a){return{name:b.name,value:a.replace(xb,\"\\r\\n\")}}):{name:b.name,value:c.replace(xb,\"\\r\\n\")}}).get()}});var Bb=/%20/g,Cb=/#.*$/,Db=/([?&])_=[^&]*/,Eb=/^(.*?):[ \\t]*([^\\r\\n]*)$/gm,Fb=/^(?:about|app|app-storage|.+-extension|file|res|widget):$/,Gb=/^(?:GET|HEAD)$/,Hb=/^\\/\\//,Ib={},Jb={},Kb=\"*/\".concat(\"*\"),Lb=d.createElement(\"a\");Lb.href=tb.href;function Mb(a){return function(b,c){\"string\"!=typeof b&&(c=b,b=\"*\");var d,e=0,f=b.toLowerCase().match(L)||[];if(r.isFunction(c))while(d=f[e++])\"+\"===d[0]?(d=d.slice(1)||\"*\",(a[d]=a[d]||[]).unshift(c)):(a[d]=a[d]||[]).push(c)}}function Nb(a,b,c,d){var e={},f=a===Jb;function g(h){var i;return e[h]=!0,r.each(a[h]||[],function(a,h){var j=h(b,c,d);return\"string\"!=typeof j||f||e[j]?f?!(i=j):void 0:(b.dataTypes.unshift(j),g(j),!1)}),i}return g(b.dataTypes[0])||!e[\"*\"]&&g(\"*\")}function Ob(a,b){var c,d,e=r.ajaxSettings.flatOptions||{};for(c in b)void 0!==b[c]&&((e[c]?a:d||(d={}))[c]=b[c]);return d&&r.extend(!0,a,d),a}function Pb(a,b,c){var d,e,f,g,h=a.contents,i=a.dataTypes;while(\"*\"===i[0])i.shift(),void 0===d&&(d=a.mimeType||b.getResponseHeader(\"Content-Type\"));if(d)for(e in h)if(h[e]&&h[e].test(d)){i.unshift(e);break}if(i[0]in c)f=i[0];else{for(e in c){if(!i[0]||a.converters[e+\" \"+i[0]]){f=e;break}g||(g=e)}f=f||g}if(f)return f!==i[0]&&i.unshift(f),c[f]}function Qb(a,b,c,d){var e,f,g,h,i,j={},k=a.dataTypes.slice();if(k[1])for(g in a.converters)j[g.toLowerCase()]=a.converters[g];f=k.shift();while(f)if(a.responseFields[f]&&(c[a.responseFields[f]]=b),!i&&d&&a.dataFilter&&(b=a.dataFilter(b,a.dataType)),i=f,f=k.shift())if(\"*\"===f)f=i;else if(\"*\"!==i&&i!==f){if(g=j[i+\" \"+f]||j[\"* \"+f],!g)for(e in j)if(h=e.split(\" \"),h[1]===f&&(g=j[i+\" \"+h[0]]||j[\"* \"+h[0]])){g===!0?g=j[e]:j[e]!==!0&&(f=h[0],k.unshift(h[1]));break}if(g!==!0)if(g&&a[\"throws\"])b=g(b);else try{b=g(b)}catch(l){return{state:\"parsererror\",error:g?l:\"No conversion from \"+i+\" to \"+f}}}return{state:\"success\",data:b}}r.extend({active:0,lastModified:{},etag:{},ajaxSettings:{url:tb.href,type:\"GET\",isLocal:Fb.test(tb.protocol),global:!0,processData:!0,async:!0,contentType:\"application/x-www-form-urlencoded; charset=UTF-8\",accepts:{\"*\":Kb,text:\"text/plain\",html:\"text/html\",xml:\"application/xml, text/xml\",json:\"application/json, text/javascript\"},contents:{xml:/\\bxml\\b/,html:/\\bhtml/,json:/\\bjson\\b/},responseFields:{xml:\"responseXML\",text:\"responseText\",json:\"responseJSON\"},converters:{\"* text\":String,\"text html\":!0,\"text json\":JSON.parse,\"text xml\":r.parseXML},flatOptions:{url:!0,context:!0}},ajaxSetup:function(a,b){return b?Ob(Ob(a,r.ajaxSettings),b):Ob(r.ajaxSettings,a)},ajaxPrefilter:Mb(Ib),ajaxTransport:Mb(Jb),ajax:function(b,c){\"object\"==typeof b&&(c=b,b=void 0),c=c||{};var e,f,g,h,i,j,k,l,m,n,o=r.ajaxSetup({},c),p=o.context||o,q=o.context&&(p.nodeType||p.jquery)?r(p):r.event,s=r.Deferred(),t=r.Callbacks(\"once memory\"),u=o.statusCode||{},v={},w={},x=\"canceled\",y={readyState:0,getResponseHeader:function(a){var b;if(k){if(!h){h={};while(b=Eb.exec(g))h[b[1].toLowerCase()]=b[2]}b=h[a.toLowerCase()]}return null==b?null:b},getAllResponseHeaders:function(){return k?g:null},setRequestHeader:function(a,b){return null==k&&(a=w[a.toLowerCase()]=w[a.toLowerCase()]||a,v[a]=b),this},overrideMimeType:function(a){return null==k&&(o.mimeType=a),this},statusCode:function(a){var b;if(a)if(k)y.always(a[y.status]);else for(b in a)u[b]=[u[b],a[b]];return this},abort:function(a){var b=a||x;return e&&e.abort(b),A(0,b),this}};if(s.promise(y),o.url=((b||o.url||tb.href)+\"\").replace(Hb,tb.protocol+\"//\"),o.type=c.method||c.type||o.method||o.type,o.dataTypes=(o.dataType||\"*\").toLowerCase().match(L)||[\"\"],null==o.crossDomain){j=d.createElement(\"a\");try{j.href=o.url,j.href=j.href,o.crossDomain=Lb.protocol+\"//\"+Lb.host!=j.protocol+\"//\"+j.host}catch(z){o.crossDomain=!0}}if(o.data&&o.processData&&\"string\"!=typeof o.data&&(o.data=r.param(o.data,o.traditional)),Nb(Ib,o,c,y),k)return y;l=r.event&&o.global,l&&0===r.active++&&r.event.trigger(\"ajaxStart\"),o.type=o.type.toUpperCase(),o.hasContent=!Gb.test(o.type),f=o.url.replace(Cb,\"\"),o.hasContent?o.data&&o.processData&&0===(o.contentType||\"\").indexOf(\"application/x-www-form-urlencoded\")&&";
    const char PROGMEM_JS_JQUERY11[] PROGMEM = "(o.data=o.data.replace(Bb,\"+\")):(n=o.url.slice(f.length),o.data&&(f+=(vb.test(f)?\"&\":\"?\")+o.data,delete o.data),o.cache===!1&&(f=f.replace(Db,\"$1\"),n=(vb.test(f)?\"&\":\"?\")+\"_=\"+ub++ +n),o.url=f+n),o.ifModified&&(r.lastModified[f]&&y.setRequestHeader(\"If-Modified-Since\",r.lastModified[f]),r.etag[f]&&y.setRequestHeader(\"If-None-Match\",r.etag[f])),(o.data&&o.hasContent&&o.contentType!==!1||c.contentType)&&y.setRequestHeader(\"Content-Type\",o.contentType),y.setRequestHeader(\"Accept\",o.dataTypes[0]&&o.accepts[o.dataTypes[0]]?o.accepts[o.dataTypes[0]]+(\"*\"!==o.dataTypes[0]?\", \"+Kb+\"; q=0.01\":\"\"):o.accepts[\"*\"]);for(m in o.headers)y.setRequestHeader(m,o.headers[m]);if(o.beforeSend&&(o.beforeSend.call(p,y,o)===!1||k))return y.abort();if(x=\"abort\",t.add(o.complete),y.done(o.success),y.fail(o.error),e=Nb(Jb,o,c,y)){if(y.readyState=1,l&&q.trigger(\"ajaxSend\",[y,o]),k)return y;o.async&&o.timeout>0&&(i=a.setTimeout(function(){y.abort(\"timeout\")},o.timeout));try{k=!1,e.send(v,A)}catch(z){if(k)throw z;A(-1,z)}}else A(-1,\"No Transport\");function A(b,c,d,h){var j,m,n,v,w,x=c;k||(k=!0,i&&a.clearTimeout(i),e=void 0,g=h||\"\",y.readyState=b>0?4:0,j=b>=200&&b<300||304===b,d&&(v=Pb(o,y,d)),v=Qb(o,v,y,j),j?(o.ifModified&&(w=y.getResponseHeader(\"Last-Modified\"),w&&(r.lastModified[f]=w),w=y.getResponseHeader(\"etag\"),w&&(r.etag[f]=w)),204===b||\"HEAD\"===o.type?x=\"nocontent\":304===b?x=\"notmodified\":(x=v.state,m=v.data,n=v.error,j=!n)):(n=x,!b&&x||(x=\"error\",b<0&&(b=0))),y.status=b,y.statusText=(c||x)+\"\",j?s.resolveWith(p,[m,x,y]):s.rejectWith(p,[y,x,n]),y.statusCode(u),u=void 0,l&&q.trigger(j?\"ajaxSuccess\":\"ajaxError\",[y,o,j?m:n]),t.fireWith(p,[y,x]),l&&(q.trigger(\"ajaxComplete\",[y,o]),--r.active||r.event.trigger(\"ajaxStop\")))}return y},getJSON:function(a,b,c){return r.get(a,b,c,\"json\")},getScript:function(a,b){return r.get(a,void 0,b,\"script\")}}),r.each([\"get\",\"post\"],function(a,b){r[b]=function(a,c,d,e){return r.isFunction(c)&&(e=e||d,d=c,c=void 0),r.ajax(r.extend({url:a,type:b,dataType:e,data:c,success:d},r.isPlainObject(a)&&a))}}),r._evalUrl=function(a){return r.ajax({url:a,type:\"GET\",dataType:\"script\",cache:!0,async:!1,global:!1,\"throws\":!0})},r.fn.extend({wrapAll:function(a){var b;return this[0]&&(r.isFunction(a)&&(a=a.call(this[0])),b=r(a,this[0].ownerDocument).eq(0).clone(!0),this[0].parentNode&&b.insertBefore(this[0]),b.map(function(){var a=this;while(a.firstElementChild)a=a.firstElementChild;return a}).append(this)),this},wrapInner:function(a){return r.isFunction(a)?this.each(function(b){r(this).wrapInner(a.call(this,b))}):this.each(function(){var b=r(this),c=b.contents();c.length?c.wrapAll(a):b.append(a)})},wrap:function(a){var b=r.isFunction(a);return this.each(function(c){r(this).wrapAll(b?a.call(this,c):a)})},unwrap:function(a){return this.parent(a).not(\"body\").each(function(){r(this).replaceWith(this.childNodes)}),this}}),r.expr.pseudos.hidden=function(a){return!r.expr.pseudos.visible(a)},r.expr.pseudos.visible=function(a){return!!(a.offsetWidth||a.offsetHeight||a.getClientRects().length)},r.ajaxSettings.xhr=function(){try{return new a.XMLHttpRequest}catch(b){}};var Rb={0:200,1223:204},Sb=r.ajaxSettings.xhr();o.cors=!!Sb&&\"withCredentials\"in Sb,o.ajax=Sb=!!Sb,r.ajaxTransport(function(b){var c,d;if(o.cors||Sb&&!b.crossDomain)return{send:function(e,f){var g,h=b.xhr();if(h.open(b.type,b.url,b.async,b.username,b.password),b.xhrFields)for(g in b.xhrFields)h[g]=b.xhrFields[g];b.mimeType&&h.overrideMimeType&&h.overrideMimeType(b.mimeType),b.crossDomain||e[\"X-Requested-With\"]||(e[\"X-Requested-With\"]=\"XMLHttpRequest\");for(g in e)h.setRequestHeader(g,e[g]);c=function(a){return function(){c&&(c=d=h.onload=h.onerror=h.onabort=h.onreadystatechange=null,\"abort\"===a?h.abort():\"error\"===a?\"number\"!=typeof h.status?f(0,\"error\"):f(h.status,h.statusText):f(Rb[h.status]||h.status,h.statusText,\"text\"!==(h.responseType||\"text\")||\"string\"!=typeof h.responseText?{binary:h.response}:{text:h.responseText},h.getAllResponseHeaders()))}},h.onload=c(),d=h.onerror=c(\"error\"),void 0!==h.onabort?h.onabort=d:h.onreadystatechange=function(){4===h.readyState&&a.setTimeout(function(){c&&d()})},c=c(\"abort\");try{h.send(b.hasContent&&b.data||null)}catch(i){if(c)throw i}},abort:function(){c&&c()}}}),r.ajaxPrefilter(function(a){a.crossDomain&&(a.contents.script=!1)}),r.ajaxSetup({accepts:{script:\"text/javascript, application/javascript, application/ecmascript, application/x-ecmascript\"},contents:{script:/\\b(?:java|ecma)script\\b/},converters:{\"text script\":function(a){return r.globalEval(a),a}}}),r.ajaxPrefilter(\"script\",function(a){void 0===a.cache&&(a.cache=!1),a.crossDomain&&(a.type=\"GET\")}),r.ajaxTransport(\"script\",function(a){if(a.crossDomain){var b,c;return{send:function(e,f){b=r(\"<script>\").prop({charset:a.scriptCharset,src:a.url}).on(\"load error\",c=function(a){b.remove(),c=null,a&&f(\"error\"===a.type?404:200,a.type)}),d.head.appendChild(b[0])},abort:function(){c&&c()}}}});var Tb=[],Ub=/(=)\\?(?=&|$)|\\?\\?/;r.ajaxSetup({jsonp:\"callback\",jsonpCallback:function(){var a=Tb.pop()||r.expando+\"_\"+ub++;return this[a]=!0,a}}),r.ajaxPrefilter(\"json jsonp\",function(b,c,d){var e,f,g,h=b.jsonp!==!1&&(Ub.test(b.url)?\"url\":\"string\"==typeof b.data&&0===(b.contentType||\"\").indexOf(\"application/x-www-form-urlencoded\")&&Ub.test(b.data)&&\"data\");if(h||\"jsonp\"===b.dataTypes[0])return e=b.jsonpCallback=r.isFunction(b.jsonpCallback)?b.jsonpCallback():b.jsonpCallback,h?b[h]=b[h].replace(Ub,\"$1\"+e):b.jsonp!==!1&&(b.url+=(vb.test(b.url)?\"&\":\"?\")+b.jsonp+\"=\"+e),b.converters[\"script json\"]=function(){return g||r.error(e+\" was not called\"),g[0]},b.dataTypes[0]=\"json\",f=a[e],a[e]=function(){g=arguments},d.always(function(){void 0===f?r(a).removeProp(e):a[e]=f,b[e]&&(b.jsonpCallback=c.jsonpCallback,Tb.push(e)),g&&r.isFunction(f)&&f(g[0]),g=f=void 0}),\"script\"}),o.createHTMLDocument=function(){var a=d.implementation.createHTMLDocument(\"\").body;return a.innerHTML=\"<form></form><form></form>\",2===a.childNodes.length}(),r.parseHTML=function(a,b,c){if(\"string\"!=typeof a)return[];\"boolean\"==typeof b&&(c=b,b=!1);var e,f,g;return b||(o.createHTMLDocument?(b=d.implementation.createHTMLDocument(\"\"),e=b.createElement(\"base\"),e.href=d.location.href,b.head.appendChild(e)):b=d),f=C.exec(a),g=!c&&[],f?[b.createElement(f[1])]:(f=qa([a],b,g),g&&g.length&&r(g).remove(),r.merge([],f.childNodes))},r.fn.load=function(a,b,c){var d,e,f,g=this,h=a.indexOf(\" \");return h>-1&&(d=pb(a.slice(h)),a=a.slice(0,h)),r.isFunction(b)?(c=b,b=void 0):b&&\"object\"==typeof b&&(e=\"POST\"),g.length>0&&r.ajax({url:a,type:e||\"GET\",dataType:\"html\",data:b}).done(function(a){f=arguments,g.html(d?r(\"<div>\").append(r.parseHTML(a)).find(d):a)}).always(c&&function(a,b){g.each(function(){c.apply(this,f||[a.responseText,b,a])})}),this},r.each([\"ajaxStart\",\"ajaxStop\",\"ajaxComplete\",\"ajaxError\",\"ajaxSuccess\",\"ajaxSend\"],function(a,b){r.fn[b]=function(a){return this.on(b,a)}}),r.expr.pseudos.animated=function(a){return r.grep(r.timers,function(b){return a===b.elem}).length},r.offset={setOffset:function(a,b,c){var d,e,f,g,h,i,j,k=r.css(a,\"position\"),l=r(a),m={};\"static\"===k&&(a.style.position=\"relative\"),h=l.offset(),f=r.css(a,\"top\"),i=r.css(a,\"left\"),j=(\"absolute\"===k||\"fixed\"===k)&&(f+i).indexOf(\"auto\")>-1,j?(d=l.position(),g=d.top,e=d.left):(g=parseFloat(f)||0,e=parseFloat(i)||0),r.isFunction(b)&&(b=b.call(a,c,r.extend({},h))),null!=b.top&&(m.top=b.top-h.top+g),null!=b.left&&(m.left=b.left-h.left+e),\"using\"in b?b.using.call(a,m):l.css(m)}},r.fn.extend({offset:function(a){if(arguments.length)return void 0===a?this:this.each(function(b){r.offset.setOffset(this,a,b)});var b,c,d,e,f=this[0];if(f)return f.getClientRects().length?(d=f.getBoundingClientRect(),b=f.ownerDocument,c=b.documentElement,e=b.defaultView,{top:d.top+e.pageYOffset-c.clientTop,left:d.left+e.pageXOffset-c.clientLeft}):{top:0,left:0}},position:function(){if(this[0]){var a,b,c=this[0],d={top:0,left:0};return\"fixed\"===r.css(c,\"position\")?b=c.getBoundingClientRect():(a=this.offsetParent(),b=this.offset(),B(a[0],\"html\")||(d=a.offset()),d={top:d.top+r.css(a[0],\"borderTopWidth\",!0),left:d.left+r.css(a[0],\"borderLeftWidth\",!0)}),{top:b.top-d.top-r.css(c,\"marginTop\",!0),left:b.left-d.left-r.css(c,\"marginLeft\",!0)}}},offsetParent:function(){return this.map(function(){var a=this.offsetParent;while(a&&\"static\"===r.css(a,\"position\"))a=a.offsetParent;return a||ra})}}),r.each({scrollLeft:\"pageXOffset\",scrollTop:\"pageYOffset\"},function(a,b){var c=\"pageYOffset\"===b;r.fn[a]=function(d){return T(this,function(a,d,e){var f;return r.isWindow(a)?f=a:9===a.nodeType&&(f=a.defaultView),void 0===e?f?f[b]:a[d]:void(f?f.scrollTo(c?f.pageXOffset:e,c?e:f.pageYOffset):a[d]=e)},a,d,arguments.length)}}),r.each([\"top\",\"left\"],function(a,b){r.cssHooks[b]=Pa(o.pixelPosition,function(a,c){if(c)return c=Oa(a,b),Ma.test(c)?r(a).position()[b]+\"px\":c})}),r.each({Height:\"height\",Width:\"width\"},function(a,b){r.each({padding:\"inner\"+a,content:b,\"\":\"outer\"+a},function(c,d){r.fn[d]=function(e,f){var g=arguments.length&&(c||\"boolean\"!=typeof e),h=c||(e===!0||f===!0?\"margin\":\"border\");return T(this,function(b,c,e){var f;return r.isWindow(b)?0===d.indexOf(\"outer\")?b[\"inner\"+a]:b.document.documentElement[\"client\"+a]:9===b.nodeType?(f=b.documentElement,Math.max(b.body[\"scroll\"+a],f[\"scroll\"+a],b.body[\"offset\"+a],f[\"offset\"+a],f[\"client\"+a])):void 0===e?r.css(b,c,h):r.style(b,c,e,h)},b,g?e:void 0,g)}})}),r.fn.extend({bind:function(a,b,c){return this.on(a,null,b,c)},unbind:function(a,b){return this.off(a,null,b)},delegate:function(a,b,c,d){return this.on(b,a,c,d)},undelegate:function(a,b,c){return 1===arguments.length?this.off(a,\"**\"):this.off(b,a||\"**\",c)}}),r.holdReady=function(a){a?r.readyWait++:r.ready(!0)},r.isArray=Array.isArray,r.parseJSON=JSON.parse,r.nodeName=B,\"function\"==typeof define&&define.amd&&define(\"jquery\",[],function(){return r});var Vb=a.jQuery,Wb=a.$;return r.noConflict=function(b){return a.$===r&&(a.$=Wb),b&&a.jQuery===r&&(a.jQuery=Vb),r},b||(a.jQuery=a.$=r),r});";
    
    const char *const AirNginClient::PROGMEM_JS_JQUERY[] PROGMEM = {
        PROGMEM_JS_JQUERY1,
        PROGMEM_JS_JQUERY2,
        PROGMEM_JS_JQUERY3,
        PROGMEM_JS_JQUERY4,
        PROGMEM_JS_JQUERY5,
        PROGMEM_JS_JQUERY6,
        PROGMEM_JS_JQUERY7,
        PROGMEM_JS_JQUERY8,
        PROGMEM_JS_JQUERY9,
        PROGMEM_JS_JQUERY10,
        PROGMEM_JS_JQUERY11};
    const byte AirNginClient::maxPROGMEM_JS_JQUERY = 11;
    
    
    
   /*---- Main / Network ----*/
const char PROGMEM_JS_MAIN1[] PROGMEM = "var _c=String.fromCharCode,_w=window;if(String.prototype.trim===undefined){String.prototype.trim=function(){return this.replace(/^ +| +$/g,'')}}if(_w.atob===undefined){_w.atob=function(s){if(s===undefined||s===null||s==='')return'';var b=l=0,r='',s=s.split(''),i,m='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';for(i in s){b=(b<<6)+m.indexOf(s[i]);l+=6;if(l>=8)r+=String.fromCharCode((b>>>(l-=8))&0xff)}return r}}_w.charTab=_c(9);_w.charLine=_c(10);_w.charEnter=_c(13);_w.charAmpersand=_c(38);_w.charQuotation=_c(39);_w.charQuotation2=_c(34);_w.charBackslash=_c(92);_w.charBox=_c(9619);if(_w.OPT===undefined)_w.OPT={};_w.SG={Data:{}};_w.SS={Data:{}};_w.ST={Data:{}};_w.SN={Data:{}};_w.SU={Data:{}};_w.T_LIGHT=0x00;_w.T_HEX_LIGHT='00';_w.T_SENKEY=0x01;_w.T_HEX_SENKEY='01';_w.T_CURTAIN=0x02;_w.T_HEX_CURTAIN='02';_w.T_FANCOIL_SIMPLE=0x03;_w.T_HEX_FANCOIL_SIMPLE='03';_w.T_COOLER=0x04;_w.T_HEX_COOLER='04';_w.T_RGB=0x05;_w.T_HEX_RGB='05';_w.T_IRREMOTE=0x06;_w.T_HEX_IRREMOTE='06';_w.T_GSM=0x07;_w.T_HEX_GSM='07';_w.T_SIREN=0x08;_w.T_HEX_SIREN='08';_w.T_RELAY_REMOTE=0x09;_w.T_HEX_RELAY_REMOTE='09';_w.T_TEMP=0x0B;_w.T_HEX_TEMP='0B';_w.T_LUX=0x0C;_w.T_HEX_LUX='0C';_w.T_GAZ=0x0D;_w.T_HEX_GAZ='0D';_w.T_MOTION=0x0E;_w.T_HEX_MOTION='0E';_w.T_MAGNET=0x0F;_w.T_HEX_MAGNET='0F';_w.T_HUMIDITY_AIR=0x10;_w.T_HEX_HUMIDITY_AIR='10';_w.T_HUMIDITY_SOUL=0x11;_w.T_HEX_HUMIDITY_SOUL='11';_w.T_KEYPAD=0x12;_w.T_HEX_KEYPAD='12';_w.T_RF433_SIMPLE=0x13;_w.T_HEX_RF433_SIMPLE='13';_w.T_RF433_TOGGLE=0x14;_w.T_HEX_RF433_TOGGLE='14';_w.T_DIGITAL_TOGGLE=0x15;_w.T_HEX_DIGITAL_TOGGLE='15';_w.T_DIGITAL_DOUBLE=0x16;_w.T_HEX_DIGITAL_DOUBLE='16';_w.T_THERMOSTAT_1PIPE=0x17;_w.T_HEX_THERMOSTAT_1PIPE='17';_w.T_THERMOSTAT_2PIPE=0x18;_w.T_HEX_THERMOSTAT_2PIPE='18';_w.T_THERMOSTAT_0PIPE=0x19;_w.T_HEX_THERMOSTAT_0PIPE='19';_w.T_THERMOSTAT_COOLER=0x1A;_w.T_HEX_THERMOSTAT_COOLER='1A';_w.T_PLAYER_MINI=0x1E;_w.T_HEX_PLAYER_MINI='1E';_w.lang_all={'fa':{SS_Exim_Filename:'نام فایل را جهت استخراج وارد نمایید',SS_Exim_NeedRF:'آیا کدهای لرن شده آراف433 نیز لحاظ شود ؟',SS_Exim_NeedIR:'آیا کدهای لرن شده ریموت مادون قرمز نیز لحاظ شود ؟',SS_Exim_NeedSetting:'آیا تنظیمات آدرس دهی و سناریو انجام شود ؟',SN_Project_CorrectCode:'لطفا شناسه پروژه را بصورت صحیح وارد نمایید (12 الی 20 حرف)',SN_Project_CorrectExtender:'درصورتی که میخواهید این ابزار بصورت اکستندر عمل کرده و واسط بین سیم و بیسیم باشد باید نوع مدار مربوطه را بطور صحیح انتخاب کنید (WiOT / WireFi)',SN_Project_CorrectCircuit:'لطفا نوع ارتباط دستگاه را انتخاب نمایید (نوع مدار سیمی یا بیسیم و...)',SN_Project_CorrectEncryption:'لطفا کلید رمزنگاری اطلاعات را بصورت صحیح وارد نمایید (0 الی 16 حرف)',SN_Mesh_CorrectChannel:'لطفا عدد کانال را برای تنظیم صحیح فرکانس بدون تداخل سایر وای فای ها انتخاب کنید ، ضمنا توجه داشته باشید که در هر شبکه مش ابزارهای مرتبط باید دارای کانال هماهنگ با هم باشند',SN_Mesh_CorrectName:'نام مش را بصورت صحیح وارد کنید (تعداد 1 الی 6 حرف)',SN_Mesh_CorrectPass:'رمزعبور مش را بصورت صحیح (حداقل 8 حرف و حداکثر 16 حرف) وارد کنید',SN_Modem_WarningChannel:'هشدار'+charEnter+'عدد کانال باید با عدد موجود در تنظیمات مودم یکسان باشد ، درصورتی که از صحت آن اطلاع ندارید ، مقدار را روی حالت خودکار تنظیم کنید',SN_Modem_CorrectSSID:'شناسه مودم را بصورت صحیح وارد کنید (بین 2 الی 32 حرف)',SN_Modem_CorrectPass:'رمزعبور مودم را بصورت  صحیح (حداقل 8 و حداکثر 63 حرف) وارد نمایید',SN_Mqtt_CorrectBroker:'لطفا آدرس لینک ارتباطی سرور را بصورت صحیح وارد نمایید',SN_Mqtt_CorrectUser:'نام کاربری جهت ارتباط با سرور را بصورت صحیح (تا 16 حرف فقط شامل اعداد و حروف انگلیسی) وارد نمایید',SN_Mqtt_CorrectPass:'رمزعبور جهت ارتباط با سرور را بصورت صحیح (تا 16 حرف فقط شامل اعداد و حروف انگلیسی) وارد نمایید',SN_Save_SelectTargetDvc:'ابتدا باید یکی از گزینه ها را انتخاب نمایید تا تنظیمات روی همین ابزار به تنهایی یا بهمراه ابزارهای متصل در مش مربوطه اعمال گردد',SU_UpdateErr:'آپدیت با خطا مواجه شد'+charEnter+'احتمالا آپدیت انجام نشده و شما پس از بررسی میتوانید مجددا اقدام نمایید',SU_UpdateFail:'آپدیت فریمور...'+charEnter+'احتمالا آپدیت انجام نشده و شما پس از بررسی میتوانید مجددا اقدام نمایید',SU_UpdateOk:'فریمور آپدیت شد'+charEnter+'آپدیت با موفقیت انجام شد و ابزار تا لحظاتی دیگر ریست خواهد شد',SU_UpdateWait:'فایل بارگذاری شد ، تا تکمیل آپدیت صبر کنید',SU_WrongFile:'این فایل حاوی آپدیت متناسب با این ابزار نیست ، لطفا فایل صحیح را بارگذاری و مجددا اقدام نمایید',SU_SelectFile:'ابتدا فایل حاوی آپدیت را بارگذاری کرده ، سپس  مجددا اقدام نمایید',SU_SureToCleanMemory:'تنها در برخی از آپدیت ها لازم است که حافظه پاک شود ، در هرصورت با انجام این کار همه تنظیمات قبلی پاک خواهند شد '+charEnter+charEnter+'آیا از پاک کردن حافظه فعلی و راه اندازی مجدد دستگاه اطمینان دارید؟',SU_CurrentVersion:'نسخه فعلی فریمور : ',SG_AskForBack:'آیا میخواهید از این پنل خارج شده و به صفحه انتخاب پیکربندی بازگردید ؟',SG_AskForDefault:'آیا میخواهید تغییرات ذخیره نشده ی فعلی لغو شده و مجددا جهت انجام تنظیمات وارد شوید ؟',SG_CleanSure:'آیا از حذف این مورد اطمینان دارید ؟',SG_ExitSure:'آیا میخواهید پنل تنظیمات را بسته و از حالت کانفیگ ابزار خارج شوید ؟',SG_RebootSure:'آیا میخواهید ابزارهای متصل در این شبکه مش ، همگی باهم مجددا راه اندازی شوند؟',SG_OperationWait:'در حال انجام عملیات ، لطفا کمی صبر کنید',SG_OperationSuccess:'عملیات با موفقیت انجام شد',SG_OperationFail:'عملیات با خطا مواجه شد ، پس از اطمینان از صحت ارتباط دستگاه میتوانید مجددا اقدام نمایید',SG_DataCannotLoad:'اطلاعات بارگیری نمی شوند، لطفاً دوباره سعی کنید یا صفحه را بروزرسانی نمایید',SG_PageWarning:'هشدار !'+charEnter+'انجام هر گونه تغییرات غیر اصولی در این صفحه میتواند کارکرد ابزار را مختل کند ، اگر در مورد نحوه صحیح انجام آن اطلاعی ندارید بهتر است با زدن دکمه کنسل از ورود به این صفحه خودداری نمایید',SG_EnterPassword:'رمز عبور را وارد نمایید :',SG_Disconnect:'قطع ارتباط . . .'},'en':{SS_Exim_Filename:'Enter the Export-file name',SS_Exim_NeedRF:'Do you need RF433-learned codes',SS_Exim_NeedIR:'Do you need Ir-Remote learned codes',SS_Exim_NeedSetting:'Do you need to import other settings (like Address, Scenarios,...) ?',SN_Project_CorrectCode:'Enter the correct Project-Code (12 to 20 chars)',SN_Project_CorrectExtender:'If you need EXTENDER, first select the correct connection type of wired with wireless (WiOT / WireFi)',SN_Project_CorrectCircuit:'Select one of the connection type (Device Circuit)',SN_Project_CorrectEncryption:'Enter the correct encryption key (0 to 16 chars)',SN_Mesh_CorrectChannel:'Select a Mesh-Channel to set Wifi-Mesh frequecy without any noise',SN_Mesh_CorrectName:'Enter the correct Mesh-Name (1 to 5 chars)',SN_Mesh_CorrectPass:'Enter the Mesh-Password (8 to 20 chars)',SN_Modem_WarningChannel:'Warning'+charEnter+'This channel must be set as the same as your Modem-Channel. Also it can set to Auto, if you are not sure about the channel number',SN_Modem_CorrectSSID:'Enter the correct Modem-SSID (2 to 32 chars)',SN_Modem_CorrectPass:'Enter the correct Modem-Password (8 to 63 chars)',SN_Mqtt_CorrectBroker:'Enter the correct URL of MQTT-Broker',SN_Mqtt_CorrectUser:'Enter the correct username for MQTT-Connection',SN_Mqtt_CorrectPass:'Enter the correct password for MQTT-Connection',SN_Save_SelectTargetDvc:'Select one of target options (This_Device_Only / Mesh_Group_Devices) to continue',SU_UpdateErr:'Update Error !'+charEnter+'Probably The Device Is Not Updated ,Try Again After Checking...',SU_UpdateFail:'Firmware Update...'+charEnter+'Probably The Device Is Not Updated , You Can Try Again After Checking...',SU_UpdateOk:'Firmware Updated !'+charEnter+'The Device Updated & It Will Restart Now ...',SU_UpdateWait:'The file uploaded, Wait to complete update...',SU_WrongFile:'The file is not compatible with this device type, Use the correct update file and try again',SU_SelectFile:'You must select update file, then try again',SU_SureToCleanMemory:'Only some version of updates needs to clean memory, Note that this operation will remove all current settings'+charEnter+charEnter+' Are you sure to CLEAN MEMORY and RESTART ?',SU_CurrentVersion:'Current Firmware Version : ',SG_AskForBack:'Are you sure to return back to the config panel ?',SG_AskForDefault:'Are you sure to cancel all unsaved changes and reload to config again ?',SG_CleanSure:'Are you sure to CLEAN this case ?',SG_ExitSure:'Are you sure to CLOSE this configuration and EXIT ?";
const char PROGMEM_JS_MAIN2[] PROGMEM = "',SG_RebootSure:'Are you sure to REBOOT all devices that curently exist in this MESH-Network ?',SG_OperationWait:'Please wait , to complete the operation',SG_OperationSuccess:'The operation done successfully',SG_OperationFail:'The operation failed !'+charEnter+'Check the connection and try again',SG_DataCannotLoad:'Data cannot be loaded, please try again or refresh the page',SG_PageWarning:'Warning !'+charEnter+'Making any unprincipled changes on this page can disrupt the functionality of the device, Press cancel button if you do not know how to do it correctly',SG_EnterPassword:'Enter the password :',SG_Disconnect:'Disconnected . . .'}};_w.lang_text={};_w.lang_mode='fa';_w.Tool_LanguageSet=function(lng){lng=(lng+'').toLowerCase().trim();lang_mode=(lng=='1'||lng=='en'?'en':'fa');lang_text=lang_all[lang_mode];var delboxes=$(lang_mode=='en'?'.lang-fa':'.lang-en');if(delboxes&&delboxes.length>0){for(var i=0;i<delboxes.length;i++){try{delboxes[i].outerHTML=''}catch(x){console.log('Lang err: ',x)}}}delboxes=$(lang_mode=='en'?'.lang-fa':'.lang-en');console.log(lang_mode,'lang not deleted: '+delboxes.length);setTimeout(SG.Selection_Show,200)};_w.Tool_MsgShow=function(tx,e){alert(charEnter+tx+charEnter);if(e)try{e.focus()}catch(x){};return''};_w.Tool_PageShow=function(id,warn,pass){if(warn){var warnCheck=false;if(_w.sessionStorage){if(sessionStorage.getItem('warncheck'+id)=='1')warnCheck=true;sessionStorage.setItem('warncheck'+id,'1')}if(!warnCheck){if(!confirm(lang_text['SG_PageWarning']))return}}if(pass){if(!SG.PasswordChecked&&_w.sessionStorage){if(sessionStorage.getItem('passcheck')=='1')SG.PasswordChecked=true}if(!SG.PasswordChecked){if((prompt(lang_text['SG_EnterPassword'])+'').replace(/[^0-9^a-z^A-Z]/g,'')=='0000'){SG.PasswordChecked=true;if(_w.sessionStorage)sessionStorage.setItem('passcheck','1')}else return}}var i,act=$('.page-show');for(i=0;i<act.length;i++)Tool_RemoveClass(act[i],'page-show');Tool_AddClass($(id)[0],'page-show')};_w.Tool_PageSave=function(q,delay,tx){try{console.log('AjaxData: ',Tool_ObjClone(q))}catch(x){console.log('AjaxData: ',q)}SG.Wait_Show(tx,delay);var url=Tool_UrlAddress('/set');$.ajax({url:url,type:'POST',dataType:'json',data:'data='+JSON.stringify(q),success:Tool_PageReload})};_w.Tool_PageSend=function(q,success,error){try{console.log('AjaxData: ',Tool_ObjClone(q))}catch(x){console.log('AjaxData: ',q)}var url=Tool_UrlAddress('/set');success=success||function(){Tool_MsgShow(lang_text['SG_OperationSuccess'])};error=error||function(){Tool_MsgShow(lang_text['SG_OperationFail'])};$.ajax({url:url,type:'POST',dataType:'json',data:'data='+JSON.stringify(q),async:false,success:success,error:function(r){if((typeof(r)+'').toLowerCase()=='string'){success(r);return}if(r){if(r.responseText){success(r.responseText);return}if(r.status==200){success(r.statusText);return}r=r.status}error(r)}})};_w.Tool_PageGet=function(q,success,error){console.log('PageGet AjaxData: ',q);var url=Tool_UrlAddress('/get');$.ajax({url:url,type:'POST',async:false,data:'data='+JSON.stringify(q),success:success||function(r){console.log('PageGet success',r)},error:function(r){console.log('PageGet error',r);if(r&&r.responseText){success(r.responseText)}else{error?error(r):Tool_MsgShow(lang_text['SG_Disconnect'])}}})};_w.Tool_PageReload=function(){_w.location.reload(true)};_w.Tool_UrlGo=function(url){if(_w.location.href)_w.location.href=url;else _w.location=url};_w.Tool_UrlQuery=function(q){var loc=_w.location+'';if(q===undefined){var p=loc.indexOf('?');if(p>0)loc=loc.substr(p+1);else loc='';return decodeURIComponent(loc)}else{var match=RegExp('([?&]{1})('+q+'=)([^&]*)','m').exec(loc);return match&&decodeURIComponent(match[3].replace(new RegExp('['+charBackslash+charBackslash+']+','g'),' '))}};_w.Tool_UrlAddress=function(q){if(q){var r=1000000000;try{r=Math.floor(Math.random()*r+r);r=r.toString(36)}catch(x){};q=q+(q.indexOf('?')>0?'&':'?')+'token='+OPT.token+'&rand='+r}else q='';var url=(_w.location||document.URL);url=((url.href||url)+'').replace(new RegExp('[/]{1}$','g'),'').replace('//','###');var p=url.indexOf('/');if(p>0)url=url.substr(0,p);return url.replace('###','//')+q};_w.Tool_AddClass=function(el,cls){cls=' '+(cls||'')+' ';var c=' '+(el.getAttribute('class')||'')+' ';if(c.indexOf(cls)<0)el.setAttribute('class',(c+cls).replace(/ {2,}/g,' ').trim())};_w.Tool_DomHasValue=function(el,value){try{var inner=$(el)[0].innerHTML.replace(new RegExp(charQuotation2,'g'),charQuotation).replace(/ *= */g,'=').toLowerCase();return(inner.indexOf('value='+charQuotation+value.toLowerCase()+charQuotation)>0)}catch(x){}return false};_w.Tool_DomDisable=function(e,dis){for(i=0;i<e.length;i++){if(dis)e[i].setAttribute('disabled','disabled');else e[i].removeAttribute('disabled')}};_w.Tool_RemoveClass=function(el,cls){cls=' '+(cls||'')+' ';var c=' '+(el.getAttribute('class')||'')+' ';if(c.indexOf(cls)>=0)el.setAttribute('class',c.replace(cls,' ').replace(/ {2,}/g,' ').trim())};_w.Tool_NumToHex=function(n,len){len=(len||1)*2;var r=Number('0'+n).toString(16);while(r.length<len)r='0'+r;return r.toUpperCase()};_w.Tool_HexToNum=function(h){h=h.replace(/[^0-9^A-F]/g,'');if(h.length%2!=0)h='0'+h;return parseInt(h||'00',16)};_w.Tool_NumToTime=function(sec,maxM){maxM=Number(''+maxM)||240;sec=Number((''+sec).replace(/[^0-9]/g,''))||0;var s,m=Math.floor(sec/60);if(m>maxM)m=maxM;s=sec%60;return((m>0)?m+(s>0?'m ':' min'):(''))+((s>0||m+s==0)?s+(m>0?'s':' sec'):(''))};_w.Tool_TimeToNum=function(v,minSec,maxM){minSec=Number(''+minSec)||0;maxM=Number(''+maxM)||240;var s,m;v=(v+'').toLowerCase().replace(/[^0-9^m^\"^:^;]+/g,' ').trim().replace(/[^0-9]+/g,' ').trim();if(v.indexOf(' ')>=0){v=v.split(' ')}else{v=Number(''+v)||0;v=[Math.floor(v/60),v%60]}m=Number(''+v[0])||0;s=Number(''+v[1])||0;if(s+m===0)s=minSec;if(s>59)s=59;if(m>maxM)m=maxM;return(m*60)+s};_w.Tool_StrToHex=function(s){var i,c,res='';for(i=0;i<s.length;i++){c=s.charCodeAt(i).toString(16);res+=(c.length%2==0?c:'0'+c)}return res.toUpperCase()};_w.Tool_HexToStr=function(h){h=h.toUpperCase().replace(/[^0-9^A-F]/g,'');if(h.length%2!=0)h='0'+h;var i,c,res='';for(i=0;i<h.length;i+=2)res+=''+String.fromCharCode(parseInt(h.substr(i,2),16));return res};_w.Tool_JsonCorrect=function(d){try{if((typeof(d)+'').toLowerCase()=='string'){if(d.indexOf(charQuotation2)<0)d=d.replace(/'/g,charQuotation2);if(d.indexOf(charEnter)<0)d=d.split(charEnter+charLine).join(' ').split(charEnter).join(' ').split(charLine).join(' ');if(d.indexOf(charTab)<0)d=d.split(charTab).join('    ');d=d.replace(/[^\\u0020-\\u007E]/g,'\\0').replace(/ *\\0+ */g,' ').trim();return JSON.parse(d)}else if(d&&Object.values(d).length>0)return d}catch(x){}return null};_w.Tool_ObjClone=function(obj){try{return JSON.parse(JSON.stringify(obj))}catch(x){return null}};_w.Tool_FileDownload=function(name,data){const blob=new Blob([data],{type:'text/plain;charset=utf-8'});const url=URL.createObjectURL(blob);const el=document.createElement('a');el.href=url;el.download=name||'download';el.style.display='none';document.body.appendChild(el);el.click();document.body.removeChild(el);URL.revokeObjectURL(url)};_w.Tool_FileRead=function(fn,accept){var b=document.body,f=document.createElement('input');f.setAttribute('type','file');f.setAttribute('accept',accept||'*.*');f.style.display='none';b.appendChild(f);$(f).on('change',function(event){try{var file=$(f).prop('files')[0];if(file==undefined)return fn();var d='',reader=new FileReader();reader.readAsText(file,'UTF-8');reader.onloadend=function(evt){try{d=(evt.target.result||'');fn(d)}catch(x){}b.removeChild(f)};reader.onerror=function(evt){b.removeChild(f)}}catch(x){}});$(f).on('cancel',function(event){b.removeChild(f)});$(f).click()};_w.prepare=function(){var loadall=false;if(_w.$){if(_w.SG&&_w.SN&&_w.SU){if(SG.Loaded&&SN.Loaded&&SU.Loaded){if($('.panel-box')[0])loadall=true}}}if(!loadall){console.log('wait to load...');setTimeout(prepare,500);return}SG.AddStyle();try{var lang=Tool_UrlQuery('lang');if(lang)Tool_LanguageSet(lang);else Tool_UrlGo('#')}catch(x){}try{SS.SetOptions();if(OPT.dvcBrand.toLowerCase().indexOf('vatra')>=0)$('body').addClass('vatra');$('.div-footer')[0].innerHTML='SN: '+OPT.serial+' &nbsp;&nbsp;/&nbsp;&nbsp; Hardware";
const char PROGMEM_JS_MAIN3[] PROGMEM = ": '+OPT.dvc}catch(x){}var logoLoad=function(){try{$.ajax({url:'/logo.png',type:'GET',async:false,success:function(r){if(r&&r.length>500){clearInterval(window.LogoIntervalH);$('#logo')[0].src=r}}})}catch(x){}};if(!OPT.develope){window.LogoIntervalH=setInterval(logoLoad,6000);logoLoad()}$('body')[0].style.display='block';setTimeout(function(){SG.ZoomView(470)},10);SG.Alive_Check()};SG.Selection_Show=function(){if(!Tool_UrlQuery('lang')){Tool_UrlGo('#?lang='+lang_mode)}Tool_PageShow('#SG_Page_Selection')};SG.Selection_Click=function(pg){switch((pg+'').toLowerCase()){case'3':case'network':SN.Prepare();SN.Project_Show();break;case'4':case'update':SU.Prepare();SU.Update_Show();break;case'8':case'exit':SG.Exit();break;case'9':case'reboot':SG.Reboot();break}};SG.Selection_Backed=function(){if(confirm(charEnter+lang_text['SG_AskForBack'])){SG.Selection_Show()}};SG.AnyPage_Cancel=function(){if(confirm(charEnter+lang_text['SG_AskForDefault'])){Tool_PageReload()}else{SG.Selection_Backed()}};SG.Wait_Show=function(tx,delay){if(!tx)tx=lang_text['SG_OperationWait'];$('#SG_wait_msg')[0].innerHTML=(tx+'').replace(new RegExp('['+charEnter+']','gm'),'<br>');Tool_PageShow('#SG_Page_Wait');setInterval(function(){var el=$('#SG_wait_progress')[0],v=el.innerHTML;if(v.length<30)v+=' '+charBox;else v=' '+charBox;el.innerHTML=v},500);if(delay)setTimeout(function(){Tool_PageReload()},delay)};SG.Reboot=function(lng){var t=(lng===2?lang_all.fa['SG_RebootSure']+charEnter+charEnter+lang_all.en['SG_RebootSure']:lang_text['SG_RebootSure']);if(confirm(charEnter+t)){t=(lng===2?lang_all.fa['SG_OperationWait']+charEnter+charEnter+lang_all.en['SG_OperationWait']:lang_text['SG_OperationWait']);Tool_PageSave({section:'general',requestMode:'reboot'},7900,t)}};SG.Exit=function(lng){var question=(lng===2?lang_all.fa['SG_ExitSure']+charEnter+charEnter+lang_all.en['SG_ExitSure']:lang_text['SG_ExitSure']);if(confirm(charEnter+question)){var msg=(lng===2?lang_all.fa['SG_OperationWait']+charEnter+charEnter+lang_all.en['SG_OperationWait']:lang_text['SG_OperationWait']);Tool_PageSave({section:'general',requestMode:'exit'},7900,msg)}};SG.ZoomView=function(pw,ph,rotate){var w,h,b=document.body,e=document.documentElement;h=Math.max(b.offsetHeight,e.clientHeight,e.scrollHeight,e.offsetHeight);w=Math.max(b.offsetWidth,e.clientWidth,e.scrollWidth,e.offsetWidth);var wd=Math.floor(w)-pw;pw=pw/100;wd=100+(Math.floor(10*wd/pw)/10);if(wd>10)for(;(wd*pw)>h;)wd-=0.1;b.style.zoom=wd+'%'};SG.AddStyle=function(){var i,m='',p='',w='',h='',f='',e='px!important;}'+charEnter;for(i=0;i<=500;i++){m+='.m-'+i+',.m-tb-'+i+',.m-t-'+i+'{margin-top:'+i+e+'.m-'+i+',.m-tb-'+i+',.m-b-'+i+'{margin-bottom:'+i+e+'.m-'+i+',.m-lr-'+i+',.m-l-'+i+'{margin-left:'+i+e+'.m-'+i+',.m-lr-'+i+',.m-r-'+i+'{margin-right:'+i+e;w+='.w-'+i+'{width:'+i+e;h+='.h-'+i+'{height:'+i+e;f+='.f-'+i+'{font-size:'+i+e}m=m.replace(/:0px/g,':0');p=m.replace(/.m-/g,'.p-').replace(/margin/g,'padding');$('body').append('<style>'+w+h+f+m+p+'</style>')};SG.Alive_Check=function(){return;SG.Data.PageTimeoutCount=0;setInterval(function(){if(SG.Data.PageTimeoutCount++>3){SG.Data.PageTimeoutCount=0;Tool_MsgShow(lang_text['SG_Disconnect']);Tool_PageReload()}var p,d='',u=Tool_UrlAddress('/alive?requestMode=alive');p=u.indexOf('?');if(p>0){d=u.substr(p+1);u=u.substr(0,p)}$.ajax({url:u,data:d,type:'GET',success:function(){SG.Data.PageTimeoutCount=0}})},33000)};SG.Loaded=true;SN.Prepare=function(){$('#SN_project_circuit')[0].value='iot';$('#SN_project_extender')[0].value='2';$('#SN_project_code')[0].value=OPT.projectCode;$('#SN_project_enckey')[0].value=OPT.encryptionKey;$('#SN_modem_channel')[0].value=OPT.modemChannel;$('#SN_modem_ssid')[0].value=OPT.modemSsid;$('#SN_modem_pass')[0].value=OPT.modemPass;$('#SN_mesh_channel')[0].value=OPT.meshChannel;$('#SN_mesh_name')[0].value=OPT.meshName;$('#SN_mesh_pass')[0].value=OPT.meshPass;$('#SN_mqtt_broker')[0].value=OPT.mqttBroker;$('#SN_mqtt_user')[0].value=OPT.mqttUser;$('#SN_mqtt_pass')[0].value=OPT.mqttPass;var i,r=document.getElementsByName('SN_save_modes');for(i=0;i<r.length;i++){if(r[i].value=='one')r[i].checked=true}};SN.Project_Show=function(){Tool_PageShow('#SN_Page_Project',1);SN.Project_Change()};SN.Project_Next=function(){if(SN.Project_CheckSave()){var cir='iot';if(cir=='wired')SN.Finish_Show();else if(cir=='iot'||cir=='wiot')SN.Modem_Show();else SN.Mesh_Show()}};SN.Project_CheckSave=function(){var tmp,q={},e=$('#SN_project_circuit')[0],v=(e.value||'').toLowerCase().replace(/[^a-z]/g,'');if(v.length<1){Tool_MsgShow(lang_text['SN_Project_CorrectCircuit'],e);return null}q.circuit=v;tmp=v;e=$('#SN_project_extender')[0];v=Number(('0'+e.value).replace(/[^0-9]/g,''));if(v==1||v==3){if(tmp!=='wiot'&&tmp!=='wirefi'){e.value=(v==1?'0':'2');Tool_MsgShow(lang_text['SN_Project_CorrectExtender'],e);return null}}q.extender=v+'';e=$('#SN_project_code')[0];v=(e.value||'').replace(/[^A-Z^a-z^0-9]/g,'');if(v.length<12||v.length>20){Tool_MsgShow(lang_text['SN_Project_CorrectCode'],e);return null}q.projectCode=v;e=$('#SN_project_enckey')[0];v=(e.value||'').replace(/[^A-Z^a-z^0-9]/g,'');if(v.length>16)return Tool_MsgShow(lang_text['SN_Project_CorrectEncryption'],e);q.encryptionKey=v;return q};SN.Project_Change=function(){var cir=$('#SN_project_circuit')[0].value;SN.Data.PageDisabled='';var unlock=function(page,setLock){var i,lst='';if(page=='SN_Page_Modem')lst='SN_modem_channel,SN_modem_ssid,SN_modem_pass,SN_modem_btnext,SN_modem_btprev';else if(page=='SN_Page_Mqtt')lst='SN_mqtt_broker,SN_mqtt_user,SN_mqtt_pass,SN_mqtt_btnext,SN_mqtt_btprev';else if(page=='SN_Page_Mesh')lst='SN_mesh_channel,SN_mesh_name,SN_mesh_pass,SN_mesh_btnext,SN_mesh_btprev';lst=lst.split(',');if(setLock){SN.Data.PageDisabled+=','+page+',';Tool_AddClass($('#'+page)[0],'disabled');for(i=0;i<lst.length;i++)$('#'+lst[i])[0].setAttribute('disabled','disabled')}else{Tool_RemoveClass($('#'+page)[0],'disabled');for(i=0;i<lst.length;i++)$('#'+lst[i])[0].removeAttribute('disabled')}};var lock=function(page){unlock(page,true)};if(cir=='wired'){lock('SN_Page_Modem');lock('SN_Page_Mqtt');lock('SN_Page_Mesh')}else if(cir=='iot'||cir=='wiot'){unlock('SN_Page_Modem');unlock('SN_Page_Mqtt');lock('SN_Page_Mesh')}else if(cir=='wifi'||cir=='wirefi'){lock('SN_Page_Modem');lock('SN_Page_Mqtt');unlock('SN_Page_Mesh')}};SN.Mesh_Show=function(){Tool_PageShow('#SN_Page_Mesh')};SN.Mesh_Next=function(){if(SN.Mesh_CheckSave())SN.Finish_Show()};SN.Mesh_Back=function(){SN.Project_Show()};SN.Mesh_CheckSave=function(){if(SN.Data.PageDisabled.indexOf('SN_Page_Mesh')>=0){return{meshChannel:9,meshName:'',meshPass:''}}var q={},e=$('#SN_mesh_channel')[0],v=(e.value||'').replace(/[^0-9]/g,'');if(!v){Tool_MsgShow(lang_text['SN_Mesh_CorrectChannel'],e);return null}q.meshChannel=v;e=$('#SN_mesh_name')[0];v=(e.value||'').toLowerCase().replace(/[^a-z^0-9]/g,'');if(v.length<1||v.length>6){Tool_MsgShow(lang_text['SN_Mesh_CorrectName'],e);return null}q.meshName=v;e=$('#SN_mesh_pass')[0];v=(e.value||'').replace(/[^A-Z^a-z^0-9]/g,'');if(v.length<8||v.length>16){Tool_MsgShow(lang_text['SN_Mesh_CorrectPass'],e);return null}q.meshPass=v;return q};SN.Modem_Show=function(){Tool_PageShow('#SN_Page_Modem')};SN.Modem_Next=function(){if(SN.Modem_CheckSave(1))SN.Mqtt_Show()};SN.Modem_Back=function(){if(e=$('#SN_project_circuit')[0].value=='iot')SN.Project_Show();else SN.Mesh_Show()};SN.Modem_CheckSave=function(warn){if(SN.Data.PageDisabled.indexOf('SN_Page_Modem')>=0){return{modemChannel:0,modemSsid:'',modemPass:''}}var q={},e=$('#SN_modem_channel')[0],v=(e.value||'').replace(/[^0-9]/g,'');if(warn&&v!='0')Tool_MsgShow(lang_text['SN_Modem_WarningChannel'],e);q.modemChannel=(v||'0');e=$('#SN_modem_ssid')[0];v=(e.value||'');if(v.length<2||v.length>32){Tool_MsgShow(lang_text['SN_Modem_CorrectSSID'],e);return null}q.modemSsid=v;e=$('#SN_modem_pass')[0];v=(e.value||'');if(v.length<8||v.length>63){Tool_MsgShow(lang_text['SN_Modem_CorrectPass'],e);return null}q.modemPass=v.replace(new RegExp(charQuotation2,'g'),'\\\\'+charQuotation2);return q};SN.Mqtt_Show=function(){Tool_PageShow('#SN_Page_Mqtt')};SN.Mqtt_Next=function(){if(SN.Mqtt_CheckSave())SN.Finish_Show()};SN.Mqtt_Back=function(){SN.Modem_Show()};SN.Mqtt_CheckSave=f";
const char PROGMEM_JS_MAIN4[] PROGMEM = "unction(){if(SN.Data.PageDisabled.indexOf('SN_Page_Mqtt')>=0){return{mqttUser:'',mqttPass:'',mqttBroker:'192.168.1.200'}}var tmp,q={},e=$('#SN_mqtt_user')[0],v=(e.value||'').replace(/[^A-Z^a-z^0-9]/g,'');if(v.length<1||v.length>16){Tool_MsgShow(lang_text['SN_Mqtt_CorrectUser'],e);return null}q.mqttUser=v;e=$('#SN_mqtt_pass')[0];v=(e.value||'').replace(/[^A-Z^a-z^0-9]/g,'');if(v.length<1||v.length>16){Tool_MsgShow(lang_text['SN_Mqtt_CorrectPass'],e);return null}q.mqttPass=v;e=$('#SN_mqtt_broker')[0];v=(e.value||'').trim();if(v.length<7||v.length>100){Tool_MsgShow(lang_text['SN_Mqtt_CorrectBroker'],e);return null}tmp=v.replace(new RegExp('[^0-9^'+charBackslash+'.]','g'),' ').trim();if((tmp.indexOf(' ')<0)&&(tmp.length>=7)&&(tmp.split('.').length==4)&&(v.indexOf('http:://')==0))v=v.replace('http://','');q.mqttBroker=v;return q};SN.Finish_Show=function(){Tool_PageShow('#SN_Page_Finish')};SN.Finish_Save=function(exportOnly){var i,v,q=SN.Finish_Check();if(q&&q.target){v=SN.Project_CheckSave();if(!v)return SN.Project_Show();for(i in v)q[i]=v[i];if(q.circuit=='iot'||q.circuit=='wiot'){v=SN.Modem_CheckSave();if(!v)return SN.Modem_Show();for(i in v)q[i]=v[i];v=SN.Mqtt_CheckSave();if(!v)return SN.Mqtt_Show();for(i in v)q[i]=v[i]}else if(q.circuit=='wifi'||q.circuit=='wirefi'){v=SN.Modem_CheckSave();if(!v)return SN.Modem_Show();for(i in v)q[i]=v[i];v={}}if(exportOnly)return q;q.section='network';q.requestMode='save';Tool_PageSend(q,function(){Tool_MsgShow(lang_text['SG_OperationSuccess']);SG.Selection_Show();_w.Tool_PageReload()})}};SN.Finish_Restore=function(){if(SN.Finish_Check())SN.Restore_Show()};SN.Finish_Check=function(){var i,q={},v='',r=document.getElementsByName('SN_save_modes');for(i=0;i<r.length;i++){if(r[i].checked)v=r[i].value}if(!v)return Tool_MsgShow(lang_text['SN_Save_SelectTargetDvc']);q.target=v;return q};SN.Finish_Import=function(){Tool_FileRead(function(data){var q=Tool_JsonCorrect(data);q.section='network';q.requestMode='save';Tool_PageSend(q,function(){SG.Selection_Show();_w.Tool_PageReload()})},'.json')};SN.Finish_Export=function(){var fname,i,d=SN.Finish_Save(true);if(!d)return;d.target='one';i=((OPT.projectCode||'NoProject')+'_'+OPT.dvcType.substr(0,1).toUpperCase()+OPT.dvcType.substr(1)+'_Network_'+d.circuit).replace(/[^A-Z^a-z^0-9^_^ ]/g,'').replace(/ {2,}/g,' ').trim();fname=(prompt(lang_text['SS_Exim_Filename'],i)||'');if(fname)Tool_FileDownload(fname+'.json',JSON.stringify(d))};SN.Restore_Show=function(){Tool_PageShow('#SN_Page_Restore')};SN.Restore_Ok=function(){var q=SN.Finish_Check();if(q&&q.target){q.section='network';q.requestMode='restore';Tool_PageSave(q,7900)}};SN.Loaded=true;SU.Prepare=function(){$('#SU_cver')[0].innerHTML=lang_text['SU_CurrentVersion']+OPT.firmware+'<br>_'+OPT.dvcType.toUpperCase()+'_'};SU.Update_Show=function(){Tool_PageShow('#SU_Page_Update',1)};SU.Update_GetFile=function(){var win=_w.open(OPT.updateUrl+'?requestMode=device&dvcType='+OPT.dvcType+'&ver='+OPT.firmware+'&serial='+OPT.serial,'_balnk');win.focus()};SU.Update_Apply=function(){var ext,filename,file=$('#SU_file')[0].files[0];if(!file){Tool_MsgShow(lang_text['SU_SelectFile']);return}filename=(file.name+'').toLowerCase().split('.').join('@').replace(/[^a-z^0-9^@]+/g,'_').split('@').join('.').replace(/^_+|_+$/g,'_');ext=filename.split('.');ext=(ext[ext.length-1]+'').replace(/[^a-z]+/g,'');function post(bytes){var formData=new FormData();if(!filename)filename='upload_'+OPT.dvcType+'_ver.bin';formData.append('contentType','application/octet-stream');formData.append('update',new File(bytes,filename));$.ajax({url:'/firmware',type:'POST',data:formData,contentType:false,processData:false,xhr:function(){try{var xhr=new window.XMLHttpRequest();xhr.upload.addEventListener('progress',function(evt){if(evt.lengthComputable){var p=Math.round(100*evt.loaded / evt.total)+'%';$('#SU_prg')[0].innerHTML=p;$('#SU_bar')[0].style.width=p}},false);xhr.upload.addEventListener('load',function(){$('#SU_prg')[0].innerHTML=lang_text['SU_UpdateWait']+''},false);return xhr}catch(x){}},success:function(d){d=(d+'').toLowerCase().trim();setTimeout(Tool_MsgShow,2500,lang_text[d=='fail'?'SU_UpdateFail':'SU_UpdateOk'])},error:function(a,b,c){setTimeout(Tool_MsgShow,2500,lang_text['SU_UpdateErr'])}})}var data,jdata,reader=new FileReader();if(ext=='eot'||ext=='json'){data='';reader.onload=function(){jdata=Tool_JsonCorrect(reader.result);if(!jdata)return Tool_MsgShow(lang_text['SU_WrongFile']);try{var i,keys=Object.keys(jdata);for(i=0;i<keys.length;i++){if((keys[i]+'').toLowerCase().replace(/[^a-z^0-9]/g,'')==OPT.dvcType){data=jdata[keys[i]];break}}}catch(x){}if(!data||data.length<100)return Tool_MsgShow(lang_text['SU_WrongFile']);data=atob(data);var bytes=[];for(var i=0;i<data.length;i+=1024){const slice=data.slice(i,i+1024);const byteNumbers=new Array(slice.length);for(var j=0;j<slice.length;j++)byteNumbers[j]=slice.charCodeAt(j);bytes.push(new Uint8Array(byteNumbers))}bytes=new Blob(bytes);post([bytes])};reader.readAsText(file)}else if(ext=='bin'){data=[];reader.onload=function(){try{data=reader.result}catch(x){}if(!data||data.length<10)return Tool_MsgShow(lang_text['SU_WrongFile']);post([data])};reader.readAsArrayBuffer(file)}else{Tool_MsgShow(lang_text['SU_WrongFile']);return}};SU.Update_ChangeFile=function(el){var fileName='...';try{fileName=el.files[0].name}catch(x){var tmp=el.value.split(charBackslash);tmp=tmp[tmp.length-1];tmp=tmp.split('/');fileName=tmp[tmp.length-1]}var target=$('#SU_file-input')[0];target.style['textAlign']='left';target.innerHTML=''+fileName};SU.Update_MemoryClean=function(){if(!SU.goSettingCheck)SU.goSettingCheck='';if(_w.sessionStorage){var v=sessionStorage.getItem('SU_goSettingCheck');if(v)SU.goSettingCheck=v}if(confirm(charEnter+lang_text['SU_SureToCleanMemory'])){Tool_PageSave({section:'update',requestMode:'clean'})}else{}sessionStorage.setItem('SU_goSettingCheck',SU.goSettingCheck)};SU.Loaded=true;";

const char *const AirNginClient::PROGMEM_JS_MAIN[] PROGMEM = {
    PROGMEM_JS_MAIN1,
    PROGMEM_JS_MAIN2,
    PROGMEM_JS_MAIN3,
    PROGMEM_JS_MAIN4};
const byte AirNginClient::maxPROGMEM_JS_MAIN = 4;
    
    
    
    /*---- Style.css ----*/
    const char  PROGMEM_CSS1[] PROGMEM = "@keyframes falldown{0%{opacity:0.01;margin-top:-100vh;overflow:hidden}98%{opacity:.70;margin-top:0}100%{opacity:1;overflow:unset}}@keyframes blinking{0%{opacity:0.02}70%{opacity:1}100%{opacity:0.01}}@keyframes blinking-light{0%{opacity:0.51}50%{opacity:1}100%{opacity:0.51}}body{display:none;font:normal 14px Tahoma,sans-serif;background-color:#808488;user-select:none;animation:0.8s linear falldown}body *{box-sizing:border-box}#logo{width:130px;height:40px}.panel-box{width:450px;margin:0 auto}.div-title{padding:8px 0;background-color:#000000;border-radius:15px 15px 0 0;border:1px solid #d4d6d8;border-bottom:0}.div-footer{border-radius:0 0 10px 10px;padding:10px 0 15px 0;background-color:#a7a7a7;border:1px solid #d4d6d8;border-top:0}.top-menu{width:450px;box-sizing:border-box;margin:0 0 25px 0;padding:1px 10px 5px 10px;border-left:1px solid #ccc;border-right:1px solid #ccc;background-color:#000;font-weight:bold;font-size:16px;text-align:left}.top-menu span{color:#888;background-color:#45484a;cursor:pointer;border:1px solid #666;border-bottom:0;border-radius:7px 7px 0 0;overflow:hidden;display:inline-block;height:25px;text-align:center;margin:0 2px -8px 1px;padding:3px 3px}.top-menu span:hover{color:#f0d008}.top-menu span.menu-active{color:#2c2c2c;background-color:#d4d6d8;border:1px solid #d4d6d8;margin:0 2px -9px 1px;cursor:default}.SS-PageSelector-TempHolder{clear:both;background:#000;border-left:1px solid #ccc;border-right:1px solid #ccc;padding:0 0 8px 0}.SS-PageSelector-TempHolder select{float:unset!important;width:146px;height:25px;background:#ffdd00;color:#000;margin-top:-5px}.Page{display:none;min-height:312px;width:450px;border-radius:0;padding:0;background-color:#d4d6d8}.page-show{display:inline-block!important}.row{clear:both}.paragraph{width:312px;text-align:justify;font-size:16px}.mode-title{font-size:20px;font-weight:bold;text-align:center;color:#1492ac}.mode-text{font-size:16px;text-align:center;color:#1492ac}.blink{animation:blinking 0.8s infinite}.blink-light{animation:blinking-light 1.0s infinite}.e-txt{float:left}.checkbox{float:left;accent-color:#047fa6;background-color:#d1d3d1;width:20px;height:20px;margin:0;box-sizing:border-box}.checkbox.c-red{accent-color:#f5263f}label{float:left;padding:6px 0;color:#047fa6}label div{float:left;margin:0 6px}select,input{float:right;text-align:center;border:1px solid #444;box-sizing:border-box;font-size:15px;font-weight:bold;width:180px;height:27px;border-radius:5px;background-color:#acc5ed}input:autofill,select:autofill{box-shadow:0 0 0 1000px #acc5ed inset!important;text-fill-color:black!important}input:-webkit-autofill,select:-webkit-autofill{-webkit-box-shadow:0 0 0 1000px #acc5ed inset!important;-webkit-text-fill-color:black!important}.right{float:right!important}.left{float:left!important}.float-none{float:none!important}.key{float:right;background-color:#39c600;padding:4px 8px;margin:6px 0;height:35px;width:130px;font-size:16px;font-weight:bold;color:#FFFFFF;border-radius:40px;border:2px solid #FFFFFF;cursor:pointer}.key:hover{border-color:#000000;color:#000000;transition:border-color,color 0.35s ease-in}.key:active{background-color:#a8a8a5}.key.left{float:left!important}.key.right{float:right!important}.bt-med{background-color:#7cabc7;color:#FFFFFF;padding:3px 5px;margin:1px 0;height:28px;width:127px;font-size:16px;border-radius:5px;border:1px solid #FFFFFF;cursor:pointer}.bt-med:hover{border-color:#000000;color:#000000;transition:border-color,color 0.35s ease-in}.bt-med:active{background-color:#989895}.disabled,.disabled .mode-title{color:#666!important;cursor:not-allowed}.disabled input,.disabled select,.disabled label,.disabled .key,.disabled .key:hover{color:#666!important;background-color:#bcbfc3!important;cursor:not-allowed;border:1px solid #000000}.disabled input:-webkit-autofill,.disabled select:-webkit-autofill{-webkit-box-shadow:0 0 0 1000px #bcbfc3 inset!important;-webkit-text-fill-color:#666!important}.hold{color:#1e6c00;background:#bee278;direction:ltr!important}#SG_Page_Language .top-menu{font-size:18px!important;color:#888}#SG_Page_Wait #SG_wait_progress{font-size:20px;color:#5eabc6;word-spacing:-4px}#SG_Page_Selection .key{width:320px;font-size:15px}#SN_Page_Restore p{text-align:justify}#SN_Page_Restore .text-ok{background-color:#84c058!important;color:#d5ecce;padding:1px 6px;border-radius:5px}#SN_Page_Restore .text-cancel{background-color:#eb5868!important;color:#f0d8d8;padding:1px 6px;border-radius:5px}#SN_Page_Restore .warning{font-size:32px;font-weight:bold;text-align:center;color:#ff0000}.hide,.template{display:none}#SU_Page_Update form{background:#fff8;max-width:370px;height:180px;margin:15px auto 0 auto;padding:10px;border-radius:10px;text-align:center}#SU_Page_Update .bt-site{width:198px;border-radius:25px;border:1px solid #999;padding:3px;background:#d3d3d3;font-size:12px;cursor:pointer}#SU_file-input,#SU_Page_Update input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}#SU_file-input{overflow:hidden;padding:0 7px;color:#fff;background:#5685b0;border:2px solid #fff;border-radius:9px;line-height:44px;cursor:pointer}#SU_prg{font-size:13px}#SU_prgbar{background-color:#fff;border-radius:10px;margin-top:35px}#SU_bar{background-color:#3498db;width:0%;height:10px;border-radius:10px;margin-top:45px}#ST_Page_KeyColors .keypanel{direction:ltr;background:#000;border:15px solid #000;border-radius:9px;text-align:center;cursor:pointer}#ST_Page_KeyColors .keypanel td{color:#000;border-radius:5px;padding:1px;font-size:16px;font-weight:bold}#ST_Page_KeyColors .key{width:90px}#SS_Page_AddressRelay input{width:54px;float:right;margin:1px 0 0 4px}#SS_Page_AddressRelay *:disabled,#SS_Page_Scenario *:disabled{background:#bdbfc1;border-color:#fff;color:#777}#SS_Page_AddressScenekey input{width:54px;float:right;margin:1px 0 0 4px}#SS_Page_AddressScenekey .usemode{color:#1e6c00;background:#bee278}#SS_Page_AddressScenekey *:disabled{background:#bdbfc1;border-color:#fff;color:#777}#SS_Page_Scenario select.used{border-color:#000!important;color:#000!important}#SS_Page_SettingBackup p{border-radius:7px;padding:8px 5px;margin:1px 0}.p-t-10p{padding-top:10%}.w-100p{width:100%!important}.f-normal{font-weight:normal!important}.f-bold{font-weight:bold!important}.t-center{text-align:center!important}.t-justify{text-align:justify!important}.t-left{text-align:left!important}.t-right{text-align:right!important}.hide-disabled:disabled{visibility:hidden}.wrong{color:#ac0404!important;background:#ffa5a5!important}.address-double1{background:#feec6b!important}.address-double2{background:#ffbc6b!important}.c-red{color:#e80e1c!important}.c-green{color:#3fbd00!important}.c-white{color:#ffffff!important}.c-blue{color:#498db8!important}.c-gray{color:#a0a6aa!important}.green{background-color:#7dc032}.greenlight{background-color:#9bc869}.greendark{background-color:#59a206}.blue{background-color:#498db8}.red{background-color:#f03848}.redbright{background-color:#fd6d6d}.redlight{background-color:#fb9696}.lang-fa{direction:rtl}.lang-fa .top-menu{text-align:right}.lang-fa .checkbox,.lang-fa label{float:right}.lang-fa label div{float:right;margin-left:5px}.lang-fa input,.lang-fa select{float:left}.lang-fa .e-txt{float:right}.lang-fa .key.right{float:left!important}.lang-fa .key.left{float:right!important}.direction-ltr{direction:ltr!important}body.vatra{background-color:#7f7c72}.vatra .div-footer{color:#e0e0e0;background-color:#8e8e8e;border:1px solid #b6b9bc;color:#f9f9f9;background-color:#acaba8;border:1px solid #d4d6d8}.vatra .Page{background-color:#cccdcf}.vatra .div-title{background-color:#a40521}.vatra .top-menu{background-color:#a40521}.vatra .top-menu span{color:#b76060;background-color:#84031a;border:1px solid #d45252}.vatra .top-menu span:hover{color:#ff7070}.vatra .top-menu span.menu-active{color:#ce0024;border-color:#cccdcf;background:#cccdcf;text-shadow:0px 0px 6px #fff}.vatra .mode-title{color:#ffffff;text-shadow:2px 1px 7px #000,0px 0px 2px #000}.vatra .disabled .mode-title{text-shadow:none}.vatra select,.vatra input{background-color:#edede8}.vatra .key.blue{background-color:#ece3be;color:#966e02;border-color:#cfb168}.vatra .key.blue:hover{background-color:#969592;c";
    const char  PROGMEM_CSS2[] PROGMEM = "olor:#fff;border-color:#fff}.vatra .SS-PageSelector-TempHolder{background:#a40521}.vatra .SS-PageSelector-TempHolder select{background:#ffb600!important;color:#000;border-color:#000}";
    
    const char* const AirNginClient::PROGMEM_CSS[] PROGMEM = {
      PROGMEM_CSS1,
      PROGMEM_CSS2
    };
    const byte AirNginClient::maxPROGMEM_CSS = 2;
    
    
    
/*---- Page.html ----*/
const char PROGMEM_HTML1[] PROGMEM = "<script id='TmP123'>try{(function(v){var i,k,c=String.fromCharCode,cq=c(34),cf=c(12),a=(\" class='\\f</option> <option value='\\f' onclick='S\\f> </div> <\\fcenter\\frow p-t-\\f</span> <\\fbutton\\f id='SN_\\f_Show()'>\\f value='\\f' type='text'\\f> <div\\f> <span\\f' maxlength=\\felect\\fkey blue\\f&nbsp;\\fPage lang-\\fe-txt'>\\froject\\fmode-title'>\\f menu-active'>\\fN.Finish\\f id='S\\f0'> <\\finput\\f right\\ftop-menu'\\f</option> <\\fModem\\fw-320'\\fion_Click(\\fUpdate\\fPage_\\f left\\f_Next()'>\\fتنظیمات \\fion_Backed()'>\\fcheckbox' name\\fN.Mqtt\\fBack()'>Previous</\\f</span\\fyle='color:black'>Current\\fvice Needs Be An Extender\\fve_modes' checked='checke\\fthod='POST' action='/firm\\frcuit' readonly disabled \\fChangeFile(this)'> <label\\frow'> <\\f Settings</\\fkey green\\f><div>اعمال تغییرات <B>\\f</div> <\\fw-100p t-left f-normal\\ftype='\\fmultipart/form-data'\\fhide'\\fhange\\fBack()'>قبلی</\\fRestore\\ftitle\\f5'> <\\f One Type Of List'\\fodem_\\fkey red \\fTool_LanguageSet(\\f Password : \\fhannel\\f_MemoryClean()'>\\fwarning blink'>\\f><div>Set To Th\\f Yes If This De\\fانتخاب \\f Device\\fپیشفرض کارخانه\\fپروژه\\fپروتکل\\flabel>\\f_btprev'\\f_btnext'\\fU_file\\f</b> را بزنید \\f)'>Close Panel\\f w-90\\f Vresion: 1.00\\f Configur\\fشبکه مش\\fاینترنت اشیا\\fsubmit'\\frow f-1\\fkey red\").split(cf);for(i=91;i>=0;i--){k=cf+c(i+35);if(v.indexOf(k)>=0)v=v.split(k).join(a[i]);}document.body.innerHTML=v;})(\"<\f'\f#panel-box'\f/\f#div-\f`'> <img id='logo' src=''></img\f&div\f#div-body'\f/\f;G_\fELanguage'\f#Page page-show'> <\f'\f#p-l-130 \f?>\fm\fyation <br> <b style='color:#555'>پنل \fHو پیکربندی</b> </\f'> <\f'\f#w-420'\f/\f#\f}6'> برای ادامه ، زبان مورد نظر خود را \flنمایید </div\f/\f#\f}4 p-t-10'> S\f2 One Of The Below Languages To Continue \fX/\f'> <\f'\f#\fB\f/\f#\f(1\f<\f*\f#key\fF blue' onclick='\fe1)'>English</\f*> <\f*\f#key\f> blue' onclick='\fe0)'>فارسی</\f*\f&/\f'> <\f'\f#w-420'\f/\f#\f}6 p-t-20'> با زدن دکمه قرمز زیر میتوانید از \fHخارج شوید </div\f/\f#\f}4 p-t-10'> You Can Close This\fyation By The Below Red-Button \fX/\f'> <\f'\f#\fB\f/\f#\f(1\f<\f*\f#\fdw-100p\f%G.Exit(2\fv \f4/\f4 خروج از تنظیمات</\f*\f&/\f'\f&div\f;G_\fEWait'\f#Page'> <\f'\f#w-420 p-t-40'\f/\f;G_wait_msg'\f#\f(10p'> Please Wait , To Complete The Operation </div\f/\f#\fTp\f;G_wait_progress'></p\f&/\f'\f&div\f;G_\fES\f2ion'\f#\f5en'> <\f'\f#\fB\f/\f#\f(2\f<\f'\f#\f8Choose Your Desired Panel</\f'\f&\f'\f#\f(4\f<\f*\f#\f3\f%G.S\f2\fC3)'>Network\fU\f*> </\f'> <\f'\f#\f(2\f<\f*\f#\f3\f%G.S\f2\fC4)'>Firmware \fD</\f*> </\f'> <\f'\f#\f(2\f<\f*\f#\fdw-150\fF\f%G.S\f2\fC8\fv</\f*> <\f*\f#\fdw-150\f>\f%G.S\f2\fC9)'>Reboot</\f*> </\f'> </\f'\f&div\f;G_\fES\f2ion'\f#\f5fa'> <\f'\f#\fB\f/\f#\f(2\f<\f'\f#\f8پنل مورد نظرتان را \flکنید</\f'\f&\f'\f#\f(4\f<\f*\f#\f3\f%G.S\f2\fC3)'>\fHشبکه و \fo</\f*> </\f'> <\f'\f#\f(2\f<\f*\f#\f3\f%G.S\f2\fC4)'>آپدیت نسخه فریمور</\f*> </\f'> <\f'\f#\f(2\f<\f*\f#\fdw-150\fF\f%G.S\f2\fC8)'>بستن این پنل</\f*> <\f*\f#\fdw-150\f>\f%G.S\f2\fC9)'>راه اندازی مجدد</\f*> </\f'> </\f'\f&div\f+\fEP\f7'\f#\f5en'\f/\f#\f?\f0\f#w-80\f9P\f7\f)span\f#w-75\f%N.\fA\f,\fA\f)span\f#w-65\f%\fK\f,Mqtt\f)span\f#w-70\f%\f:\f,Save\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8P\f7\fU\f'\f&div\f#\f(15 \f\\\\ \f0\f#\f6Connection Type :\f)s\f2\f+p\f7_ci\fR\f`='S\f2\fb\f-iot' onc\f]='SN.P\f7_C\f]()'> <option\f-iot' s\f2ed='s\f2ed' \f`='IOT (Internet Of Things)'>IOT\f@/s\f2\f&div\f#\f(10 \f\\\\\f0\f#\f6Is Extender :\f)s\f2\f+p\f7_extender' \f`='S\f2\fk\fO'\f-2' onc\f]='SN.P\f7_C\f]()'> <option\f-2' \f`='Cloud Support and Response as a\fm Only'>Cloud / No\f@/s\f2\f&div\f#\f(10'\f0\f#\f6P\f7\f4-\f4ID : \f4\f)\f=\f+p\f7_code\f. maxlength=20\f-'\f&div\f#\f(10'\f0\f#\f6Encryption\f4-\f4Key : \f4\f)\f=\f+p\f7_enckey\f. maxlength=16\f-'\f&div\f#\f(2\f<\f*\f#\f3\f>\f%N.P\f7\fGNext</\f*> <\f*\f#\f3\fF\f%G.S\f2\fIBack</\f*\f&/\f'\f&div\f+\fEMesh'\f#\f5en \f\\\\\f/\f#\f?\f0\f#w-80\f%N.P\f7\f,P\f7\f)span\f#w-65\f9Mesh\f)span\f#w-75\f%N.\fA\f,\fA\f)span\f#w-65\f%\fK\f,Mqtt\f)span\f#w-70\f%\f:\f,Save\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8WiFi Mesh\fU\f'\f&div\f#\f(25'\f0\f#\f6Mesh C\fg :\f)s\f2\f+mesh_c\fg' \f`='Use a c\fg that has lowest noise according to the other Wifis' > <option\f-1'>1\f$2'>2\f$3'>3\f$4'>4\f$5'>5\f$6'>6\f$7'>7\f$8'>8\f$9' s\f2ed='s\f2ed'>9\f$10'>10\f$11'>11\f@/s\f2\f&div\f#\f(10'\f0\f#\f6Mesh \f4 Name \f4: \f4\f)\f=\f+mesh_name\f.\f-\f16\f&div\f#\f(10'\f0\f#\f6Mesh\ff\f4\f)\f=\f+mesh_pass\f.\f-\f116\f&div\f#\f(2\f<\f*\f+mesh\fs\f#\f3\f>\f%N.Mesh\fGNext</\f*> <\f*\f+mesh\fr\f#\f3\fF\f%N.Mesh_\fL\f*\f&/\f'\f&div\f+\fE\fA'\f#\f5en'\f/\f#\f?\f0\f#w-80\f%N.P\f7\f,P\f7\f)span\f#w-75\f9\fA\f)span\f#w-65\f%\fK\f,Mqtt\f)span\f#w-70\f%\f:\f,Save\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8\fA\fU\f'\f&div\f#\f(25'\f0\f#\f6\fA C\fg :\f)s\f2\f+m\fcc\fg' > <option\f-0' s\f2ed='s\f2ed'>Auto\f$1'>1\f$2'>2\f$3'>3\f$4'>4\f$5'>5\f$6'>6\f$7'>7\f$8'>8\f$9'>9\f$10'>10\f$11'>11\f@/s\f2\f&div\f#\f(10'\f0\f#\f6\fA SSID : \f4\f)\f=\f+m\fcssid\f.\f-\f132\f&div\f#\f(10'\f0\f#\f6\fA\ff\f4\f)\f=\f+m\fcpass\f.\f-\f163\f&div\f#\f(2\f<\f*\f+m\fcbtnext'\f#\f3\f>\f%N.\fA\fGNext</\f*> <\f*\f+m\fcbtprev'\f#\f3\fF\f%N.\fA_\fL\f*\f&/\f'\f&div\f+\fEMqtt'\f#\f5en'\f/\f#\f?\f0\f#w-80\f%N.P\f7\f,P\f7\f)span\f#w-75\f%N.\fA\f,\fA\f)span\f#w-65\f9Mqtt\f)span\f#w-70\f%\f:\f,Save\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8MQTT\fU\f'\f&div\f#\f(25'\f0\f#\f6Mqtt Username : \f4\f)\f=\f+mqtt_user\f.\f-\f116\f&div\f#\f(10'\f0\f#\f6Mqtt\ff\f4\f)\f=\f+mqtt_pass\f.\f-\f116\f&div\f#\f(10'\f0\f#e-txt\f1100>Mqtt Broker URL: \f4\fM\f&div\f#\f(\fa\f=\f+mqtt_broker'\f#\fY\f.\f-'\f&div\f#\f(2\f<\f*\f+mqtt\fs\f#\f3\f>\f%\fK\fGNext</\f*> <\f*\f+mqtt\fr\f#\f3\fF\f%\fK_\fL\f*\f&/\f'\f&div\f+\fEFinish'\f#\f5en'\f/\f#\f?\f0\f#w-80\f%N.P\f7\f,P\f7\f)span\f#w-75\f%N.\fA\f,\fA\f)span\f#w-65\f%\fK\f,Mqtt\f)span\f#w-70\f9Save\fM\f&\f'\f#paragraph p-t-10'> S\f2 one of the below options, then Save to apply your c\f]s to the device(s) </\f'> <\f'\f#\fB\f/\f#row m-t-1\f<\fq<\f= \fZradio'\f#\fJ='SN_sa\fPd'\f-one'\fjis <B>Device Only</B></div></label\f&div\f#row m-t-10 \f\\\\> <\fq<\f= \fZradio'\f#\fJ='SN_save_modes'\f-all' \fjese <B>Mesh-Group\fms</B></div></label\f&/\f'> <\f'\f#w-400'\f/\f#\f(1\f<\f*\f#\fV\fF\fw\f%\f:_Import()'>Import</\f*> <\f*\f#\f3\fF m-l-5\fw\f%\f:_Export()'>Export</\f*> <\f*\f#\fV\f>\fw\f%\f:_Save()'>Save</\f*> <\f*\f#\f~\f> m-r-5\fw\f%G.Any\fECancel()'>Cancel</\f*\f&div\f#\f(2\f<\f*\f#\f3 w-100p\f%\f:_\f_()'>\f_ To Factory</\f*\f&/\f'\f&div\f+\fE\f_'\f#\f5en'> <\f'\f#w-350'\f/\f#\f(3\f<\f'\f#\fiWarning</\f'\f&div\f#\f(1\f<p>If You Press <b\f#text-ok'>OK</b> , Your\fm(s) Will Be\fyed To Factory Settings After Restart</p\f&div\f#\f(\fap>Else Press <b\f#text-cancel'>Cancel</b> , To Return To The Previous Page, Without Any C\f]s</p\f&/\f'> <\f'\f#\fB\f/\f#\f(1\fa\f*\f#\fV\f>\f%N.\f__Ok()'>OK</\f*> <\f*\f#\f~\fF\f%\f:\f,Cancel</\f*\f&/\f'\f&div\f+\fEP\f7'\f#\f5fa'\f/\f#\f?\f0\f#w-65\f9\fo\f)span\f#w-65\f%N.\fA\f,مودم\f)span\f#w-70\f%\fK\f,\fp\f)span\f#w-70\f%\f:\f,ذخیره\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8\fH\fo</\f'\f&div\f#\f(15 \f\\\\\f0\f#\f6نوع ارتباط ابزار :\f)s\f2\f+p\f7_ci\fR\f`='S\f2\fb\f-iot' onc\f]='SN.P\f7_C\f]()'> <option\f-iot' s\f2ed='s\f2ed' \f`='آی او تی (\f{ از طریق مودم)'>\f{\f@/s\f2\f&div\f#\f(10 \f\\\\\f0\f#\f6گسترش دهنده :\f)s\f2\f+p\f7_extender' \f`='S\f2\fk\fO'\f-2' onc\f]='SN.P\f7_C\f]()'> <option\f-2' \f`='پاسخگویی بعنوان ابزار معمولی / سرور ابری'>خیر / سرور ابری\f@/s\f2\f&div\f#\f(10'\f0\f#\f6شناسه\f4\f4\fo : \f4\f)\f=\f+p\f7_code\f.\f-\f120\f&div\f#\f(10'\f0\f#\f6کلید رمزنگاری : \f4\f)\f=\f+p\f7_enckey\f.\f-\f116\f&div\f#\f(2\f<\f*\f#\f3\f>\f%N.P\f7\fGبعدی</\f*> <\f*\f#\f3\fF\f%G.S\f2\fIبازگشت</\f*\f&/\f'\f&div\f+\fEMesh'\f#\f5fa \f\\\\\f/\f#\f?\f0\f#w-65\f%N.P\f7\f,\fo\f)span\f#w-100\f9\fz\f)span\f#w-65\f%N.\fA\f,مودم\f)span\f#w-70\f%\fK\f,\fp\f)span\f#w-70\f%\f:\f,ذخیره\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8\fH\fz</\f'\f&div\f#\f(25'\f0\f#\f6کانال\f4 فرکانس مش :\f)s\f2\f+mesh_c\fg' \f`='از یک کانال فرکانسی که در آن کمترین نویز سایر وای فای ها هست استفاده کنید' > <option\f-1'>1\f$2'>2\f$3'>3\f$4'>4\f$5'>5\f$6'>6\f$7'>7\f$8'>8\f$9' s\f2ed='s\f2ed'>9\f$10'>10\f$11'>11\f@/s\f2\f&div\f#\f(10'\f0\f#\f6شناسه \fz : \f4\f)\f=\f+me";
const char PROGMEM_HTML2[] PROGMEM = "sh_name\f.\f-\f16\f&div\f#\f(10'\f0\f#\f6رمزعبور \fz : \f4\f)\f=\f+mesh_pass\f.\f-\f116\f&div\f#\f(2\f<\f*\f+mesh\fs\f#\f3\f>\f%N.Mesh\fGبعدی</\f*> <\f*\f+mesh\fr\f#\f3\fF\f%N.Mesh_\f^\f*\f&/\f'\f&div\f+\fE\fA'\f#\f5fa'\f/\f#\f?\f0\f#w-65\f%N.P\f7\f,\fo\f)span\f#w-65\f9مودم\f)span\f#w-70\f%\fK\f,\fp\f)span\f#w-70\f%\f:\f,ذخیره\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8\fHمودم</\f'\f&div\f#\f(25'\f0\f#\f6\fl\f4 کانال : \f)s\f2\f+m\fcc\fg'> <option\f-0' s\f2ed='s\f2ed'>خودکار\f$1'>1\f$2'>2\f$3'>3\f$4'>4\f$5'>5\f$6'>6\f$7'>7\f$8'>8\f$9'>9\f$10'>10\f$11'>11\f@/s\f2\f&div\f#\f(10'\f0\f#\f6شناسه مودم : \f4\f)\f=\f+m\fcssid\f.\f-\f132\f&div\f#\f(10'\f0\f#\f6رمز عبور مودم : \f4\f)\f=\f+m\fcpass\f.\f-\f163\f&div\f#\f(2\f<\f*\f+m\fcbtnext'\f#\f3\f>\f%N.\fA\fGبعدی</\f*> <\f*\f+m\fcbtprev'\f#\f3\fF\f%N.\fA_\f^\f*\f&/\f'\f&div\f+\fEMqtt'\f#\f5fa'\f/\f#\f?\f0\f#w-65\f%N.P\f7\f,\fo\f)span\f#w-65\f%N.\fA\f,مودم\f)span\f#w-70\f9\fp\f)span\f#w-70\f%\f:\f,ذخیره\fM\f&\f'\f#\fB\f/\f#\fT\f'\f#\f8\fH\fp MQTT</\f'\f&div\f#\f(25'\f0\f#\f6نام کاربری اتصال : \f4\f)\f=\f+mqtt_user\f.\f-\f116\f&div\f#\f(10'\f0\f#\f6رمز\f4 عبور\f4 اتصال\f4\f4: \f4\f)\f=\f+mqtt_pass\f.\f-\f116\f&div\f#\f(10'\f0\f#\f6آدرس سرور بروکر: \f4\fM\f&div\f#\f(\fa\f=\f+mqtt_broker'\f#\fY\f.\f-\f1100\f&div\f#\f(2\f<\f*\f+mqtt\fs\f#\f3\f>\f%\fK\fGبعدی</\f*> <\f*\f+mqtt\fr\f#\f3\fF\f%\fK_\f^\f*\f&/\f'\f&div\f+\fEFinish'\f#\f5fa'\f/\f#\f?\f0\f#w-65\f%N.P\f7\f,\fo\f)span\f#w-65\f%N.\fA\f,مودم\f)span\f#w-70\f%\fK\f,\fp\f)span\f#w-70\f9ذخیره\fM\f&\f'\f#paragraph f-14 p-t-10'> یکی از دو گزینه زیر را جهت اعمال تغییرات \flکنید </\f'> <\f'\f#\fB\f/\f#row m-t-1\f<\fq<\f= \fZradio'\f#\fJ='SN_sa\fPd'\f-one'\fWفقط برای همین ابزار</B></div></label\f&div\f#row m-t-10 \f\\\\> <\fq<\f= \fZradio'\f#\fJ='SN_save_modes'\f-all' \fWبه همه ابزارهای متصل در مش</B></div></label\f&/\f'> <\f'\f#w-400'\f/\f#\f(2\fa\f*\f#\fV\f>\fw\f%\f:_Import()'>بارگذاری</\f*> <\f*\f#\f3\f> m-l-5\fw\f%\f:_Export()'>استخراج</\f*> <\f*\f#\fV\fF\fw\f%\f:_Save()'>ذخیره</\f*> <\f*\f#\f~\fF m-r-5\fw\f%G.Any\fECancel()'>انصراف</\f*\f&div\f#\f(1\fa\f*\f#\f3 w-100p\f%\f:_\f_()'>ریست به \fH\fn</\f*\f&/\f'\f&div\f+\fE\f_'\f#\f5fa'> <\f'\f#w-350'\f/\f#\f(2\fa\f'\f#\fiهشدار</\f'\f&div\f#\f(2\f<p>اگر دکمه <b\f#text-ok'>تایید\fu، \fH\fn بارگذاری شده و سپس ابزار(ها) ریست میشوند</p\f&div\f#\f(\fap>درغیر اینصورت دکمه <b\f#text-cancel'>انصراف\fuتا بدون اعمال هیچگونه تغییری به صفحه قبلی باز گردید</p\f&/\f'> <\f'\f#\fB\f/\f#\f(1\fa\f*\f#\fV\f>\f%N.\f__Ok()'>تایید</\f*> <\f*\f#\f~\fF\f%\f:\f,انصراف</\f*\f&/\f'\f&div\f;U_\fE\fD'\f#\f5en'> <\f'\f#w-400'\f/\f#\f(2\f<\f'\f#\f8\fD Firmware Panel</\f'\f&\f'\f#\f(\faform me\fQware' enc\fZ\f[ > <\f'\f;U_cver' st\fN\fx</\f'> <\f'\f#\f(10'\f/\f#bt-site\f%U.\fD_GetFile()'>Download From Web\fX/\f'> <\f'\f#\f(2'> <\f=\f;U_bt_\f| \fZ\f|\f#\f\\\\\f-\fD'> <\f= \fZfile'\f#\f\\\\ name='update'\f;\ft' onc\f]='SU.\fD_\fS\f;\ft-\f=' for='S\ft'>S\f2 \fDr File...</\fq <br\f/\f;U_prgbar'\f/\f;U_bar'></div\f&div\f;U_prg'>\fX/\f'> </form> </\f'> </\f'> <\f'\f#\f(10 w-38\f<\f*\f#key\f> green w-100\f%U.\fD_Apply()'>\fD</\f*> <\f*\f#key\f> red w-150 m-r-14\f%U.\fD\fhClean Memory</\f*> <\f*\f#key\fF blue w-100\f%G.S\f2\fIBack</\f*> </\f'\f&div\f;U_\fE\fD'\f#\f5fa'> <\f'\f#w-400'\f/\f#\f(2\f<\f'\f#\f8پنل ارتقاء فریمور</\f'\f&\f'\f#\f(\faform me\fQware' enc\fZ\f[> <\f'\f;U_cver' st\fN\fx</\f'> <\f'\f#\f(10'\f/\f#bt-site\f%U.\fD_GetFile()'>دانلود از طریق اینترنت\fX/\f'> <\f'\f#\f(2'> <\f=\f;U_bt_\f| \fZ\f|\f#\f\\\\\f-\fD'> <\f= \fZfile'\f#\f\\\\ name='update'\f;\ft' onc\f]='SU.\fD_\fS\f;\ft-\f=' for='S\ft'>\flفایل آپدیت...</\fq <br\f/\f;U_prgbar'\f/\f;U_bar'></div\f&div\f;U_prg'>\fX/\f'> </form> </\f'> </\f'> <\f'\f#\f(10 w-37\f<\f*\f#key\f> green w-110\f%U.\fD_Apply()'>انجام آپدیت</\f*> <\f*\f#key\f> red w-120 m-l-14\f%U.\fD\fhتخلیه حافظه</\f*> <\f*\f#key\fF blue w-110\f%G.S\f2\fIبازگشت</\f*> </\f'\f&/div\f/\f#div-footer'> SN: 0000000000 /\fm Details \fX/\f'>\");document.getElementById('TmP123').outerHTML=''}catch(x){}</script>";

const char* const AirNginClient::PROGMEM_HTML[] PROGMEM = {
  PROGMEM_HTML1,
  PROGMEM_HTML2
};
const byte AirNginClient::maxPROGMEM_HTML = 2;

