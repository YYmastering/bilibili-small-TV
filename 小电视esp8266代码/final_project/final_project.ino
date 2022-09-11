#include <ESP8266_Seniverse.h>
#include <Forecast.h>
#include <LifeInfo.h>
#include <WeatherNow.h>

/***************************************************************************************************************************************
项目名称                : ESP8266天气站+微信小程序控制
作者                   : YangYi
日期                   : 2022/9/7
程序功能                : 一键配网,自动获取时间,天气以及气温,留言板功能,微信小程序控制并可以填写留言 
代码参考                :  1.太极创客物联网教程          http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-tuttorial/
                          2.b站up主:_铁甲依旧在_   https://www.bilibili.com/video/BV1hP4y1x7gL?share_source=copy_pc
****************************************************************************************************************************************
***********头文件*************/
#include <ESP8266WiFi.h>     
#include <WiFiUdp.h>     
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include <ESP8266_Seniverse.h>       
#include "ESP8266_BiliBili.h"
#include <time.h>
#include <sys/time.h>
#include <coredecls.h>
#include <ESP8266HTTPClient.h>

#include "OLEDDisplayUi.h"
#include <OLEDDisplay.h>
#include <Wire.h>
//#include "SH1106Wire.h"            (1.3寸)                         
#include <SSD1306Wire.h> //0.9寸用这个
#include "images.h"
#include "WeatherStationFonts.h"
#include "DrawPicture.h"

/************************************************************************************/
// 心知天气API请求所需信息
// 请对以下信息进行修改，填入您的心知天气私钥以及需要获取天气信息的城市和温度单位
// 如需进一步了解心知天气API所提供的城市列表等信息，请前往心知天气官方产品文档网址：
// https://www.seniverse.com/docs
String reqUserKey = "SxWl6jNcCu90w5Iy8";   // 私钥
String reqLocation = "changde";            // 城市
String reqUnit = "c";                      // 摄氏/华氏

WiFiUDP Udp;//创建udp对象
char incomingPacket[255];  // 保存接受到的信息
 int tal=0;//标志位
 int tal2=0;
String lyb="";//留言板字符串
WeatherNow weatherNow;                     // 建立WeatherNow对象用于获取心知天气信息
Forecast forecast;                         // 建立Forecast对象用于获取心知天气信息
LifeInfo lifeInfo;                         // 建立Forecast对象用于获取生活指数信息

/******************************************************************************************/
// 哔哩哔哩HTTP请求所需信息
String mid = "335597209";                   // 哔哩哔哩用户uid361778729
UpInfo upInfo("335597209");                 // 建立对象用于获取粉丝信息
                                            // 括号中的参数是B站的UUID
                                         
const char* host = "api.bilibili.com";     // 将要连接的服务器地址  
const int httpPort = 80;                   // 将要连接的服务器端口      

String reqRes = "/x/space/arc/search?mid=" + mid +"&pn=1&ps=10&order=pubdate&jsonp=jsonp";           
                                           // b站视频播放数请求网址
long numberoffans = -1;
long numberofplay = -1;

#define NUMBEROFVIDEOS           15
bool bilibili_first = true;
char Bilibili_Ate;
/*************************************中断定义***********************************************/
const byte interruptPin = 5;                                //用5号引脚作为中断触发引脚

volatile int value = -1;
int value_2 =-1;
const unsigned long DELAY_PERIOD = 20000;

void drawtest(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y); //提前声明




void callback() //返回数据函数
{
   
  Udp.beginPacket(Udp.remoteIP(),atoi(incomingPacket));//配置远端ip地址和端口
  Udp.write("linked"); //把数据写入发送缓冲区
  Udp.endPacket(); //发送数据 
  Serial.printf("yes");
  delay(50);
}

/******************************************************************************************

****************************************************************************显示定义****************************************************************************************************/         
FrameCallback frames[] = {drawTime, drawCurrentWeather, drawForecastWeather, drawtest, drawMoveImage};     //创建FrameCallback给ui使用，Frame的作用可以让ui来切换不同的frame，绘制不同的画面
int frameCount = 5;         

//OverlayCallback overlays[] = { drawHeaderOverlay };      // 覆盖回调函数
//int numberOfOverlays = 1;  //覆盖数
        
