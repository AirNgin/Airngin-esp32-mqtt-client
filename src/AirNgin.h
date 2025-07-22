#pragma once
/*
 airngin.h - A simple client for Connect To AirNgin Broker.
  AirNgin VERSION : 0.1.11 Beta
  https://airngin.com
*/
#ifndef airngin_h
#define airngin_h

#define SOFTWARE_VERSION "1.0.0"

#define DVC_DEFAULTPASS "00000000"  // DONT CHANGE THIS
#define SOFTWARE_DATE "1404.04.31"



#include <Arduino.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <SPIFFS.h>
#include <WString.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPUpdate.h>
#include <ESPmDNS.h>
#include <ESPping.h>


#define TASK_PRIORITY_HIGH 1       // 5
#define TASK_PRIORITY_MID 2        // 3
#define TASK_PRIORITY_LOW 3        // 2
#define TASK_STACKSIZE_SMALL 2048  // 1024
#define TASK_STACKSIZE_MEDIUM 4096 // 4096
#define TASK_STACKSIZE_LARGE 8192  // 8192
#define TASK_CORE_0 0
#define TASK_CORE_1 1
#define TASK_LOOP_DELAY 20


#define BYTE_1 1
#define BYTE_2 2
#define BYTE_3 3
#define BYTE_10 10
#define BYTE_11 11
#define BYTE_12 12
#define BYTE_14 14
#define BYTE_15 15
#define BYTE_16 16
#define BYTE_17 17
#define BYTE_20 20
#define BYTE_32 32
#define BYTE_37 37
#define BYTE_38 38
#define BYTE_100 100
#define BYTE_400 400

#define MIN_20 1200
#define MIN_15 900
#define MIN_10 600
#define MIN_5 300
#define MIN_4 240
#define MIN_3 180
#define MIN_2 120
#define MIN_1 60
#define SEC_90 90
#define SEC_30 30
#define SEC_20 20
#define SEC_10 10
#define SEC_5 5
#define SEC_4 4
#define SEC_3 3
#define SEC_2 2
#define SEC_1 1
#define TIMER_NEED_RUN 0
#define TIMER_JOB_DONE -1
#define TIMER_EXPIRED_FOREVER -2




    
#define MESSAGE_CLOUD_REBOOT 1001
#define MESSAGE_CLOUD_MODEM_WAKE 9000
#define MESSAGE_CLOUD_MODEM_OUT 9001
#define MESSAGE_CLOUD_GSM_SIGNAL_UNKNOWN 8001
#define MESSAGE_CLOUD_GSM_SIGNAL_OUT 8002
#define MESSAGE_CLOUD_GSM_SIGNAL_WAKE 8003
#define MESSAGE_CLOUD_GSM_SIGNAL_MID 8004
#define MESSAGE_CLOUD_GSM_SIGNAL_GOOD 8005
#define MESSAGE_CLOUD_GSM_SIGNAL_BEST 8006
#define MESSAGE_CLOUD_GSM_QUALITY_UNKNOWN 8010
#define MESSAGE_CLOUD_GSM_QUALITY_GOOD 8011
#define MESSAGE_CLOUD_GSM_QUALITY_MID 8012
#define MESSAGE_CLOUD_GSM_QUALITY_WAKE 8013
#define MESSAGE_CLOUD_GSM_QUALITY_BAD 8014
#define MESSAGE_CLOUD_GSM_WALET_0_TO_5 8020
#define MESSAGE_CLOUD_GSM_WALET_5_TO_10 8021
#define MESSAGE_CLOUD_GSM_WALET_10_TO_20 8022
#define MESSAGE_CLOUD_GSM_WALET_20_TO_MORE 8023
#define TX_MESSAGE_CLOUD_REBOOT "1001"
#define TX_MESSAGE_CLOUD_MODEM_WAKE "9000"
#define TX_MESSAGE_CLOUD_MODEM_OUT "9001"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_UNKNOWN "8001"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_OUT "8002"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_WAKE "8003"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_MID "8004"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_GOOD "8005"
#define TX_MESSAGE_CLOUD_GSM_SIGNAL_BEST "8006"
#define TX_MESSAGE_CLOUD_GSM_QUALITY_UNKNOWN "8010"
#define TX_MESSAGE_CLOUD_GSM_QUALITY_GOOD "8011"
#define TX_MESSAGE_CLOUD_GSM_QUALITY_MID "8012"
#define TX_MESSAGE_CLOUD_GSM_QUALITY_WAKE "8013"
#define TX_MESSAGE_CLOUD_GSM_QUALITY_BAD "8014"
#define TX_MESSAGE_CLOUD_GSM_WALET_0_TO_5 "8020"
#define TX_MESSAGE_CLOUD_GSM_WALET_5_TO_10 "8021"
#define TX_MESSAGE_CLOUD_GSM_WALET_10_TO_20 "8022"
#define TX_MESSAGE_CLOUD_GSM_WALET_20_TO_MORE "8023"


