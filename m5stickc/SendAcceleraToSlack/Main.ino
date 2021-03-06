#include <M5StickC.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// 加速度(単位が重力加速度[g])
float accX_g = 0;
float accY_g = 0;
float accZ_g = 0;

// 加速度(単位が加速度[m/s2])
float accX = 0;
float accY = 0;
float accZ = 0;

// 角速度
float GyroX = 0;
float GyroY = 0;
float GyroZ = 0;

// post json
char buffer[255];

void wifi_setup() {
  const char* ssid = get_ssid();
  const char* passwd = get_passwd();
  M5.Lcd.printf("ssid : %s", ssid);
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  M5.Lcd.printf("WiFi connected");
}

void send_slack() {
  const char* slack_bot_token = get_slack_bot_token();
  const char* oauth_token = get_oauth_token();
  
  HTTPClient http;
  String url = "https://slack.com/api/chat.postMessage";

  const int capacity = JSON_OBJECT_SIZE(3);
  StaticJsonDocument<capacity> json_request;
  json_request["token"] = oauth_token;
  json_request["channel"] = "#t_mizushima";
  json_request["text"] = "Now bunbun m5stickC!";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + slack_bot_token);

  serializeJson(json_request, buffer, sizeof(buffer));
  unsigned int responseCode = http.POST((uint8_t*)buffer, strlen(buffer));
  String payload = http.getString();

  // M5.Lcd.printf("status code: %d", responseCode);
  // M5.Lcd.printf("payload: %s", payload.c_str());
}


void setup() {
  // Initialize the M5StickC object
  M5.begin();
  // 6軸センサ初期化
  M5.MPU6886.Init();
  // LED初期化   
  pinMode(10, OUTPUT);
  
  M5.Lcd.setRotation(1);  // ボタンBが上になる向き
  M5.Lcd.fillScreen(BLACK);
  
  // Wifi初期化
  wifi_setup();
  digitalWrite(10, HIGH);
}

void loop() {
  digitalWrite(10, HIGH);
  M5.MPU6886.getAccelData(&accX_g,&accY_g,&accZ_g);
  M5.MPU6886.getGyroData(&GyroX,&GyroY,&GyroZ);
  M5.Lcd.setCursor(0, 30);
  accX = accX_g * 9.8;
  accY = accY_g * 9.8;
  accZ = accZ_g * 9.8;
  M5.Lcd.printf("Acc : %.2f  %.2f  %.2f   ", accX, accY, accZ);
  M5.Lcd.printf("Gyro : %.2f  %.2f  %.2f   ", GyroX, GyroY, GyroZ);

  if (abs(accX + accY + accZ) > 19.6) {
    digitalWrite(10, LOW);
    send_slack();
  }
  delay(500);
}
