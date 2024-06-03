#include <ESP8266WiFi.h>
#include <RTClib.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Firebase_ESP_Client.h>
#include <UrlEncode.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_PWMServoDriver.h>


const int buzzerPin = D8;
const int irSensorPin = D4;

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

const char *ssid = "Your_SSID";
const char *password = "Password";
#define FIREBASE_HOST "pill-dispensing-system-6c2b6-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_API_KEY "AIzaSyDi3DIZm9jGJYTcDGtK3cIey46Hk0hW_KY"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int alarmHours[3] = {0};
int alarmMinutes[3] = {0};
bool isDispensed[3] = {false};
bool signupOK = false;

// RTC_DS3231<TwoWire> Rtc(Wire);
RTC_DS3231 rtc;
// const int servoOpenPosition = 150;
// const int servoClosePosition = 600;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
String phoneNumber = "phone_number";
String apiKey = "4486037";

void displayDateTime(const DateTime &dt);
void checkAndDispensePills(const DateTime &now);
void dispensePills(int slot);
void sendMessage(String message);

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  pinMode(buzzerPin, OUTPUT);
  pinMode(irSensorPin, INPUT);
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // rtc.adjust(DateTime(2024, 4, 24, 19, 00, 0));

  lcd.init();
  lcd.backlight();
  pwm.begin();
  pwm.setPWMFreq(60);
  delay(10);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Database URL: " + String(FIREBASE_HOST));
  Serial.println("API Key: " + String(FIREBASE_API_KEY));

  config.api_key = FIREBASE_API_KEY;
  config.database_url = FIREBASE_HOST;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);

  if (Firebase.ready())
  {
    Serial.println("Firebase is ready!");
  }
  else
  {
    Serial.println("Failed to initialize Firebase");
  }

  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo, "/times"))
  {
    Serial.println("Failed to begin stream: " + fbdo.errorReason());
  }
}

void loop()
{
  DateTime now = rtc.now();
  displayDateTime(now);

  // Check for updates at a less frequent interval or upon specific triggers
  static unsigned long lastFirebaseUpdate = 0;
  unsigned long currentMillis = millis();
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    lastFirebaseUpdate = millis();
    // sendDataPrevMillis = millis();
    Serial.println("Updating alarm times from Firebase...");

    String pathHour = "/times/0/hour";
    String pathMinute = "/times/0/minute";
    Serial.print("Checking path for hours: ");
    Serial.println(pathHour);
    if (!Firebase.RTDB.getInt(&fbdo, pathHour))
    {
      Serial.print("Failed to fetch alarm hours for index ");
      Serial.print(0);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmHours[0] = fbdo.intData();
      Serial.print("Fetched hour for index ");
      Serial.print(0);
      Serial.print(": ");
      Serial.println(alarmHours[0]);
    }

    Serial.print("Checking path for minutes: ");
    Serial.println(pathMinute);
    if (!Firebase.RTDB.getInt(&fbdo, pathMinute))
    {
      Serial.print("Failed to fetch alarm minutes for index ");
      Serial.print(0);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmMinutes[0] = fbdo.intData();
      Serial.print("Fetched minute for index ");
      Serial.print(0);
      Serial.print(": ");
      Serial.println(alarmMinutes[0]);
    }

    String pathHour1 = "/times/1/hour";
    String pathMinute1 = "/times/1/minute";
    Serial.print("Checking path for hours: ");
    Serial.println(pathHour1);
    if (!Firebase.RTDB.getInt(&fbdo, pathHour1))
    {
      Serial.print("Failed to fetch alarm hours for index ");
      Serial.print(1);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmHours[1] = fbdo.intData();
      Serial.print("Fetched hour for index ");
      Serial.print(1);
      Serial.print(": ");
      Serial.println(alarmHours[1]);
    }
    Serial.print("Checking path for minutes: ");
    Serial.println(pathMinute1);
    if (!Firebase.RTDB.getInt(&fbdo, pathMinute1))
    {
      Serial.print("Failed to fetch alarm minutes for index ");
      Serial.print(1);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmMinutes[1] = fbdo.intData();
      Serial.print("Fetched minute for index ");
      Serial.print(1);
      Serial.print(": ");
      Serial.println(alarmMinutes[1]);
    }

    String pathHour2 = "/times/2/hour";
    String pathMinute2 = "/times/2/minute";
    Serial.print("Checking path for hours: ");
    Serial.println(pathHour2);
    if (!Firebase.RTDB.getInt(&fbdo, pathHour2))
    {
      Serial.print("Failed to fetch alarm hours for index ");
      Serial.print(2);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmHours[2] = fbdo.intData();
      Serial.print("Fetched hour for index ");
      Serial.print(2);
      Serial.print(": ");
      Serial.println(alarmHours[2]);
    }

    Serial.print("Checking path for minutes: ");
    Serial.println(pathMinute2);
    if (!Firebase.RTDB.getInt(&fbdo, pathMinute2))
    {
      Serial.print("Failed to fetch alarm minutes for index ");
      Serial.print(2);
      Serial.print(": ");
      Serial.println(fbdo.errorReason());
    }
    else
    {
      alarmMinutes[2] = fbdo.intData();
      Serial.print("Fetched minute for index ");
      Serial.print(2);
      Serial.print(": ");
      Serial.println(alarmMinutes[2]);
    }
  }

  checkAndDispensePills(now);
  dispensePills(0);
  // rotateServoAndBack(0);
  delay(1000);

  int sensorState = digitalRead(irSensorPin);
  static int lastSensorState = HIGH;
  if (sensorState == LOW && lastSensorState == HIGH)
  {

    sendMessage("Pills dispensed\nThank You!");
    delay(1000);
  }
  lastSensorState = sensorState;
}

void displayDateTime(const DateTime &dt)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date:");
  lcd.print(dt.year());
  lcd.print("-");
  lcd.print(dt.month());
  lcd.print("-");
  lcd.print(dt.day());
  lcd.setCursor(0, 1);
  lcd.print("Time:");
  lcd.print(dt.hour());
  lcd.print(":");
  lcd.print(dt.minute());
  lcd.print(":");
  lcd.print(dt.second());
}

void checkAndDispensePills(const DateTime &now)
{
  for (int i = 0; i < 3; i++)
  {
    Serial.print("Checking alarm ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(alarmHours[i]);
    Serial.print(":");
    Serial.println(alarmMinutes[i]);
    if (now.hour() == alarmHours[i] && now.minute() == alarmMinutes[i] && !isDispensed[i])
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hello!");
      delay(3000);
      digitalWrite(buzzerPin, HIGH);
      lcd.clear();
      lcd.print("Pills're Ready!");
      delay(4000);
      digitalWrite(buzzerPin, LOW);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Dispensing Pills!");
      delay(4000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank You!");
      delay(4000);
      dispensePills(i);
      isDispensed[i] = true;
      break;
    }
  }
}

void dispensePills(int slot)
{
  const int servoOpenPosition = 150;
  const int servoClosePosition = 600;
  pwm.setPWM(slot, 0, servoOpenPosition);
  delay(500);

  pwm.setPWM(slot, 0, servoClosePosition);
  delay(500);
  // pwm.write(90);
  delay(200);
}
void sendMessage(String message)
{
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200)
  {
    Serial.println("Message sent successfully");
  }
  else
  {
    Serial.print("Error sending the message, HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}