//===========================================================================================
//                                           EPPROM
//===========================================================================================
#define EEPROM_SIZE 4000

#define EP_MEMORY_NOT_CLEANABLE 84

#define EP_RGBTYPE (EP_MEMORY_NOT_CLEANABLE + 1) // (85)
#define EP_TESTMODE (EP_RGBTYPE + 1)             // (86)
#define EP_BANNED (EP_TESTMODE + 1)              // (87)
#define EP_CONFIGURED (EP_BANNED + 1)            // (88)
#define EP_STARTMODE (EP_CONFIGURED + 1)         // (89)
#define EP_SERIAL_S (EP_STARTMODE + 1)           // (90-99)
#define EP_SERIAL_E (EP_SERIAL_S + BYTE_10 - 1)  // (90-99)
#define EP_MEMORYSTART1 100                      // MEMORY POINT 1 BEGIN :  Cleanable Settings, Clear Begin From Here To End Of EPROM

#define EP_EXTENDER (EP_MEMORYSTART1 + 1) //........................................................... NETWORK
#define EP_CIRCUIT (EP_EXTENDER + 1)
#define EP_PROJECTCODE_S (EP_CIRCUIT + 1)
#define EP_PROJECTCODE_E (EP_PROJECTCODE_S + BYTE_20 - 1)
#define EP_CLIENTID_S (EP_PROJECTCODE_E + 1)
#define EP_CLIENTID_E (EP_CLIENTID_S + BYTE_38 - 1)
#define EP_ENCRYPTIONKEY_S (EP_CLIENTID_E + 1)
#define EP_ENCRYPTIONKEY_E (EP_ENCRYPTIONKEY_S + BYTE_10 - 1)
#define EP_MESHRSSI (EP_ENCRYPTIONKEY_E + 1) //........................................................ NETWORK MESH
#define EP_MESHCHANNEL (EP_MESHRSSI + 1)
#define EP_MESHCHANNEL_OLD (EP_MESHCHANNEL + 1)
#define EP_MESHTYPE (EP_MESHCHANNEL_OLD + 1)
#define EP_MESHNAME_S (EP_MESHTYPE + 1)
#define EP_MESHNAME_E (EP_MESHNAME_S + BYTE_20 - 1)
#define EP_MESHNAME_OLD_S (EP_MESHNAME_E + 1)
#define EP_MESHNAME_OLD_E (EP_MESHNAME_OLD_S + BYTE_20 - 1)
#define EP_MESHPASS_S (EP_MESHNAME_OLD_E + 1)
#define EP_MESHPASS_E (EP_MESHPASS_S + BYTE_20 - 1)
#define EP_MESHPASS_OLD_S (EP_MESHPASS_E + 1)
#define EP_MESHPASS_OLD_E (EP_MESHPASS_OLD_S + BYTE_20 - 1)
#define EP_MQTTUSER_S (EP_MESHPASS_OLD_E + 1) //....................................................... NETWORK MQTT
#define EP_MQTTUSER_E (EP_MQTTUSER_S + BYTE_16 - 1)
#define EP_MQTTPASS_S (EP_MQTTUSER_E + 1)
#define EP_MQTTPASS_E (EP_MQTTPASS_S + BYTE_16 - 1)
#define EP_MQTTBROKER_S (EP_MQTTPASS_E + 1)
#define EP_MQTTBROKER_E (EP_MQTTBROKER_S + BYTE_100 - 1)
#define EP_MODEMCHANNEL (EP_MQTTBROKER_E + 1) //....................................................... NETWORK MODEM
#define EP_MODEMSSID_S (EP_MODEMCHANNEL + 1)
#define EP_MODEMSSID_E (EP_MODEMSSID_S + BYTE_32 - 1)
#define EP_MODEMPASS_S (EP_MODEMSSID_E + 1)
#define EP_MODEMPASS_E (EP_MODEMPASS_S + BYTE_20 - 1)
#define EP_MODEMRSSI (EP_MODEMPASS_E + 1)

