#include <AirNgin.h>

#define KEY_OF_CENTER "AIRN"  // IT'S PRODUCER CENTER CODE

#define Pushbotton 23  // PIN FOR GO TO Config Panel and AP MODE
#define RELAY1 16      // RELAY ONE
#define RELAY2 17      // RELAY TWO

#define Pushbutton_RELAY1 32  // push botton RELAY ONE
#define Pushbutton_RELAY2 33  // push botton RELAY TWO



AirNginClient airnginClient;

String _SerialNo = "";

unsigned long _TimerSecCurrent, _TimerSecOld = 0;
byte _TimerSecDef = 0;
byte _TimerKeyPush = 0;


void MqttSend(String topic, String data) {

  Serial.println("MqttSend : >> topic >> " + topic + " ::: data >> " + data);

  airnginClient.Mqtt_Send(topic, data);
}



void setup() {

  airnginClient.CALL_Global_Mqtt_CALLBACK = true;  // if is true just call this airnginClient.setOnMessageCallback(myMqttCallback); \
                                                 // else is false desn't call airnginClient.setOnMessageCallback(myMqttCallback); and call other callback
  bool isDebugEnabled = true; // if you want disable serial print in liberary false this


  Serial.begin(9600);
  Tools__SerialBarcodeReload();
  airnginClient.begin("AirNgin", "1.0.0", _SerialNo,isDebugEnabled);
  airnginClient.setOnMessageCallback(myMqttCallback);
  airnginClient.setOnSaveScenarioCallback(saveScenarioCallback);
  airnginClient.setOnDebuggerCallback(debuggerCallback);
  airnginClient.setOnMessage_From_Topic_DeviceToDevice_Callback(message_From_Topic_DeviceToDevice_Callback);
  airnginClient.setOnMessage_From_Topic_ServerToDevice_Callback(message_From_Topic_ServerToDevice_Callback);

  // Set Pin Mode
  pinMode(Pushbotton, INPUT_PULLUP);
  pinMode(Pushbutton_RELAY1, INPUT_PULLUP);
  pinMode(Pushbutton_RELAY2, INPUT_PULLUP);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
}


void loop() {
  delay(1000);
  airnginClient.client_Loop();
  TimerSec_Refresh();  //read Time
  if (digitalRead(Pushbutton_RELAY1) == LOW && digitalRead(Pushbutton_RELAY2) == LOW) {
    _TimerKeyPush += _TimerSecDef;
  } else
    _TimerKeyPush = 0;
  if (_TimerKeyPush >= 5) {  //
    _TimerKeyPush = 0;
    if (airnginClient.isConfigMode) {
      airnginClient.Tools__SetMode(airnginClient._StartMode == "config_panel" ? "normal" : "config_panel", false);
      airnginClient.isConfigMode = false;
      airnginClient._Mqtt_TryConnecting = false;

    } else {
      airnginClient.Tools__SetMode(airnginClient._StartMode == "config_panel" ? "normal" : "config_panel", false);
      airnginClient.isConfigMode = true;
      airnginClient._Mqtt_TryConnecting = true;
      airnginClient.Config__Setup();
    }
  }

  StaticJsonDocument<4096> doc;

  // ایجاد یک آرایه در JSON
  JsonArray dataArray = doc.to<JsonArray>();

  // اضافه کردن چندین آبجکت به آرایه


  if (digitalRead(Pushbutton_RELAY1) == LOW) {
    if (digitalRead(RELAY1) == LOW) {
      digitalWrite(RELAY1, HIGH);
      // JsonObject obj1 = dataArray.createNestedObject();
      // obj1["Key"] = "ch1";
      // obj1["Value"] = "on";
      // // تبدیل JSON به String برای ارسال با MQTT
      // String payload;
      // serializeJson(doc, payload);

      // // ارسال با MQTT
      // MqttSend("DeviceToServer", payload);
          MqttSend("DeviceToServer", "aliagha");

      Serial.println("relay 1 : on");
    } else {
      digitalWrite(RELAY1, LOW);
      // JsonObject obj1 = dataArray.createNestedObject();
      // obj1["Key"] = "ch1";
      // obj1["Value"] = "off";
      // // تبدیل JSON به String برای ارسال با MQTT
      // String payload;
      // serializeJson(doc, payload);
      // MqttSend("DeviceToServer", payload);
      
          MqttSend("DeviceToServer", "mohammadagha");

      Serial.println("relay 1 : off");
    }
  }

  if (digitalRead(Pushbutton_RELAY2) == LOW) {
    if (digitalRead(RELAY2) == LOW) {
      digitalWrite(RELAY2, HIGH);
      // JsonObject obj1 = dataArray.createNestedObject();
      // obj1["Key"] = "ch2";
      // obj1["Value"] = "on";
      // // تبدیل JSON به String برای ارسال با MQTT
      // String payload;
      // serializeJson(doc, payload);
      // MqttSend("DeviceToServer", payload);
          MqttSend("DeviceToServer", "ch2on");

      Serial.println("relay 2 : on");
    } else {
      digitalWrite(RELAY2, LOW);
      // JsonObject obj1 = dataArray.createNestedObject();
      // obj1["Key"] = "ch2";
      // obj1["Value"] = "off";
      // // تبدیل JSON به String برای ارسال با MQTT
      // String payload;
      // serializeJson(doc, payload);
      // MqttSend("DeviceToServer", payload);
       MqttSend("DeviceToServer", "ch2off");
      Serial.println("relay 2 : off");
    }
  }
}


