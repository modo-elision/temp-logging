#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include<EEPROM.h>
/*
#ifndef STASSID
#define STASSID "ESP_ACCESS_POINT"
#define STAPSK  "12345678"
#endif
*/
String ssid,password,new_ssid,new_password;
char check_ssid;
int new_input=0;
ESP8266WebServer server(80);

//Check if header is present and correct
bool is_authenticated() {
  Serial.println("Enter is_authenticated");
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
    if (cookie.indexOf("ESPSESSIONID=1") != -1) {
      Serial.println("Authentication Successful");
      return true;
    }
  }
  Serial.println("Authentication Failed");
  return false;
}

//login page, also called for disconnect
void set_wifi_page() {
  String msg;
  /*if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }*/
  if (server.hasArg("USERNAME")) {
    if (server.arg("USERNAME") == "admin") {
      new_ssid=server.arg("ssid");
      new_password=server.arg("password");
      new_input=1;
      Serial.println("value set");
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form  method='POST'>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Wifi Name:<input type='text' name='ssid' placeholder='wifi name'><br>";
  content += "Password:<input type='password' name='password' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  //content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

void handleLogin() {
  String msg;
  if (server.hasHeader("Cookie")) {
    Serial.print("Found cookie: ");
    String cookie = server.header("Cookie");
    Serial.println(cookie);
  }
  if (server.hasArg("DISCONNECT")) {
    Serial.println("Disconnection");
    server.sendHeader("Location", "/login");
    server.sendHeader("Cache-Control", "no-cache");
    server.sendHeader("Set-Cookie", "ESPSESSIONID=0");
    server.send(301);
    return;
  }
  if (server.hasArg("USERNAME") && server.hasArg("PASSWORD")) {
    if (server.arg("USERNAME") == "admin" &&  server.arg("PASSWORD") == "admin") {
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", "ESPSESSIONID=1");
      server.send(301);
      Serial.println("Log in Successful");
      return;
    }
    msg = "Wrong username/password! try again.";
    Serial.println("Log in Failed");
  }
  String content = "<html><body><form action='/login' method='POST'>To log in, please use : admin/admin<br>";
  content += "User:<input type='text' name='USERNAME' placeholder='user name'><br>";
  content += "Password:<input type='password' name='PASSWORD' placeholder='password'><br>";
  content += "<input type='submit' name='SUBMIT' value='Submit'></form>" + msg + "<br>";
  content += "You also can go <a href='/inline'>here</a></body></html>";
  server.send(200, "text/html", content);
}

//root page can be accessed only if authentication is ok
void handleRoot() {
  Serial.println("Enter handleRoot");
  String header;
  if (!is_authenticated()) {
    server.sendHeader("Location", "/");
    server.sendHeader("Cache-Control", "no-cache");
    server.send(301);
    return;
  }
  String content = "<html><body><H2>hello, you successfully connected to esp8266!</H2><br>";
  if (server.hasHeader("User-Agent")) {
    content += "the user agent used is : " + server.header("User-Agent") + "<br><br>";
  }
  content += "You can access this page until you <a href=\"/login?DISCONNECT=YES\">disconnect</a></body></html>";
  server.send(200, "text/html", content);
}

//no need authentication
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
String Readword(int addr){
  String wor;
  char readchar;
  int i=addr;
  while(readchar!='\0'){
    readchar = char(EEPROM.read(i));
    delay(10);
    i++;
    if(readchar!='\0'){
      wor+=readchar;
    }
    
  }
}
void change_to_STA(){
  int i=0;
    ssid=Readword(10);
    password=Readword(40);
    new_input=0;
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");
  
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      i++;
      Serial.print(".");
      if(i>10){
        EEPROM.write(2,'0');
      }
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); 
}

void setup(void) {
  int i=0;
  Serial.begin(115200);
  EEPROM.begin(512);
  check_ssid=EEPROM.read(2);
  if(check_ssid=='1'){
    change_to_STA();
  }
  else{
    WiFi.softAP("ESP_ACCESS_POINT","12345678");
    IPAddress IP=WiFi.softAPIP();
    Serial.println(IP);
    delay(1000);
  }
    server.on("/",set_wifi_page);
    //server.on("/", handleRoot);
    //server.on("/login", handleLogin);
    server.on("/inline", []() {
      server.send(200, "text/plain", "this works without need of authentication");
    });
  
    server.onNotFound(handleNotFound);
    //ask server to track these headers
    server.collectHeaders("User-Agent", "Cookie");
    server.begin();
    //Serial.println("HTTP server started");*/
  
}
void writeword(int addr,String wor){
  for(int i=0;i<wor.length();i++){
    EEPROM.write(i+addr,wor[i]);
  }
  EEPROM.write(wor.length()+addr,'\0');
  EEPROM.commit();
}

void loop(void) {
  
  server.handleClient();
  if(new_input==1){
    Serial.println("NEW input found");
    writeword(10,new_ssid);
    writeword(40,new_password);
    EEPROM.write(2,'1');
    EEPROM.commit();
    delay(10);
    change_to_STA();
  }
}
