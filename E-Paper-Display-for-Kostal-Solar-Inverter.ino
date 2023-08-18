#include <WiFi.h>
//Display includes
#include <GxEPD.h>
#include <GxGDEY027T91/GxGDEY027T91.h>    // 2.7" b/w
#include GxEPD_BitmapExamples
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

//define ESP Pinout
GxIO_Class io(SPI, /*CS=5*/ 26, /*DC=*/ 25, /*RST=*/ 33); // arbitrary selection of 17, 16
GxEPD_Class display(io, /*RST=*/ 33, /*BUSY=*/ 27); // arbitrary selection of (16), 4

const char* ssid = "xx";
const char* password = "yy";

const char* host = "xx";
uint16_t hostPort = yy;
uint32_t altzeit;
uint8_t bufferRead[50];
WiFiClient client;
bool tcpConnectionErrorFlag = false;
bool wifiConnectionErrorFlag = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Display setup");
  display.init(115200);
  display.setRotation(3);
  display.setTextColor(GxEPD_BLACK);
  Serial.println("Display setup done");

  while (WiFi.status() != WL_CONNECTED) {
    if(!wifiConnectionErrorFlag){
      drawError("Keine Verbindung!");
      display.update();
      display.powerDown();
      wifiConnectionErrorFlag = true;
    }
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  wifiConnectionErrorFlag = false;
  Serial.println("Wifi connected");
  
  drawBackground();
  drawStromerzeugung();
  drawHomeConsumption();
  drawBatterySOC();
  drawMonthlyYieldInEUR();
  if(tcpConnectionErrorFlag){
    drawError("TCP Error!");
  }
  display.update();
  display.powerDown();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() != WL_CONNECTED){
    if(!wifiConnectionErrorFlag){
      drawError("Keine Verbindung!");
      display.update();
      display.powerDown();
      wifiConnectionErrorFlag = true;
    }
    delay(1000);
    WiFi.disconnect();
    WiFi.reconnect();
  }
  else{
    wifiConnectionErrorFlag = false;
    if (millis() - altzeit >= 60000) {
      altzeit=millis();
      drawBackground();
      drawStromerzeugung();
      drawHomeConsumption();
      drawBatterySOC();
      drawMonthlyYieldInEUR();
      if(tcpConnectionErrorFlag){
        drawError("TCP Error!");
      }
      display.update();
      display.powerDown();
    }
  }
}

void drawError(char* error){
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(error, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center bounding box by transposition of origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  drawBackground();
  display.setCursor(x, y);
  display.print(error);
}

void drawStromerzeugung()
{
  display.fillRect(2, 2, 200, 36, GxEPD_WHITE);
  display.setCursor(4, 14);
  display.setFont(&FreeMonoBold9pt7b);
  display.print("Stromerzeugung");
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(4, 36);
  float stromerzeugung = readFloat(0x4, 0x2A) / 1000;
  display.print(stromerzeugung);
  display.print(" kW");
  //Serial.println("drawHelloWorld done");
}

void drawHomeConsumption()
{
  display.fillRect(2, 37, 200, 36, GxEPD_WHITE);
  display.setCursor(4, 56);
  display.setFont(&FreeMonoBold9pt7b);
  display.print("Hausverbrauch");
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(4, 78);
  float hausverbrauch = readFloat(0x0, 0x6A);
  hausverbrauch += readFloat(0x0, 0x6C);
  hausverbrauch += readFloat(0x0, 0x74);
  hausverbrauch /= 1000;
  display.print(hausverbrauch);
  display.print(" kW");
  //Serial.println("drawHelloWorld done");
}

void drawBatterySOC()
{
  display.fillRect(2, 79, 200, 36, GxEPD_WHITE);
  display.setCursor(4, 98);
  display.setFont(&FreeMonoBold9pt7b);
  display.print("Batteriefuellstand");
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(4, 120);
  display.print(readUint16(0x2, 0x2));
  display.print("%");
  //Serial.println("drawHelloWorld done");
}

void drawMonthlyYieldInEUR()
{
  display.fillRect(2, 121, 200, 36, GxEPD_WHITE);
  display.setCursor(4, 140);
  display.setFont(&FreeMonoBold9pt7b);
  display.print("Umsatz");
  display.setFont(&FreeMonoBold12pt7b);
  display.setCursor(4, 162);
  float umsatz = readFloat(0x1, 0x46) * 0.18 / 1000;
  display.print(umsatz);
  display.print(" EUR");
  //Serial.println("drawHelloWorld done");
}

void drawBackground(){
  display.fillScreen(GxEPD_WHITE);
  display.fillRect(0, 0, 264, 176, GxEPD_BLACK);
  display.fillRect(2, 2, 260, 172, GxEPD_WHITE);
}

int TCP_send(uint8_t* message) {
  Serial.printf("connecting to %s %d \n", host, hostPort);
  if (!client.connect(host, hostPort, 1000)) {
    Serial.println("connection failed");
    delay(5000);
    tcpConnectionErrorFlag = true;
    return -1;
  }
  tcpConnectionErrorFlag = false;
  Serial.println("sending data to server");
  client.write(message, 12);
  while(!client.available() && client.connected()){

  }
  if(client.available()){
    bufferRead[0] = client.available();
    for(int i = 1; i <= bufferRead[0]; i++){
      bufferRead[i] = client.read();
    }
  }
  client.stop();
  return 0;
}
float readFloat(uint8_t addrHIGH, uint8_t addrLOW){
  uint8_t message[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x47, 0x03, addrHIGH , addrLOW, 0x00, 0x02};
  int error = TCP_send(message);
  if(error == -1){

  }
  else{
    float f = bytesToFloat(bufferRead[10], bufferRead[11], bufferRead[12], bufferRead[13]);
    Serial.println(f);
    return f;
  }
  return 0.0;
}

uint16_t readUint16(uint8_t addrHIGH, uint8_t addrLOW){
  uint8_t message[] = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x47, 0x03, addrHIGH , addrLOW, 0x00, 0x01};
  int error = TCP_send(message);
  if(error == -1){

  }
  else{
    uint16_t ui = (bufferRead[10] << 8) + bufferRead[11];
    Serial.println(ui);
    return ui;
  }
  return 0;
}
float bytesToFloat(byte b0, byte b1, byte b2, byte b3)
{
  float f;
  byte b[] = {b3, b2, b1, b0};
  memcpy(&f, &b, 4);
  return f;
}