void Tools__SerialBarcodeReload() {

  String chip = (String(airnginClient.Tools__GetChipID()) + "0000000").substring(0, 7);
  if (chip == "0000000")
    chip = (String(airnginClient.Tools__Random(1000000, 9999998)) + "0000000").substring(0, 7);
  _SerialNo = KEY_OF_CENTER + ("000" + chip).substring(0, 10);


  Serial.println("_SerialNo : " + _SerialNo);
}


void TimerSec_Refresh() {
  //-------------------------------------------- TIMER
  try {
    _TimerSecCurrent = millis();
    _TimerSecDef = ((_TimerSecCurrent - _TimerSecOld) / 1000) & 0xFF;
    if (_TimerSecDef < 0)
      _TimerSecDef = 1;
    if (_TimerSecDef >= 1)
      _TimerSecOld = _TimerSecCurrent;
  } catch (...) {
  }
}


void myMqttCallback(char *topic, uint8_t *payload, unsigned int length) {


  Serial.print("Received data length: ");
  Serial.println(String(length));

  Serial.print("Data: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);  // تبدیل هر بایت به کاراکتر
  }
  Serial.println();

  String projectTopic = String(topic);
  int p = projectTopic.indexOf('/');
  if (projectTopic.substring(0, p) != airnginClient._ProjectCode)  // فرض بر این است که متدی برای گرفتن ProjectCode وجود دارد
    return;
  projectTopic = projectTopic.substring(p + 1);


  JsonDocument doc;
  doc.clear();

  DeserializationError error = deserializeJson(doc, payload, length);

  Serial.print("Received data from topic: ");
  Serial.println(topic);

  if (error) {
    Serial.println("JSON parse failed!");
    return;
  }

  if (projectTopic == "ServerToDevice") {  // اگر داده ارسالی از سمت سرور باشد

    // خروجی ما بصورت زیر است
    //{"data":"{\"type\":\"command\",\"value\":\"ch2on\"}","deviceSerial":"AIRN0001208520"}
    // دقت نمایید همیشه خروجی شما از طریق js generate
    // درون یک json بصورت بالا
    // در کلید data
    // قرار می گیرد

    // اسختراج سریال ابزاری که سرور برای آن داده را ارسال کرده ، جهت اینکه بدانید برای کدام ابزار است و با سریال خودتان در ادامه قیاس نمایید.
    String deviceSerial = doc["deviceSerial"].as<String>();
    String operationName = "";

    if ((doc["data"])) {

      if (deviceSerial == airnginClient._SerialCloud) {  // مطمین شویم که این داده برای ما ارسال شده است.

        operationName = doc["data"].as<String>();  // اگر دیتا داشت چون خروجی فایل جاوااسکریپت ما هم یک json بوده
                                                   // تشکیل یک json در json داده ایم

        if (operationName == "mohammadagha") {
          digitalWrite(RELAY1, LOW);
          Serial.println(" with command from server  >  Relay 1 : OFF");
          MqttSend("DeviceToServer", operationName);

        } else if (operationName == "aliagha") {
          digitalWrite(RELAY1, HIGH);
          MqttSend("DeviceToServer", operationName);

          Serial.println(" with command from server  >  Relay 1 : ON");
        } else if (operationName == "ch2off") {
          digitalWrite(RELAY2, LOW);
          Serial.println(" with command from server  >  Relay 2 : OFF");
          MqttSend("DeviceToServer", operationName);

        } else if (operationName == "ch2on") {
          digitalWrite(RELAY2, HIGH);
          MqttSend("DeviceToServer", operationName);
          Serial.println(" with command from server  >  Relay 2 : ON");
        }
        // if (operationName != "") {



        // doc.clear();
        // deserializeJson(doc, operationName);
        // // String type = doc["type"].as<String>();
        // String value = doc["value"].as<String>();
        // if (doc["key"]) operationName = doc["key"].as<String>();

        // // Serial.println(" type > " + type);
        // Serial.println(" value > " + value);
        // Serial.println(" operationName > " + operationName);
        // if (operationName == "ch1") {
        //   if (value == "on") {
        //     digitalWrite(RELAY1, HIGH);
        //     Serial.println(" with command from server  >  Relay 1 : on");
        //   } else {
        //     digitalWrite(RELAY1, LOW);
        //     Serial.println(" with command from server  >  Relay 1 : OFF");
        //   }
        // } else if (operationName == "ch2") {
        //   if (value == "on") {
        //     digitalWrite(RELAY2, HIGH);
        //     Serial.println(" with command from server  >  Relay 2 : on");
        //   } else {
        //     digitalWrite(RELAY2, LOW);
        //     Serial.println(" with command from server  >  Relay 2 : OFF");
        //   }
        // }


        // StaticJsonDocument<4096> doc;

        // // ایجاد یک آرایه در JSON
        // JsonArray dataArray = doc.to<JsonArray>();

        // // اضافه کردن چندین آبجکت به آرایه
        // JsonObject obj1 = dataArray.createNestedObject();
        // obj1["Key"] = operationName;
        // obj1["Value"] = value;


        // // تبدیل JSON به String برای ارسال با MQTT
        // String payload;
        // serializeJson(doc, payload);

        // // ارسال با MQTT
        // MqttSend("DeviceToServer", payload);


        // if (type == "command") {
        //   if (value == "ch1on") {
        //     digitalWrite(RELAY1, HIGH);
        //     Serial.println(" with command from server  >  Relay 1 : on");

        //   } else if (value == "ch1off") {
        //     digitalWrite(RELAY1, LOW);
        //     Serial.println(" with command from server  >  Relay 1 : OFF");

        //   } else if (value == "ch2on") {
        //     digitalWrite(RELAY2, HIGH);
        //     Serial.println(" with command from server  >  Relay 2 : on");

        //   } else if (value == "ch2off") {
        //     digitalWrite(RELAY2, LOW);
        //     Serial.println(" with command from server  >  Relay 2 : OFF");
        //   }
        //   MqttSend("DeviceToServer", value);  // حتما تغییرات را برای ثبت در سرور و ایجاد تغییر در اپلیکیشن ها به سمت سرور ارسال می نماییم.
        // }
        //}
      }
    }
  } else if (projectTopic == "Time/Tehran") {
    Serial.println(String((char *)payload));
  }
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


//Mqtt_Send(String topic, String data);