#include <WiFi.h>
#include <Wire.h>
#include<LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "Pesan Modal"
#define WIFI_PASSWORD "planetpluto"

#define API_KEY "AIzaSyDDbNbZb7NzNanotnUJXUEZsf3vi3zyxc4"
#define DATABASE_URL "irigasiiot2-default-rtdb.asia-southeast1.firebasedatabase.app"

FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;

bool signupOK = false;

int kelembabanOutVal = 0;
int debitPin = 35;
int switchPU = 4;
int switchPS = 2;

volatile long pulse;
unsigned long lastTime;
float debitVal;

LiquidCrystal_I2C lcd(0x27,16,2);

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Irigation IoT");
  lcd.setCursor(2,1);
  lcd.print("Rifqi Dimas P");
  delay(500);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Connecting");
    lcd.setCursor(2,1);
    lcd.print("To Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Connecting");
        lcd.setCursor(2,1);
        lcd.print("................");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ip Address:");
    lcd.setCursor(2,1);
    lcd.print(WiFi.localIP());
    Serial.println();
  
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);

    Serial.print("Sign up new user... ");

    if (Firebase.signUp(&config, &auth, "", ""))
    {
        Serial.println("ok");
        signupOK = true;

    }
    else
        Serial.printf("%s\n", config.signer.signupError.message.c_str());

    Firebase.begin(&config, &auth);

 
  pinMode(debitPin, INPUT);
  pinMode(switchPU, OUTPUT);
  pinMode(switchPS, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(debitPin), increase, RISING);
}

void loop() {

  int kelembabanVal = analogRead(34);
  
  kelembabanOutVal = map(kelembabanVal, 0, 2550, 0, 600);
  Serial.print("Kelembaban tanah = ");
  Serial.println(kelembabanOutVal);
  Serial.println("");
  Firebase.setInt(fbdo, "/sensor/kelembaban", kelembabanOutVal);
  delay(500);

  debitVal = 2.663 * pulse / 1000 * 30;
  if (millis() - lastTime > 1000) {
    pulse = 0;
    lastTime = millis();
  }
  Serial.print(debitVal);
  Serial.println(" L/m");
  Serial.println("");
  Firebase.setFloat(fbdo, "/sensor/debit", debitVal);
  delay(500);
  
  if (debitVal<=2.00){
    Serial.println("Aliran air tersumbat");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Aliran Air");
    lcd.setCursor(2,1);
    lcd.print("Tersumbat");
    Firebase.setString(fbdo, "/status/statusDebit", "Aliran air tersumbat");
  }
  else if (debitVal>=2.01){
    Serial.println("Aliran air normal");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Aliran Air");
    lcd.setCursor(2,1);
    lcd.print("Normal");
    Firebase.setString(fbdo, "/status/statusDebit", "Aliran air normal");
  }
  Serial.println("");
  delay(500);

  if (kelembabanOutVal<=350){
    Serial.println("Tanah Basah");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tanah Basah");
    lcd.setCursor(2,1);
    lcd.print("");
    Firebase.setString(fbdo, "/status/statusKelembaban", "Tanah basah");
  }
  if (kelembabanOutVal>=351 && kelembabanOutVal<=430){
    Serial.println("Cukup Lembab");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tanah Lembab");
    lcd.setCursor(2,1);
    lcd.print("");
    Firebase.setString(fbdo, "/status/statusKelembaban", "Tanah lembab");
  }
  else if (kelembabanOutVal>=431){
    Serial.println("Kering");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Tanah Kering");
    lcd.setCursor(2,1);
    lcd.print("");
    Firebase.setString(fbdo, "/status/statusKelembaban", "Tanah kering");
  }
  Serial.println("");
  delay(500);

  if (kelembabanOutVal>=431) {
    digitalWrite(switchPS, LOW);
    Serial.println("Pintu ke kebun salak membuka");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pintu kebun");
    lcd.setCursor(2,1);
    lcd.print("salak membuka");
    Firebase.setString(fbdo, "/status/status_ps", "Pintu ke kebun salak membuka");
  }
  
  else if (kelembabanOutVal<=350) {
    digitalWrite(switchPS, HIGH);
    Serial.println("Pintu ke kebun salak menutup");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Pintu kebun");
    lcd.setCursor(2,1);
    lcd.print("salak menutup");
    Firebase.setString(fbdo, "/status/status_ps", "Pintu ke kebun salak menutup");
  }
  Serial.println("");
  delay(500);

  Firebase.getString(fbdo, "switch_PU");
    String pintuUtama = fbdo.stringData();
    if(pintuUtama == "1") {
      Serial.println("Pintu utama membuka");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Pintu utama");
      lcd.setCursor(2,1);
      lcd.print("sedang membuka");
      digitalWrite(switchPU, LOW);
    }
    else if (pintuUtama == "0") {
      Serial.println("Pintu utama menutup");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Pintu utama");
      lcd.setCursor(2,1);
      lcd.print("sedang menutup");
      digitalWrite(switchPU, HIGH);
  }
  Serial.println("");
  delay(500);
}

void increase() {
  pulse++;
}