bool first = true;                                         // 首次更新标志
int xunhuan=0;
const unsigned long  BILIBILI_UPDATE_PERIOD = 1*60;        // B站信息更新时间间隔
const unsigned long  WEATHER_UPDATE_PERIOD = 5*60;         // 天气信息更新时间间隔
long timeSinceLastBilibiliUpdate = 0;                      // 上次更新后的时间
long timeSinceLastWeatherUpdate = 0;                       // 上次天气更新后的时间
/***************************************************************************************************************************************************************************************/
void ICACHE_RAM_ATTR InterruptHandle();                    //提前声明中断函数

//清空upd数据
void qing()
{
  for(int i=0;i<255;i++)
  {
  incomingPacket[i]=0;
  }
  }                                       
void setup() 
{
  
    Serial.begin(9600);       
    
    display.init();                                               // 屏幕初始化

    display.setFont(ArialMT_Plain_10);                            // 设置字体字号
    
    display.clear();
    display.display();                                            // 清屏
    
    ui.setTargetFPS(30);                                          // 刷新频率30帧每秒

    ui.disableAllIndicators();                                    // 开启frame标签//（已修改）
    ui.disableAutoTransition();                                   // 开启自动切换frame功能//（已修改）
    ui.enableAllIndicators();                                     // 显示指示器

  //ui.setActiveSymbol(activeSymbole);                            // 设置活动符号（不知道干什么用）
  //ui.setInactiveSymbol(inactiveSymbole);                        // 设置非活动符号（不知道干什么用）
  
    
    ui.setIndicatorPosition(BOTTOM);                              // 符号位置，你可以把这个改成TOP, LEFT, BOTTOM, RIGHT
  
    ui.setIndicatorDirection(LEFT_RIGHT);                         // 定义第一帧在栏中的位置
  
    ui.setFrameAnimation(SLIDE_LEFT);                             // 屏幕切换方向，您可以更改使用的屏幕切换方向 SLIDE_LEFT, SLIDE_RIGHT, SLIDE_TOP, SLIDE_DOWN
    
    ui.setFrames(frames,frameCount);                              // 设置框架
    
    ui.init();
    display.flipScreenVertically();                               // 屏幕翻转
    
    pinMode(interruptPin, INPUT_PULLUP);                          // 将中断触发引脚（2号引脚）设置为INPUT_PULLUP（输入上拉）模式
    
    attachInterrupt(digitalPinToInterrupt(interruptPin), InterruptHandle, RISING);    // 设置中断触发程序



    connectWiFi();                                                //连接WiFi(自动配网函数)
    
    configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
    delay(200);


    // 配置心知天气请求信息（实时天气、预报天气、生活建议都要配置！！！）
    weatherNow.config(reqUserKey, reqLocation, reqUnit);
    forecast.config(reqUserKey, reqLocation, reqUnit);
    lifeInfo.config(reqUserKey, reqLocation, reqUnit);

//启用UDP接收数据
if(Udp.begin(1234)){//启动Udp监听服务，监听本地localUdpPort端口
    Serial.println("监听成功");
    //WiFi.localIP().toString().c_str()用于将获取的本地IP地址转化为字符串    
    Serial.printf("现在收听IP：%s, UDP端口：%d\n", WiFi.localIP().toString().c_str(), 1234);
  }else{
    Serial.println("监听失败");
  }
    
}

//自动配网
void connectWiFi()
{
    
    WiFiManager wifiManager;                                                          // 建立WiFiManager对象 
                                                                                      
    if(!wifiManager.autoConnect("我是呆瓜呀"))                                         // 自动连接WiFi。以下语句的参数是连接ESP8266时的WiFi名称，无需密码
    {
        display.drawXbm(display.width()/2 - Sad_Person_width/2,display.height()-Sad_Person_height,Sad_Person_width, Sad_Person_height, Sad_Person);
        display.drawXbm(8, 0, Character_1_width, Character_1_height, CHARACTER_1[0]);  /*没*/
        display.drawXbm(24, 0, Character_1_width, Character_1_height, CHARACTER_1[1]); /*有*/
        display.drawXbm(40, 0, Character_1_width, Character_1_height, CHARACTER_1[2]); /*网*/
        display.drawXbm(56, 0, Character_1_width, Character_1_height, CHARACTER_1[3]); /*，*/
        display.drawXbm(72, 0, Character_1_width, Character_1_height, CHARACTER_1[4]); /*不*/
        display.drawXbm(88, 0, Character_1_width, Character_1_height, CHARACTER_1[5]); /*开*/
        display.drawXbm(104, 0, Character_1_width, Character_1_height, CHARACTER_1[6]);/*心*/
        
        display.display();
        wifiManager.autoConnect("我是呆瓜呀");
    }
    
    // 如果您希望该WiFi添加密码，可以使用以下语句：
    // wifiManager.autoConnect("AutoConnectAP", "12345678");
    // 以上语句中的12345678是连接AutoConnectAP的密码
    
    // WiFi连接成功后将通过串口监视器输出连接成功信息 
    Serial.println(""); 
    Serial.print("ESP8266 Connected to ");
    Serial.println(WiFi.SSID());                // WiFi名称
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());             // IP
}