#define EP_RESERVE1_S (EP_MODEMRSSI + 1)           // RESERVE S 100
#define EP_RESERVE1_E (EP_RESERVE1_S + BYTE_1 - 1) // RESERVE E 100
#define EP_MEMORYEND1 (EP_RESERVE1_E)              // MEMORY POINT 1 END
#define EP_MEMORYPOINT1H (EP_MEMORYEND1 + 1)       // MEMORY POINT 1 SAVE : to save high-byte of current EProm-Address (to detect if upper EProm-Addresses changed)
#define EP_MEMORYPOINT1L (EP_MEMORYPOINT1H + 1)    // MEMORY POINT 1 SAVE : to save low-byte of current EProm-Address (to detect if upper EProm-Addresses changed)
#define EP_MEMORYSTART2 (EP_MEMORYPOINT1L + 1)     // MEMORY POINT 2 BEGIN : Cleanable Settings, Clear Begin From Here To End Of EPROM


//................................................................................................. END
#define EP_MEMORYEND2 (EP_MEMORYSTART2)  // MEMORY POINT 2 END : Cleanable Settings, ClearFrom Start To Here (End Of EPROM)
#define EP_MEMORYPOINT2H (EP_MEMORYEND2 + 1)    // MEMORY POINT 2 SAVE : To save high-byte of current EProm-Address (to detect if upper EProm-Addresses changed)
#define EP_MEMORYPOINT2L (EP_MEMORYPOINT2H + 1) // MEMORY POINT 2 SAVE : To save low-byte of current EProm-Address (to detect if upper EProm-Addresses changed)


//.......................................... Scenario
#define MAX_SCENARIO 20         // تعداد سناریوها

//===========================================================================================
//                                      PREFERENCES MEMORY
//===========================================================================================
#define FILE_WRITE "w"
#define FILE_APPEND "a"
#define FILE_READ "r"
#define FILE_RW "w+"
#define FILE_RA "r+"

class AirNginClient{

private:


    byte _MqttCon_Steps;
    int _IOT_ModemTimeout = MIN_3;
    int _IOT_MqttTimeout = TIMER_NEED_RUN;
    int _IOT_PingTimeout = MIN_15;
    String _ModemSSID;
    String _ModemPass;
    byte _ModemRSSI;
    byte _ModemChannel;
    String _MqttBroker;
    String _MqttUser;
    String _MqttPass;    
    WiFiClient _WifiClient;
    PubSubClient _MqttObj;
    unsigned long _TimerMqttLoopPrev;
    const unsigned long _MttLoopInterval = 5000;
    String _EncryptionKey;
    String _Circuit;
    String _CloudClientId;
    byte _Banned = 0;
    byte _Configured = 0;
    byte _Extender = 0;

    int _ConfigTimeout = 0;






    bool _IsSerialStarted = false;
    bool _Reboot;

    static AirNginClient *instance;

    unsigned long _TimerSecCurrent, _TimerSecOld = 0;
    int _TimerSecExtra = 0;
    byte _TimerSecDef = 0;
    int _TimerForActions = 0;



