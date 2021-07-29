//---------Khai bao thu vien dung trong chuong trinh-------//
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
//--------Khai bao cac chan GPIO cua ESP8266--------//
#define OUT1 13
#define OUT2 10
#define OUT3 15
#define OUT4 2
#define OUT5 0
#define OUT6 4
#define IN1 16
#define IN2 14
#define IN3 12
#define IN4 1
#define IN5 3
#define IN6 5

//-------Khai bao cac bien trong chương trinh-------//
boolean restoreConfig();
boolean checkConnection();
void startWebServer();
void setupMode();
String makePage(String title, String contents);
String urlDecode(String input);
void readResetButton1();


const IPAddress apIP(192, 168, 1, 1);
const char* apSSID = "ESP8266_SETUP";
boolean settingMode;      
String ssidList;          // Bien liet ke cac mang wifi xung quanh
bool state1=0, state2 =0, state3=0, state4=0, state5=0, state6=0;  //bien trang thai relay
bool normalState1 = 1, normalState2 = 1, normalState3 = 1, normalState4 = 1, normalState5 = 1, normalState6 = 1; //Bien trang thai nut bam
int SETTING_HOLD_TIME = 5000;   //thoi gian bam nut de chuyen trang thai
long int settingTimeout;        
DNSServer dnsServer;
ESP8266WebServer webServer(80); //khoi tao server cho esp8266 voi port 80 danh cho http
//----------Ham setup----------//
void setup() {
//----------Dat che do cho cac cong GPIO-------------//
  pinMode(OUT1, OUTPUT);
  pinMode(OUT2, OUTPUT);
  pinMode(OUT3, OUTPUT);
  pinMode(OUT4, OUTPUT);
  pinMode(OUT5, OUTPUT);
  pinMode(OUT6, OUTPUT);
  
  pinMode(IN1, INPUT);
  pinMode(IN2, INPUT);
  pinMode(IN3, INPUT);
  pinMode(IN4, INPUT);
  pinMode(IN5, INPUT);
  pinMode(IN6, INPUT);
  
  EEPROM.begin(512);       //
  delay(10);
  if (restoreConfig()) {                        //neu co du lieu wifi trong EEPROM thi ket noi va kiem tra ket noi voi wifi do
    if (checkConnection()) {
      settingMode = false;                      //set che do cai dat la false de bo qua buoc settingMode
      startWebServer();                         //khoi tao 1 webserver de truy cap
      return;                                   
    }
  }
  settingMode = true;                           // set che do cai dat lai thanh true
  setupMode();                                  //chuyen sang che do station de vao cai dat cau hinh wifi  
}

void loop() {
  if (settingMode) {                      //neu settingMode == 1 thi 
    dnsServer.processNextRequest();
  }
  readResetButton1();
  readAllButton();
  Blynk.run();
  webServer.handleClient();
}
//----------Ham doc trang thai tat ca cac nut bam----------//
void readAllButton(){
  readButton1();
  readButton2();
  readButton3();
  readButton4();
  readButton5();
  readButton6();
}
//----------Ham doc trang thai tung nut bam-------------//
//---------Kich hoat o suon am khi bam nut----------------//
void readButton1(){
  if((digitalRead(IN1)-normalState1)<0){
    normalState1 = 0;
    changeState1();
    Blynk.virtualWrite(V1,state1);
  }
  else if((digitalRead(IN1)-normalState1)>0){
    normalState1 = 1;
  }
}
void readButton2(){
  if((digitalRead(IN2)-normalState2)<0){
    normalState2 = 0;
    changeState2();
    Blynk.virtualWrite(V2,state2);
  }
  else if((digitalRead(IN2)-normalState2)>0){
    normalState2 = 1;
  }
}
void readButton3(){
  if((digitalRead(IN3)-normalState3)<0){
    normalState3 = 0;
    changeState3();
    Blynk.virtualWrite(V3,state3);
  }
  else if((digitalRead(IN3)-normalState3)>0){
    normalState3 = 1;
  }
}
void readButton4(){
  if((digitalRead(IN4)-normalState4)<0){
    normalState4 = 0;
    changeState4();
    Blynk.virtualWrite(V4,state4);
  }
  else if((digitalRead(IN4)-normalState4)>0){
    normalState4 = 1;
  }
}
void readButton5(){
  if((digitalRead(IN5)-normalState5)<0){
    normalState5 = 0;
    changeState5();
    Blynk.virtualWrite(V5,state5);
  }
  else if((digitalRead(IN5)-normalState5)>0){
    normalState5 = 1;
  }
}
void readButton6(){
  if((digitalRead(IN6)-normalState6)<0){
    normalState6 = 0;
    changeState6();
    Blynk.virtualWrite(V6,state6);
  }
  else if((digitalRead(IN6)-normalState6)>0){
    normalState6 = 1;
  }
}
//--------------Ham chuyen trang thai cac relay-------------//
void changeState1(){
  state1 = !state1;
  digitalWrite(OUT1, state1);
}
void changeState2(){
  state2 = !state2;
  digitalWrite(OUT2, state2);
}
void changeState3(){
  state3 = !state3;
  digitalWrite(OUT3, state3);
}
void changeState4(){
  state4 = !state4;
  digitalWrite(OUT4, state4);
}
void changeState5(){
  state5 = !state5;
  digitalWrite(OUT5, state5);
}
void changeState6(){
  state6 = !state6;
  digitalWrite(OUT6, state6);
}
//------------Ket noi voi Blynk---------//
BLYNK_WRITE(V1) 
{
  changeState1();
}
BLYNK_WRITE(V2) 
{
  changeState2();
}
BLYNK_WRITE(V3) 
{
  changeState3();
}
BLYNK_WRITE(V4) 
{
  changeState4();
}
BLYNK_WRITE(V5) 
{
  changeState5();
}
BLYNK_WRITE(V6) 
{
  changeState6();
}