//自动换界面函数
//void autoqiehuan()
//{
//  ui.switchto(1)
//  }


// 更新天气信息
void WeatherUpdate()
{
  if(weatherNow.update()&&lifeInfo.update()){                                  
    Serial.println(F("======Weahter Info======"));
    Serial.print("Server Response: ");
    Serial.println(weatherNow.getServerCode());                                // 获取服务器响应码
    Serial.print(F("Weather Now: "));
    Serial.print(weatherNow.getWeatherText());                                 // 获取当前天气（字符串格式）
    currentweather.weather_text = weatherNow.getWeatherText();
    Serial.print(F(" "));
    Serial.println(weatherNow.getWeatherCode());                               // 获取当前天气状态码（整数格式）
    currentweather.weather_code = weatherNow.getWeatherCode();
    Serial.print(F("Temperature: "));
    Serial.println(weatherNow.getDegree());                                    // 获取当前温度数值
    currentweather.degree = weatherNow.getDegree();
    Serial.print(F("Dressing: "));
    Serial.println(lifeInfo.getDressing());                                    // 获取穿衣建议
    currentweather.dressing = lifeInfo.getDressing();
    Serial.print(F("Last Update: "));
    Serial.println(weatherNow.getLastUpdate());                                // 获取服务器更新天气信息时间
    currentweather.lastupdate = weatherNow.getLastUpdate();
    Serial.println(F("========================"));     
    } 
    else {                                                                     // 更新失败
    Serial.println("WeatherNow Update Fail...");   
    Serial.print("Server Response: ");                                         // 输出服务器响应状态码供用户查找问题
    Serial.println(weatherNow.getServerCode());                                // 心知天气服务器错误代码说明可通过以下网址获取
    }                                                                          // https://docs.seniverse.com/api/start/error.html
  
    if(forecast.update()){                                                     // 更新天气信息
    for(int i = 0; i < 3; i++){
      Serial.print(F("========Day ")); 
      Serial.print(i);      
      Serial.println(F("========"));     

      Serial.print(F("Day Weather: "));
      Serial.print(forecast.getDate(i));                                       //获取日期(字符串格式)
      String Datereturn = forecast.getDate(i);
      forecastweather[i].date = Datereturn.substring(5, Datereturn.length());
      
      Serial.print(F("Day Weather: "));
      Serial.print(forecast.getDayText(i));                                    //获取白天天气(字符串格式)
      forecastweather[i].weather_text = forecast.getDayText(i);
      
      Serial.print(F(" "));
      Serial.println(forecast.getDayCode(i));                                 //获取白天天气状态码(整数格式)
      forecastweather[i].weather_code = forecast.getDayCode(i);
      
      Serial.print(F("High: "));
      Serial.print(forecast.getHigh(i));                                      //获取最高气温(整数格式) 
      forecastweather[i].max_temperature = forecast.getHigh(i);
      if(i == 0)
      {
         currentweather.max_temperature = forecast.getHigh(i);
      }
      Serial.println(F("°C"));     
      Serial.print(F("LOW: "));
      Serial.print(forecast.getLow(i));                                       //获取最低气温(整数格式)  
      forecastweather[i].min_temperature = forecast.getLow(i);
      if(i == 0)
      {
         currentweather.min_temperature = forecast.getLow(i);
      }
      Serial.println(F("°C"));
      Serial.print(F("Rainfall: "));                                          //获取降水概率信息(小数格式)
      Serial.print(forecast.getRain(i));  
      forecastweather[i].probability = forecast.getRain(i);
      Serial.println(F("%"));
      
      Serial.print(F("Last Update: "));                                       //获取心知天气信息更新时间(字符串格式)       
      Serial.println(forecast.getLastUpdate());                
    }
    Serial.print(F("Server Code: ")); 
    Serial.println(forecast.getServerCode()); 
    Serial.println(F("====================="));   
  } else {    // 更新失败
    Serial.println("Forecast Update Fail...");   
    Serial.print("Server Response: ");                                        // 输出服务器响应状态码供用户查找问题
    Serial.println(weatherNow.getServerCode());                               // 心知天气服务器错误代码说明可通过以下网址获取
  }                                                                           // https://docs.seniverse.com/api/start/error.html
  delay(5000);
}

