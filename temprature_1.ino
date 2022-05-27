#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2            // 定义DS18B20数据口连接STM32 PA10 
OneWire oneWire(ONE_WIRE_BUS);    // 初始连接在单总线上的单总线设备
DallasTemperature sensors(&oneWire);
 
void setup(){
  Serial.begin(115200);             // 设置串口通信波特率
  sensors.begin();                // 初始库
}
 
void loop(void){ 
  sensors.requestTemperatures();  // 发送命令获取温度
  Serial.print("温度值：");          //串口打印温度值
  Serial.print(sensors.getTempCByIndex(0)); 
  Serial.println("℃");
  delay(10);
}
