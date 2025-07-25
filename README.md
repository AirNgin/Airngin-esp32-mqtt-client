# AirNgin ESP32 MQTT Client v0.1.11 / Arduino.cc Library
**Arduino Library for Client Connection to the AirNgin Broker**

ورژن 0.1.2 دارای متدهای ارتباط با پایگاه داده ابری می باشد که به شما قابلیت افزودن، بروز رسانی و حذف رکوردها به شما می دهد.

این کتابخانه آردوینو برای پلتفرم اینترنت اشیا **AirNgin** طراحی و توسعه داده شده است. با استفاده از این کتابخانه که به‌صورت مداوم به‌روزرسانی می‌شود، می‌توانید به‌راحتی و بدون نگرانی به بروکر **AirNgin** متصل شوید.

---

## نکته :
برای غیر فعال سازی حالت دیباگ و جلوگیری از پرینت لاگ ها کافی است مقدار متغییر زیر در setup را false نمایید.

bool isDebugEnabled = true;

## نکته بسیار مهم:
از منوی Tools > Partition schema > minimal SPIFFS قرار دهید تا حافظه با حجم درست تنظیم شود.


## مراحل اولیه استفاده
قبل از استفاده از این کتابخانه، لازم است که در **پنل تولیدکنندگان AirNgin** به آدرس زیر عضو شوید و ابزار (Device) مورد نظر خود را ثبت کنید:

🔗 [https://panel.airngin.com](https://panel.airngin.com)

برای راهنمایی بیشتر، می‌توانید از آموزش‌های موجود در سایت استفاده کنید:

🔗 [اتصال 2 رله به برد توسعه ESP32](https://airngin.com/%d8%a7%d8%aa%d8%b5%d8%a7%d9%84-2-%d8%b1%d9%84%d9%87-%d8%a8%d8%b1%d8%af-%d8%a8%d8%a7-%d8%a8%d8%b1%d8%af-%d8%aa%d9%88%d8%b3%d8%b9%d9%87-esp32-wroom-32d-%d8%a8%d8%a7-%da%a9%d8%aa%d8%a7%d8%a8/)

---

## امکانات کتابخانه
این کتابخانه تمامی امکانات لازم را فراهم کرده است تا شما بتوانید با **کمترین حجم کدنویسی**، بردهای خود را توسعه داده و به دنیای اینترنت اشیا متصل کنید. برخی از ویژگی‌های کلیدی این کتابخانه عبارت‌اند از:

- اتصال خودکار به بروکر و مدیریت قطعی‌ها و فرآیند اتصال دوباره
- قابلیت آپدیت OTA فریمور ابزار از راه دور
- امکان ریست، ریبوت و فعال‌سازی حالت AP از راه دور توسط تولیدکننده (با اجازه کاربر)
- پنل کانفیگ برای راه‌اندازی سریع ابزار

---

## پنل کانفیگ
در نمونه کد `simple.ino`، پین **23** به‌عنوان یک **پوش‌باتن** در نظر گرفته شده است. هنگامی که این پوش‌باتن به مدت **6 ثانیه** فشار داده شود، پنل کانفیگ راه‌اندازی می‌شود و برد ESP32 شما به حالت **Access Point (AP)** می‌رود و یک شبکه وای‌فای ایجاد می‌کند.

در این حالت می‌توانید با استفاده از نرم‌افزار موبایل یا با اتصال به ابزار (رمز عبور: `00000000`) و وارد کردن آدرس **192.168.1.1**، به پنل کانفیگ دسترسی پیدا کنید.

---

## اتصال خودکار به بروکر
این کتابخانه به‌صورت خودکار به بروکر متصل می‌شود و در صورت بروز قطعی، فرآیند اتصال مجدد را مدیریت می‌کند.

---

## قابلیت آپدیت OTA
این قابلیت به شما اجازه می‌دهد تا به‌صورت **از راه دور** فریمور ابزار خود را به‌روزرسانی کنید. این ویژگی برای رفع مشکلات نرم‌افزاری و اضافه کردن قابلیت‌های جدید به ابزار بسیار مفید است.

دقت نمایید اگر متغییر #define CALL_Global_Mqtt_CALLBACK false // if is true just call this airnginClient.setOnMessageCallback(myMqttCallback); برابر با true باشد شما باید تمامی مراحل آپلود فایل را خودتان انجام دهید. زیرا با true بودن این متغییر پردازش تمام داده ها بصورت دستی می باشد.


---

## مدیریت راه دور
با استفاده از این کتابخانه، می‌توانید از راه دور عملیات زیر را انجام دهید (البته با اجازه کاربر ابزار):

- **ریست کردن ابزار**
- **ریبوت کردن ابزار**
- **فعال‌سازی حالت AP**
- **تست ابزار**

---

## متدهای Callback متنوع
این کتابخانه از متدهای متنوعی برای مدیریت پیام‌ها و رویدادهای مختلف پشتیبانی می‌کند.

### فعال‌سازی Callback سراسری
اگر متغیر سراسری زیر را **`true`** کنید، تنها یک Callback عمومی به نام `myMqttCallback` فعال می‌شود:


#define CALL_Global_Mqtt_CALLBACK true

---


## پیش‌نیازها

کتابخانه **AirNgin ESP32 MQTT Client** برای اجرا نیاز به کتابخانه‌های زیر دارد. برخی از این کتابخانه‌ها به‌صورت پیش‌فرض در Arduino Core برای ESP32 موجود هستند، اما برخی دیگر باید به‌صورت دستی نصب شوند.

### کتابخانه‌های پیش‌فرض (نیازی به نصب دستی ندارند):

1. `Arduino.h`  
2. `WiFi`  
3. `EEPROM`  
4. `SPIFFS`  
5. `WString`  
6. `WiFiClientSecure`  
7. `ESPmDNS`

### کتابخانه‌هایی که باید نصب شوند:

لطفاً پیش از استفاده از کتابخانه، موارد زیر را از **Library Manager** در Arduino IDE نصب کنید:

1. **WebServer**  
   بخشی از ESP32 Core است، اما اگر نصب نبود، می‌توانید از Library Manager نصب کنید.

2. **PubSubClient**  
   لینک به مخزن: [PubSubClient](https://github.com/knolleary/pubsubclient)  
   یا از Library Manager با جستجوی `PubSubClient` نصب کنید.

3. **ArduinoJson**  
   لینک به مخزن: [ArduinoJson](https://github.com/bblanchon/ArduinoJson)  
   یا از Library Manager با جستجوی `ArduinoJson` نصب کنید.

4. **HTTPUpdate**  
   این کتابخانه بخشی از ESP32 Core است. اگر نصب نبود، می‌توانید از Library Manager نصب کنید.

5. **ESPping**  
   لینک به مخزن: [ESPping](https://github.com/dancol90/ESPping)  
   یا از Library Manager با جستجوی `ESPping` نصب کنید.




## **روش‌های نصب کتابخانه**

### **روش 1: نصب از طریق Library Manager در Arduino IDE**

1. Arduino IDE را باز کنید.
2. به منوی **Sketch > Include Library > Manage Libraries...** بروید.
3. در کادر جستجو، عبارت زیر را وارد کنید:
AirNgin ESP32 MQTT Client

4. کتابخانه را از لیست نتایج پیدا کرده و روی **Install** کلیک کنید.

---

### **روش 2: نصب دستی از طریق فایل ZIP**

1. به صفحه اصلی مخزن گیت‌هاب کتابخانه بروید:  
[AirNgin ESP32 MQTT Client](https://github.com/AirNgin/Airngin-esp32-mqtt-client)
2. روی دکمه **Code** کلیک کرده و گزینه **Download ZIP** را انتخاب کنید.
3. پس از دانلود، Arduino IDE را باز کرده و به منوی **Sketch > Include Library > Add .ZIP Library...** بروید.
4. فایل ZIP دانلود شده را انتخاب کنید و روی **Open** کلیک کنید.
5. کتابخانه به Arduino IDE اضافه می‌شود و می‌توانید از آن استفاده کنید.

---

### **روش 3: نصب دستی از طریق Git**

1. به مسیر کتابخانه‌های Arduino در سیستم خود بروید:
Documents/Arduino/libraries/


2. یک ترمینال یا Command Prompt باز کنید و دستور زیر را اجرا کنید:
```bash
git clone https://github.com/AirNgin/Airngin-esp32-mqtt-client.git
```

Arduino IDE را باز کنید. کتابخانه به‌صورت خودکار شناسایی می‌شود و آماده استفاده خواهد بود.