// 向bilibili服务器请求视频播放信息并对信息进行解析
void Bilibili_Play_Request(void)
{
    WiFiClient client;
    int i = 0;
    int len;
    String httpRequest = String("GET ") + reqRes + " HTTP/1.1\r\n" +          // 建立http请求信息
                                "Host: " + host + "\r\n" + 
                                "Connection: close\r\n\r\n";
    Serial.println(""); 
    Serial.print("Connecting to "); Serial.print(host);
 
    if (client.connect(host, 80))                                             // 尝试连接服务器    
    {                                                    
      Serial.println(" Success!");

      client.print(httpRequest);                                              // 向服务器发送http请求信息
      Serial.println("Sending request: ");
      Serial.println(httpRequest);  

      String status_response = client.readStringUntil('\n');                  // 获取并显示服务器响应状态行
      Serial.print("status_response: ");
      Serial.println(status_response);
   
      if (client.find("\r\n\r\n"))                                            // 使用find跳过HTTP响应头
      {
        Serial.println("Found Header End. Start Parsing.");
      } 
      if(!bilibili_first)
      {
        client.readStringUntil(Bilibili_Ate); 
        parseInfo_1(client);                          // 利用ArduinoJson库解析响应信息
      }
      if(bilibili_first)
      {
        String temp = client.readStringUntil('{'); 
        Serial.println(temp);
        unsigned int len = temp.length();
        char p[5];
        strcpy(p,temp.c_str());
        Bilibili_Ate = p[2];
        Serial.println(Bilibili_Ate);

        char qc ={'e'};
        if (Bilibili_Ate == 'e')
        {
          Serial.println("right!!!");
        }
        //client.readStringUntil(Bilibili_Ate);
        bilibili_first = false;
      }  
                                                     
   } 
   
   else
   {
      Serial.println(" connection failed!");
   }     
   
  client.stop();                                                              // 断开客户端与服务器连接工作
}

// 利用ArduinoJson库解析响应信息
void parseInfo_1(WiFiClient client){
  const size_t capacity = JSON_ARRAY_SIZE(NUMBEROFVIDEOS)+2*JSON_OBJECT_SIZE(2) +3*JSON_OBJECT_SIZE(3)+JSON_OBJECT_SIZE(4) +NUMBEROFVIDEOS*JSON_OBJECT_SIZE(21) +550*NUMBEROFVIDEOS;
  DynamicJsonDocument doc(capacity); 
  deserializeJson(doc, client);
  
  int code = doc["code"]; 
  const char* message = doc["message"]; 
  int ttl = doc["ttl"]; 

  JsonObject data = doc["data"];
  JsonArray data_list_vlist = data["list"]["vlist"];
  int i = 0;
  numberofplay = 0;
  for(i;i<NUMBEROFVIDEOS;i++)
  {
    JsonObject data_list_vlist_i = data_list_vlist[i];
    long data_list_vlist_i_play = data_list_vlist_i["play"].as<long>();
    numberofplay = numberofplay+data_list_vlist_i_play;
    Serial.print("Play is ");
    Serial.println(String(numberofplay));
    Serial.println(String(client));
    //Serial.println(doc);
  }
  Serial.print("Bilibili Play: ");   
  Serial.println(String(numberofplay));    
}

