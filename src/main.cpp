#include <Arduino.h>
#include <aliIotLink.h>
#include <ESP8266WiFi.h>
#include <OneButton.h>

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#define LED_PIN 2
#define PAIR_KEY 0

const char *ssid = "dd-wrt-2.4g";
const char *password = "Youyou66*#";
static String mac = "";
static String ProductKey = "a19auuPraka";
static String DeviceName = "";
static String DeviceSecret = "YLIPziYCqZTg6GhmvAI8OWQv77hGXdsZ";

static String postTopic = "user/update";        //上报消息topic
static String errorTopic = "user/update/error"; //上报消息topic
static String getTopic = "user/get";            //服务器消息topic
WiFiClient espClient;                           //实例化 wifi网络
PubSubClient client(espClient);                 //将网络传入MQTT
AliIotLink aliLink(client);                     //将mqtt传入服务
OneButton button(PAIR_KEY, true);
static WiFiEventHandler e1, e2, e3;
bool wifiConnected = false;

//回调函数
void Callbacks(char *topic, byte *payload, uint8_t length)
{
  //回调功能演示，将监听到的消息直接打印到串口
  Serial.print(topic);
  Serial.print(":");
  for (uint8_t i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void onSTAConnected(WiFiEventStationModeConnected ipInfo)
{
  DEBUG_PRINT("Connected to %s\r\n", ipInfo.ssid.c_str());
}

// Start NTP only after IP network is connected
void onSTAGotIP(WiFiEventStationModeGotIP ipInfo)
{
  DEBUG_PRINT("Got IP: %s\r\n", ipInfo.ip.toString().c_str());
  DEBUG_PRINT("Connected: %s\r\n", WiFi.status() == WL_CONNECTED ? "yes" : "no");
  wifiConnected = true;
  aliLink.subTopic(getTopic);                          //订阅服务器下行消息
  aliLink.setCallback(Callbacks);                      //注册下发消息回调函数
  aliLink.begin(DeviceName, ProductKey, DeviceSecret); //完成初始化配置 三元素(DeviceName,ProductKey,DeviceSecret)
  //aliLink.begin(FPSTR(DeviceName), FPSTR(ProductKey), FPSTR(DeviceSecret)); //完成初始化配置 三元素(DeviceName,ProductKey,DeviceSecret)
}

// Manage network disconnection
void onSTADisconnected(WiFiEventStationModeDisconnected event_info)
{
  DEBUG_PRINT("Disconnected from SSID: %s\n", event_info.ssid.c_str());
  DEBUG_PRINT("Reason: %d\n", event_info.reason);
  //digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  wifiConnected = false;
}

//长按3秒种进入配网模式
void smart_config()
{
  WiFi.mode(WIFI_STA);
  WiFi.beginSmartConfig();
  //Wait for SmartConfig packet from mobile
  Serial.println("\r\nWaiting for SmartConfig.");
  while (!WiFi.smartConfigDone())
  {
    delay(200);
    //Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  Serial.println("");
  Serial.println("SmartConfig Received.");
  //Wait for WiFi to connect to AP
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  Serial.println("\r\nWiFi Connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_PIN, 0);
  // delay(1000);
  //Serial.println("Reset CPU");
  //ESP.restart();
}
void longPress()
{
  DEBUG_PRINT("Long Press...");
  smart_config();
}
void shortPress()
{
  DEBUG_PRINT("Short Press...");
}
void setup()
{

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); //配置为客户端模式
  //WiFi.begin(ssid, password); //初始化并且链接wifi
  WiFi.begin(); //初始化并且链接wifi
  pinMode(LED_PIN, OUTPUT);
  button.setPressTicks(2000);
  button.attachLongPressStart(longPress);
  button.attachClick(shortPress);
  //button.attachClick(shortPress);
  //button.attachDoubleClick(doubleClick);
  mac = WiFi.macAddress();
  mac.toLowerCase();
  DeviceName = mac;
  postTopic = "/" + ProductKey + "/" + DeviceName + "/" + postTopic;
  getTopic = "/" + ProductKey + "/" + DeviceName + "/" + getTopic;
  errorTopic = "/" + ProductKey + "/" + DeviceName + "/" + errorTopic;

  DEBUG_PRINT("\nmac:%s\r\n", mac.c_str());
  DEBUG_PRINT("postTopic:%s\r\n", postTopic.c_str());
  DEBUG_PRINT("getTopic:%s\r\n", getTopic.c_str());
  DEBUG_PRINT("errorTopic:%s\r\n", errorTopic.c_str());
  e1 = WiFi.onStationModeGotIP(onSTAGotIP); // As soon WiFi is connected, start NTP Client
  e2 = WiFi.onStationModeDisconnected(onSTADisconnected);
  e3 = WiFi.onStationModeConnected(onSTAConnected);
}
void loop()
{
  button.tick();
  if (wifiConnected)
  {
    if (aliLink.state() != 0) // 显示连接错误码，实际项目不需要
    {
      DEBUG_PRINT("error id:%d", aliLink.state());
    }
    aliLink.loop(); //循环维持心跳与消息触发，应尽可能多的执行
  }
}