    void TimerSec_Refresh(); // timer 
    void Debug_ConsolePrint(String p1);
    void Debug_ConsolePrint(int p1);   // اضافه کردن نسخه int
    void Debug_ConsolePrint(long p1);  // اضافه کردن نسخه long
    void Debug_ConsolePrint(float p1); // اضافه کردن نسخه float
    void Debug_ConsolePrintln(String p1);
    void Debug_ConsolePrintln(int p1);   // اضافه کردن نسخه int
    void Debug_ConsolePrintln(long p1);  // اضافه کردن نسخه long
    void Debug_ConsolePrintln(float p1); // اضافه کردن نسخه float
    void Mqtt__Connect();
    void IOT__WiFiStart();
    void Tools__WifiPower(bool SetToOn);
    void Network_RSSI();
    void Send_RebootAndStatus();
    void Network_Reset();
    bool Network_HelthCheck();
    bool Network_Ping();
    bool Network_Ping(String remote_host);
    static void IOT__Checker(void *param);
    static void IOT__WiFiEvent(WiFiEvent_t event);
    void Tools__Reboot(String Reason);
    void MemoClean(int p_start = EP_MEMORYSTART2, int p_end = EP_MEMORYEND2, bool cleanFiles = true);
    void MemoCheck();
    void Debug__ConsoleBegin();
    static void Mqtt__OnRecieve(char *topic, uint8_t *payload, unsigned int length);
    static void IOT__Call__Loop__MQTT();
    String CA_ToString(char input[], int start = 0, int len = 0);
    void MemoWriteByte(int p_start, byte val);
    byte MemoReadByte(int p_start);
    void Tools__File_ClearAll();
    String Tools__Memory_StrGet(String key);
    bool Tools__Memory_StrSet(String key, String val);
    void MemoWriteString(int p_start, int p_end, String val);
    String MemoReadString(int p_start, int p_end);
    int Tools__StringToInt(String inp);
    byte Tools__StringToByte(String inp);
    void Tools__StringSplit(String inp, char splitter, String outArray[], int sizeOfArray);
    int Tools__StringSplitSize(String inp, char splitter);
    void Tools__CloudJSON_Reload(String typ);
    void Tools__SettingShowInfo();
    void (*onMessageCallback)(char *topic, uint8_t *payload, unsigned int length);  // اشاره‌گر به تابع callback


    


    // Preferences _MemoFile;
    DynamicJsonDocument _JSONDoc{1024};  // 1024 یا هر اندازه‌ای که نیاز دارید

    void Tools__File_JSONLoad(String filename, JsonVariant &jsn, JsonDocument &doc);
    void Tools__File_JSONLoad(String filename, JsonVariant &jsn);
    bool Tools__File_JSONSave(String filename, JsonVariant &jsn);
    bool Tools__File_JSONSave(String filename, String &data);
    String Tools__File_TextRead(const String &filename);
    bool Tools__File_TextWrite(const String &filename, const String &data);

    bool Tools__File_Clear(const String &filename);

    void Tools__SettingRead();


    //===========================================================================================
    //                                       Config / Update
    //===========================================================================================
    WebServer _Server;
    WiFiClientSecure _WiFiClientSecure;

    bool Config__ReceiveJson_ScenarioEdit(JsonDocument doc, bool deleteOnly);
    void Config__ReceiveJson_CommandOperation(JsonVariant &inp, bool needSave, bool needReload);
    void Config__ReceiveJson_ScenarioOperation(JsonVariant &inp, bool needSave, bool needReload);
    static void Config__API_FirmwareUpdater();

    void Config__ResponseJSON(String json, bool needRestart = false);

    String Query_Data();

    static void Config__API_FirmwareResult();
    static void Config__API_GetSerial();
    static void Config__API_Alive();
    void Config__SetJson_Network(JsonDocument doc);
    String Config__SetJson(String data);
    static void Config__API_Set();
    static void Config__API_Get();
    static void Config__File_Logo();
    static void Config__File_Html();
    void Config__Loop();
    String Config__GetJson_Network();
    void Tools__SettingSave();
    void Tools__SettingDefault();
    void EEPROM_Start();
    void SPIFFS_Start();