void bilibiliUpdate()
{
    FansInfo fansInfo(mid);   
    if(fansInfo.update())                                                     // 更新信息成功   
    { 
        Serial.println("Update OK"); 
        Serial.print("Server Response: ");  
        Serial.println(fansInfo.getServerCode());     
        Serial.print(F("Fans Number: "));
        Serial.println(fansInfo.getFansNumber());
        numberoffans = fansInfo.getFansNumber();
    } 
    else                                                                      // 更新失败
    {    
        Serial.println("Bilibili Fans_Update Fail...");  
        Serial.print("Server Response: ");   
        Serial.println(upInfo.getServerCode());    
    }

    Bilibili_Play_Request();                                                  // 请求b站视频播放数
    
    Serial.println(F("======================"));  
}







/*

循环执行

*/
void loop() 
{
//监听UDP接受到的数据
    delay(10);//解析Udp数据包
  int packetSize = Udp.parsePacket();//获得解析包
  if (packetSize)//解析包不为空，代表接收到了数据
  {
    int len = Udp.read(incomingPacket, 255);//返回数据包字节数
    if (len > 0)
    { 
      incomingPacket[len] = 0;//清空缓存
      Serial.printf("UDP数据包内容为: %s\n", incomingPacket);//向串口打印信息
    }
  }     



  
   if(first)                                                                  // 首次加载，即开机
   {
      display.clear();
      display.display(); 
      drawStartImage();                                                       // 绘制开机画面
      first = false;   
      
      WeatherUpdate();
      timeSinceLastWeatherUpdate = millis();                                  // 更新天气信息
      bilibiliUpdate();
      timeSinceLastBilibiliUpdate = millis();                                 // 更新b站信息

      ui.switchToFrame(0);                                                    // 切换到第0帧，及显示时间
      Serial.println(String(timeSinceLastBilibiliUpdate));
      Serial.println("Successfully booted !");
   }
   if (millis() - timeSinceLastWeatherUpdate > (1000L * WEATHER_UPDATE_PERIOD))  //定时更新天气信息（每五分钟更新一次）
   { 
      WeatherUpdate();
      timeSinceLastWeatherUpdate = millis();
      Serial.println(String( timeSinceLastWeatherUpdate));
   }
   if (millis() - timeSinceLastBilibiliUpdate > (1000L * BILIBILI_UPDATE_PERIOD))//定时更新b站信息（每一分钟更新一次）
   { 
      bilibiliUpdate();
      timeSinceLastBilibiliUpdate = millis();
      Serial.println(String( timeSinceLastBilibiliUpdate));
   } 
   int remainingTimeBudget = ui.update();                                    // 帧速率由ui控制，更新完ui会返回下一帧需要等待的时间
   if(remainingTimeBudget > 0)                                               // 延迟对应的时间后可再次更新屏幕   
   {
     delay(remainingTimeBudget);
   }


//接受小程序端的指令

if(strcmp(incomingPacket, "ready") == 0){ tal=1;qing() ;Serial.println("tal=1");}
if(tal==1&&incomingPacket[0]!=0){Serial.println("bingo");qing();tal=2;}
if(tal==2&&incomingPacket[0]!=0){Serial.println("端口");callback();qing();tal=0;}

if(strcmp(incomingPacket, "liuyan") == 0){ tal2=1;qing() ;Serial.println("tal2=1");}
if(tal2==1&&incomingPacket[0]!=0){lyb=incomingPacket;Serial.printf("留言：%s",lyb);qing();tal2=0;}
if(strcmp(incomingPacket, "wen") == 0) {  xunhuan=11;qing();Serial.println("duile"); }
if(strcmp(incomingPacket, "time") == 0) {   xunhuan=47;qing();Serial.println("duile");}
if(strcmp(incomingPacket, "fore") == 0) {   xunhuan=23;qing();Serial.println("duile");}
if(strcmp(incomingPacket, "liu") == 0) {   xunhuan=35;qing();Serial.println("duile");}

xunhuan++;
//自动切换每一帧
if(xunhuan%48==0)  ui.switchToFrame(0);
if(xunhuan%48==1)  ui.switchToFrame(0);
if(xunhuan%48==2)  ui.switchToFrame(0);
if(xunhuan%48==3)  ui.switchToFrame(0);
if(xunhuan%48==4)  ui.switchToFrame(0);
if(xunhuan%48==5)  ui.switchToFrame(0);
if(xunhuan%48==6)  ui.switchToFrame(0);
if(xunhuan%48==7)  ui.switchToFrame(0);
if(xunhuan%48==8)  ui.switchToFrame(0);
if(xunhuan%48==9)  ui.switchToFrame(0);
if(xunhuan%48==10)  ui.switchToFrame(0);  
if(xunhuan%48==11)  ui.switchToFrame(0);
if(xunhuan%48==12)  ui.switchToFrame(1);
if(xunhuan%48==13)  ui.switchToFrame(1);
if(xunhuan%48==14)  ui.switchToFrame(1);
if(xunhuan%48==15)  ui.switchToFrame(1);
if(xunhuan%48==16)  ui.switchToFrame(1);
if(xunhuan%48==17)  ui.switchToFrame(1);
if(xunhuan%48==18)  ui.switchToFrame(1);
if(xunhuan%48==19)  ui.switchToFrame(1);
if(xunhuan%48==20)  ui.switchToFrame(1);
if(xunhuan%48==21)  ui.switchToFrame(1);
if(xunhuan%48==22)  ui.switchToFrame(1);  
if(xunhuan%48==23)  ui.switchToFrame(1);


if(xunhuan%48==24)  ui.switchToFrame(2);
if(xunhuan%48==25)  ui.switchToFrame(2);
if(xunhuan%48==26)  ui.switchToFrame(2);
if(xunhuan%48==27)  ui.switchToFrame(2);
if(xunhuan%48==28)  ui.switchToFrame(2);
if(xunhuan%48==29)  ui.switchToFrame(2);
if(xunhuan%48==30)  ui.switchToFrame(2);
if(xunhuan%48==31)  ui.switchToFrame(2);
if(xunhuan%48==32)  ui.switchToFrame(2);
if(xunhuan%48==33)  ui.switchToFrame(2);
if(xunhuan%48==34)  ui.switchToFrame(2);  
if(xunhuan%48==35)  ui.switchToFrame(2);

if(xunhuan%48==36)  ui.switchToFrame(3);
if(xunhuan%48==37)  ui.switchToFrame(3);
if(xunhuan%48==38)  ui.switchToFrame(3);
if(xunhuan%48==39)  ui.switchToFrame(3);
if(xunhuan%48==40)  ui.switchToFrame(3);
if(xunhuan%48==41)  ui.switchToFrame(3);
if(xunhuan%48==42)  ui.switchToFrame(3);
if(xunhuan%48==43)  ui.switchToFrame(3);
if(xunhuan%48==44)  ui.switchToFrame(3);
if(xunhuan%48==45)  ui.switchToFrame(3);
if(xunhuan%48==46)  ui.switchToFrame(3);  
if(xunhuan%48==47)  ui.switchToFrame(3);
  
 
   delay(200);
}


