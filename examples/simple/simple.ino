#include <airngin.h>

#define KEY_OF_CENTER "AIRN" // IT'S PRODUCER CENTER CODE

#define Pushbotton 23 // PIN FOR GO TO Config Panel and AP MODE

#define CALL_Global_Mqtt_CALLBACK true // if is true just call this airnginClient.setOnMessageCallback(myMqttCallback);
                                       // else is false desn't call airnginClient.setOnMessageCallback(myMqttCallback); and call other callback

AirNginClient airnginClient;

String _SerialNo="";

unsigned long _TimerSecCurrent, _TimerSecOld = 0;
byte _TimerSecDef = 0;

byte _TimerKeyPush = 0;

void setup() {

  Serial.begin(9600);
  Tools__SerialBarcodeReload();
  airnginClient.begin("","",_SerialNo);
  airnginClient.setOnMessageCallback(myMqttCallback);
  airnginClient.setOnSaveScenarioCallback(saveScenarioCallback);
  airnginClient.setOnDebuggerCallback(debuggerCallback);
  airnginClient.setOnMessage_From_Topic_DeviceToDevice_Callback(message_From_Topic_DeviceToDevice_Callback);
  airnginClient.setOnMessage_From_Topic_ServerToDevice_Callback(message_From_Topic_ServerToDevice_Callback);
  
  pinMode(Pushbotton, INPUT_PULLUP);
}


void loop() {
    delay(1000);
    airnginClient.client_Loop();
    TimerSec_Refresh();             //read Time
    if(digitalRead(Pushbotton)==LOW){
      _TimerKeyPush += _TimerSecDef;
    }
    else
      _TimerKeyPush = 0;
    if(_TimerKeyPush>=5){   //
      _TimerKeyPush=0;
      if(airnginClient.isConfigMode)
      {
        airnginClient.Tools__SetMode(airnginClient._StartMode == "config_panel" ? "normal" : "config_panel", false);
        airnginClient.isConfigMode=false;
        airnginClient._Mqtt_TryConnecting = false;

      }else{
        airnginClient.Tools__SetMode(airnginClient._StartMode == "config_panel" ? "normal" : "config_panel", false);
        airnginClient.isConfigMode=true;  
        airnginClient._Mqtt_TryConnecting = true;
        airnginClient.Config__Setup();
      } 
    }
}


void Tools__SerialBarcodeReload()
{

    String chip = (String(airnginClient.Tools__GetChipID()) + "0000000").substring(0, 7);
    if (chip == "0000000")
      chip = (String(airnginClient.Tools__Random(1000000, 9999998)) + "0000000").substring(0, 7);
    _SerialNo = KEY_OF_CENTER + ("000" + chip).substring(0, 10);
  

  Serial.println("_SerialNo : " + _SerialNo);
}


void TimerSec_Refresh()
{
  //-------------------------------------------- TIMER
  try
  {
    _TimerSecCurrent = millis();
    _TimerSecDef = ((_TimerSecCurrent - _TimerSecOld) / 1000) & 0xFF;
    if (_TimerSecDef < 0)
      _TimerSecDef = 1;
    if (_TimerSecDef >= 1)
      _TimerSecOld = _TimerSecCurrent;
  }
  catch (...)
  {
  }
}


void myMqttCallback(char *topic, uint8_t *payload, unsigned int length) {

  Serial.println("**************** myMqttCallback ********************************");
    String projectTopic = String(topic);
    int p = projectTopic.indexOf('/');
    if (projectTopic.substring(0, p) != airnginClient._ProjectCode)  // فرض بر این است که متدی برای گرفتن ProjectCode وجود دارد
        return;
    projectTopic = projectTopic.substring(p + 1);

    StaticJsonDocument<512> doc;  // فرض می‌کنیم اندازه 512 بایت کافی است
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error) {
        Serial.println("JSON parse failed!");
        return;
    }

    if (projectTopic == "DeviceSetting") {
        String opr = doc["operationName"].as<String>();
        if (opr == "save_scenario" || opr == "delete_scenario") {
            // کد مربوط به ذخیره یا حذف سناریو
        } else if (opr == "save_setting") {
            if (doc["deviceSerial"].as<String>() == airnginClient._SerialCloud) {
                String cmd, d = doc["value"].as<String>();
                if (d != "") {
                    deserializeJson(doc, d);
                    if (doc["request"]["commandName"] && doc["request"]["commandData"]) {
                        cmd = doc["request"]["commandName"].as<String>();
                        if (cmd == "saveScenarioOperation") {
                            JsonVariant inp = doc["scenarioOperation"].as<JsonVariant>();
                            // پردازش سناریو
                        }
                        // سایر دستورات...
                    }
                }
            }
        }
    }
    // سایر موضوعات (topics) مشابه...
}


// تعریف Callback برای ذخیره سناریو
void saveScenarioCallback(JsonDocument &doc) {
    Serial.println("Save scenario callback triggered!");
    // پردازش مربوط به ذخیره سناریو
}

// تعریف Callback برای دیباگر
void debuggerCallback(String value) {
    Serial.println("Debugger callback triggered with value: " + value);
   // فقط حالت زمانی که operationNme == "debugger" و value == "special"
   // مابقی عملیات ها را کتابخانه اجرا می نماید
}

// تعریف Callback برای دریافت داده از DeviceToDevice
void message_From_Topic_DeviceToDevice_Callback(uint8_t *payload) {
    Serial.println("message on DeviceToDevice Callback with value: " + String((char *)payload));
}


// تعریف Callback برای دریافت داده از سرور جهت اجرای دستورات
void message_From_Topic_ServerToDevice_Callback(String value) {
    Serial.println("message on ServerToDevice Callback with value: " + value);
}