    void (*onSaveScenario)(JsonDocument &doc);
    void (*onDeleteScenario)(String value);
    void (*onSaveSetting)(String value);
    void (*onDebugger)(String value);
    void (*Message_On_Topic_DeviceToDevice_Callback)(uint8_t *payload);
    void (*Message_On_Topic_ServerToDevice_Callback)(String value);


    
    bool _Update_Trying = false;
    static const String rootCACertificate;
    bool _MqttRcv_IsWorking = false;





public:
    String _StartMode;
    bool _Mqtt_TryConnecting;
    String _SerialCloud;
    String _ProjectCode;



    AirNginClient();
    bool addDataToCloud(String key, String value);
    String getDataFromCloud(String key);
    bool deleteDataFromCloud(String key);
    void begin(String name_of_brand="", String version_of_brand="", String serial_of_device="",bool isDebugEnabled=false);
    void Config__Setup();
    void client_Loop();
    void Mqtt_Send(String scope, String type, String data, String sender, bool offlineSupport=false);
    void Mqtt_Send(String topic, String data, bool offlineSupport=false);
    void MessageCloud__ViaMqtt_NotifyTo_User(int messageCode);
    void MessageCloud__ViaMqtt_NotifyTo_User(String &messageCode);
    void MessageCloud__ViaMqtt_SMSTo_User(int messageCode);
    void MessageCloud__ViaMqtt_SMSTo_User(String &messageCode);
    void MessageCloud__ViaMqtt_SMSTo_Center(int messageCode);
    void MessageCloud__ViaMqtt_SMSTo_Center(String &messageCode);            
    uint32_t Tools__GetChipID();
    void Tools__SetMode(String mode, bool restartNeed);
    long Tools__Random(long a, long b);
    void IOT__FirmwareUpdate(String DownloadLink, String Certificate = "", String UTC="");
    
    void SetClock(String UTC="");
    ~AirNginClient();
    
    void setOnMessageCallback(void (*callback)(char *topic, uint8_t *payload, unsigned int length)) {
        onMessageCallback = callback;
    }

     // متدهای ثبت Callback توسط کاربر
    void setOnSaveScenarioCallback(void (*callback)(JsonDocument &doc)) {
        onSaveScenario = callback;
    }

    void setOnDeleteScenarioCallback(void (*callback)(String value)) {
        onDeleteScenario = callback;
    }

    void setOnSaveSettingCallback(void (*callback)(String value)) {
        onSaveSetting = callback;
    }

    void setOnDebuggerCallback(void (*callback)(String value)) {
        onDebugger = callback;
    }


    void setOnMessage_From_Topic_DeviceToDevice_Callback(void (*callback)(uint8_t *payload)) {
        Message_On_Topic_DeviceToDevice_Callback = callback;
    }

    
    void setOnMessage_From_Topic_ServerToDevice_Callback(void (*callback)(String value)) {
        Message_On_Topic_ServerToDevice_Callback = callback;
    }
    
    


    String brand_name = "AirNgin";
    String brand_version = "1.0.0";
    static const char* const PROGMEM_JS_JQUERY[];
    static const char* const PROGMEM_JS_MAIN[];
    static const char* const PROGMEM_CSS[];
    static const char* const PROGMEM_HTML[];
    static const char* const PROGMEM__JS_MAIN[];
    static const char* const PROGMEM__JS_JQUERY[];

    static const char PROGMEM_LOGO[] PROGMEM;
    static const byte maxPROGMEM_JS_JQUERY;
    static const byte maxPROGMEM_JS_MAIN;
    static const byte maxPROGMEM_CSS;
    static const byte maxPROGMEM_HTML;
    bool isConfigMode = false;
    bool _WiFi_IsConnected;
    bool _WiFi_ConnectWorking;
    bool _MqttCon_IsConnected;
    bool CALL_Global_Mqtt_CALLBACK=false;
    bool isDebugEnabled = false;


};


#endif //airngin_h

