//中断处理函数
void ICACHE_RAM_ATTR InterruptHandle()
{
 delayMicroseconds(DELAY_PERIOD);
 if(digitalRead(interruptPin) == LOW )
 {
    value++;
    delayMicroseconds(1000000);
    if(digitalRead(interruptPin) == LOW )//延时消抖
    {
      value = 4;
    }
    value_2 = value;
 }
 if(value == 0)
 {
  ui.enableAllIndicators();
  ui.switchToFrame(0);                                 // 绘制时间页面
 }
  if(value == 1)
 {
  ui.enableAllIndicators();
  ui.switchToFrame(1);                                 // 绘制当日天气页面
 }
 if(value == 2)
 {
  ui.enableAllIndicators();                            
  ui.switchToFrame(2);                                 // 绘制天气预报页面
 }
 if(value == 3)
 {
  value = -1;
  ui.disableAllIndicators();                           // 绘制b站信息页面
  ui.switchToFrame(3);
 }
 if(value == 4)
 {
  value = -1;
  ui.disableAllIndicators();                           // 绘制隐藏画面
  ui.switchToFrame(4);
 }
 delayMicroseconds(50000);
}


//显示留言信息
void drawtest(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
  display->setFont(ArialMT_Plain_16);
  display->setTextAlignment(TEXT_ALIGN_CENTER);  
  display->drawString(25, 0, "Informing:");
  
  display->drawString(55, 20, lyb);
 // display->drawXbm(0,0,120,60, drawT);
  }