//----------Doc duu lieu trong EEPROM va ket noi voi mang wifi--------------
boolean restoreConfig() {
  String ssid = "";
  String pass = "";
  String auth = "";
  if (EEPROM.read(0) != 0) {                      //neu duu lieu doc ra tu EEPROM khac 0 thi doc du lieu
    for (int i = 0; i < 32; ++i) {                //32 o nho dau tieu la chua ten mang wifi SSID
      ssid += char(EEPROM.read(i));
    }

    for (int i = 32; i < 96; ++i) {               //o nho tu 32 den 96 la chua PASSWORD
      pass += char(EEPROM.read(i));
    }

    for (int i = 96; i < 128; ++i) {               //o nho tu 32 den 96 la chua TOKEN
      auth += char(EEPROM.read(i));
    }


    //WiFi.begin(ssid.c_str(), pass.c_str());       //ket noi voi mang WIFI duoc luu trong EEPROM
    Blynk.begin(auth.c_str(),ssid.c_str(),pass.c_str());
    return true;
  }
  else {

    return false;
  }
}
//-----------Kiem tra lai ket noi voi WIFI-----------------------
boolean checkConnection() {
  int count = 0;

  while ( count < 30 ) {
    if (WiFi.status() == WL_CONNECTED) {      //neu ket noi thanh cong thi in ra connected!
      return (true);
    }
    delay(500);
    count++;
  }
  return false;
}
//-----------------Thiet lap mot WEBSERVER-------------------------------
void startWebServer() {
  if (settingMode) {                            //neu o chua settingMode la true thi thiet lap 1 webserver
    webServer.on("/settings", []() {
      String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
      s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
      s += ssidList;
      s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\">";
      s += "</select><br>Token: <input name=\"auth\" length=64 type=\"text\"><input type=\"submit\"></form>";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    });
    webServer.on("/setap", []() {
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);               //xoa bo nho EEPROM
      }
      String ssid = urlDecode(webServer.arg("ssid"));
      String pass = urlDecode(webServer.arg("pass"));
      String auth = urlDecode(webServer.arg("auth"));
      //Ghi du lieu vao EEPROM
      for (int i = 0; i < ssid.length(); ++i) {
        EEPROM.write(i, ssid[i]);
      }
      for (int i = 0; i < pass.length(); ++i) {
        EEPROM.write(32 + i, pass[i]);
      }
      for (int i = 0; i < auth.length(); ++i) {
        EEPROM.write(96 + i, auth[i]);
      }
      String endtext = "";
      for (int i = 0; i < auth.length(); ++i) {
        EEPROM.write(160 + i, endtext[i]);
      }
      
      EEPROM.commit();
      String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
      s += ssid;
      s += "\" after the restart.";
      webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
      ESP.restart();
    });
    webServer.onNotFound([]() {
      String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("AP mode", s));
    });
  }
  else {
    //Bat Server
    webServer.on("/", []() {
      String s = "<h1>STA mode</h1><p><a href=\"/reset\">Reset Wi-Fi Settings</a></p>";
      webServer.send(200, "text/html", makePage("STA mode", s));
    });
    webServer.on("/reset", []() {    // kiem tra duong dan"/reset" thi xoa EEPROM
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
      String s = "<h1>Wi-Fi settings was reset.</h1><p>Please reset device.</p>";
      webServer.send(200, "text/html", makePage("Reset Wi-Fi Settings", s));
    });
  }
  webServer.begin();
}
//-----------------Che do cai dat wifi cho esp8266----------------------
void setupMode() {
  WiFi.mode(WIFI_STA);            //che do hoat dong la May Tram Station
  WiFi.disconnect();              //ngat ket noi wifi
  delay(100);
  int n = WiFi.scanNetworks();    //quet cac mang wifi xung quanh xem co bao nhieu mang
  delay(100);
  for (int i = 0; i < n; ++i) {    //dua danh sach wifi vao list
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.mode(WIFI_AP);               // chuyen sang che dong Access point
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID,"12345678");              //thiet lap 1 open netword WiFi.softAP(ssid, password)
  dnsServer.start(53, "*", apIP);
  startWebServer();
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}
//------------Ham chuyen 2 che do Station mode va Access Point khi an giu nut 1 trong 5s----------//
void readResetButton1(){
  if(digitalRead(IN1) == 0){
    //Serial.println("start");
    if((settingTimeout + SETTING_HOLD_TIME) <= millis()){
      settingMode = true;                           // set che do cai dat lai thanh true
      setupMode();
    }
  }
  if(digitalRead(IN1)){
  settingTimeout = millis();
  //Serial.println("start 1");
  }
}
