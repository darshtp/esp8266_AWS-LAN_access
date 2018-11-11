/*
 * This code is at https://github.com/darshtp/esp8266_LAN-WAN_access
 * You may refer more data on github from @darshtp for IoT, Robotics and AI projects.
  :- For more details of this code, go to README.md file of esp8266_LAN-WAN_access repository @darshtp on github.
  :- I have used https://arest.io for WAN access and local IP for LAN access.
  :- In case aREST.h file is not supporting for MQTT connection than I will suggest to download it from https://www.arduinolibraries.info/libraries/a-rest
  :- For external IP address, I have used api.ipify.org
  :- HTML logic is also available for Web page in LAN.
  :- Smart Config mode is used to get SSID and Password of any available WiFi network automatically by smartphone App.
  :- This code mainly includes below mentioned functions.
    -Debounce Button (FLASH button of ESP8266_NodeMCU).
    -Public IP of ESP8266_NodeMCU after successfully connected to WiFi network.
    -Unique client ID of ESP8266_NodeMCU by it's own MAC address.
    -Enter and Exit from Smart Config mode for ESP8266_NodeMCU.
*/
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <time.h>
#include <PubSubClient.h>
#include <aREST.h>
#define PIN_LED 16// fix GPIO 16/digital pin D0 as PIN_LED
#define PIN_BUTTON 0// fix GPIO 0/digital pin D3 as PIN_BUTTON 
#define Relay 2 //fix GPIO 2/digital pin D4 as Relay
WiFiClient client;
WiFiClient espClient;
PubSubClient client1(espClient);
aREST arestVar = aREST(client1);
WiFiServer server(80);// set port number 80 for server
int ledState = HIGH;// At starting, set level of ledState HIGH
int button1State;             
int lastButton1State = LOW;// At starting, set level of lastButton1State LOW
unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;    
#define LED_ON() digitalWrite(PIN_LED, HIGH)
#define LED_OFF() digitalWrite(PIN_LED, LOW)
#define LED_TOGGLE() digitalWrite(PIN_LED, digitalRead(PIN_LED) ^ 0x01)
char* device_id = "******"; // Use any unique ID (keep it's size 6)
Ticker ticker;
String HTML;
void arest_cloud(){
  aREST arestVar = aREST(client1);
  void callback(char* topic, byte* payload, unsigned int length);}
bool longPress(){
  static int lastPress = 0;
  if (millis() - lastPress > 3000 && digitalRead(PIN_BUTTON) == 0) { //press the FLASH button of ESP8266_NodeMCU to enter in Smart Config mode
    return true;} 
  else if (digitalRead(PIN_BUTTON) == 1) {
    lastPress = millis();}
  return false;}
void tick(){
  int state = digitalRead(PIN_LED);  
  digitalWrite(PIN_LED, !state);}     
bool in_smartconfig = false;
void enter_smartconfig(){
  if (in_smartconfig == false) {
    in_smartconfig = true;
    ticker.attach(0.1, tick);
    WiFi.beginSmartConfig();}}
void exit_smart(){
  ticker.detach();
  LED_ON();
  in_smartconfig = false;}
void callback(char* topic, byte* payload, unsigned int length) {
  arestVar.handle_callback(client1, topic, payload, length);}
void GetExternalIP(){// function for Public IP/External IP address of ESP device
  WiFiClient client;
  if (!client.connect("api.ipify.org", 80)) {
    Serial.println("Failed to connect with 'api.ipify.org' !");}
  else {
    int timeout = millis() + 5000;
    client.print("GET /?format=json HTTP/1.1\r\nHost: api.ipify.org\r\n\r\n");
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;}}
    int size;
    while ((size = client.available()) > 0) {
      uint8_t* msg = (uint8_t*)malloc(size);
      size = client.read(msg, size);
      Serial.write(msg, size);
      free(msg);}}}
String macToStr(const uint8_t* mac){// function for generating unique client ID of ESP device
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';}
  return result;}
void setup() {
  Serial.begin(115200);
  String clientName;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  clientName += macToStr(mac);
  clientName += "-";
  clientName += String(micros() & 0xff, 16);
  Serial.println("\n***  MQTT client ID= " + clientName + "  ***\n");
  client1.setCallback(callback);
  arestVar.set_id(device_id);
  arestVar.set_name("*********");// use unique name for your ESP device(can take any size) 
  char* out_topic = arestVar.get_topic();
  Serial.setDebugOutput(true);
  WiFi.mode(WIFI_STA);
  pinMode(PIN_LED, OUTPUT);
  pinMode(Relay, OUTPUT);  
  pinMode(PIN_BUTTON, INPUT);
  ticker.attach(1, tick);
  Serial.println("Setup done");
  server.begin();}
void loop() {
  if (longPress()) {
    enter_smartconfig(); //  For ESP8266 to enter in smart config mode
    Serial.println("Enter smartconfig");} 
  if (WiFi.status() == WL_CONNECTED && in_smartconfig && WiFi.smartConfigDone()) {
    exit_smart();//  For ESP8266 to exit from smart config mode
    Serial.println("Connected, Exit smartconfig");
    Serial.println("Station Mode");
    Serial.print("LocalIP:"); Serial.println(WiFi.localIP());
    Serial.println("MAC:" + WiFi.macAddress());
    Serial.print("Gateway:"); Serial.println(WiFi.gatewayIP());
    Serial.print("STA MAC:"); Serial.println(WiFi.BSSIDstr());
    GetExternalIP();} // To get public IP of ESP8266
  if (WiFi.status() == WL_CONNECTED) {
    arestVar.loop(client1);
    arest_cloud();} 
  int  reading1 = digitalRead(PIN_BUTTON);
  if (reading1 != lastButton1State){
    lastDebounceTime = millis();
    Serial.print("t1");} // testing point 1
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading1 != button1State) { 
      button1State = reading1;
      Serial.print("t2"); // testing point 2
      if (button1State == HIGH) {
        ledState = !ledState;
        Serial.print("t3\n"); // testing point 3
        digitalWrite(Relay, ledState);}}}
  lastButton1State = reading1;
  WiFiClient client = server.available();
  if (!client) {return;}
  while(!client.available()){
    delay(1);
    break;}
  String req = client.readStringUntil('\r');
  client.flush();
  if (req.indexOf("/led/off") != -1){
    digitalWrite(Relay, LOW); 
    ledState = LOW;
    Serial.print("relay OFF\n");}
  else if (req.indexOf("/led/on") != -1){
    digitalWrite(Relay, HIGH); 
    ledState = HIGH;
    Serial.print("relay ON\n");}
  else if (req.indexOf("/") != -1)
    ; 
  else {
    client.stop();
    return;}
  client.flush();
  HTML = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html> <h1>Local IP for LAN access</h1> <h2>ESP8266 Web Server</h2>";
  HTML += "<h3>Relay is now ";
  HTML += (digitalRead(Relay))?"ON":"OFF</h3>"; //High = LED ON and Low = LED OFF
  HTML += "<h3>Relay <button onclick=\"window.location.href='/led/on'\">LED ON</button>&nbsp;<button onclick=\"window.location.href='/led/off'\">LED OFF</button></h3></html>";
  Serial.print("button pressed\n");
  client.print(HTML);
  delay(10);}
