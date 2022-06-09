#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "edp.c"
#include "config.c"

#define WIFI_UART   Serial

edp_pkt* pkt;
int tick = tick_round;
int state = 0;
char cstate[20];

OneWire oneWire(ONE_WIRE_BUS);    // 初始连接在单总线上的单总线设备
DallasTemperature sensors(&oneWire);

void connectONENET(){
    while (!doCmdOk("AT", "OK"));
    while (!doCmdOk("AT+CWMODE=3", "OK"));            //工作模式
    while (!doCmdOk("AT+CWJAP=\"123\",\"lfc12345\"", "OK")); //wifiname:123, pswd: lfc12345
    while (!doCmdOk("AT+CIPSTART=\"TCP\",\"jjfaedp.hedevice.com\",876", "OK"));//onenet连接点
    while (!doCmdOk("AT+CIPMODE=1", "OK"));           //透传模式
    while (!doCmdOk("AT+CIPSEND", ">"));              //开始发送
}

void setup(){
    pinMode(wifi_pin, OUTPUT); //wifi指示灯引脚
    pinMode(lock_pin, OUTPUT);//物联网控制引脚
    pinMode(close_pin,OUTPUT);//门锁开关引脚
    pinMode(open_pin,OUTPUT);
    pinMode(smoke_pin, INPUT);
    pinMode(fire_pin,OUTPUT);
    pinMode(password_pin, INPUT);
    digitalWrite(lock_pin, LOW);
    digitalWrite(wifi_pin, LOW); 

    WIFI_UART.begin(_baudrate);
    WIFI_UART.setTimeout(3000);    //设置find超时时间
    delay(3000);
    Serial.setTimeout(100);
    delay(2000);
    connectONENET();

    sensors.begin(); 
}

void loop(){
    static int edp_connect = 0;
    static bool trigger = true;
    edp_pkt rcv_pkt;
    unsigned char pkt_type;
    int i = 0, tmp;
    char num[10];

    float data1;
    int data1int, data2;
    char cdata1[20], cdata2[20];

    int passwordCorrect = digitalRead(password_pin);
    if(passwordCorrect==1){
        Open();
        delay(3000);
        Close();
    }

    data2 = analogRead(smoke_pin);
    if(fire_auto_open && data2>smoke_threshold && trigger){
        trigger = false;
        digitalWrite(fire_pin, HIGH); 
        //sprintf(cdata2, "%d", data2); //需要吗？
        //packetSend(packetDataSaveTrans(NULL, "data2", cdata2));
        Open();
    }
    if(fire_auto_open && data2<smoke_threshold_close){
        trigger = true;
        digitalWrite(fire_pin, LOW); 
    }

    /* EDP 连接 */
    if (!edp_connect){
        while (WIFI_UART.available()) WIFI_UART.read(); //清空串口接收缓存
        packetSend(packetConnect(ID, KEY));             //发送EPD连接包
        while (!WIFI_UART.available());                 //等待EDP连接应答
        if ((tmp = WIFI_UART.readBytes(rcv_pkt.data, sizeof(rcv_pkt.data))) > 0){
            rcvDebug(rcv_pkt.data, tmp);
            if (rcv_pkt.data[0] == 0x20 && rcv_pkt.data[2] == 0x00 && rcv_pkt.data[3] == 0x00){
                edp_connect = 1;
                digitalWrite(wifi_pin, HIGH);   // EDP连接状态
            }
        }
        packetClear(&rcv_pkt);
    }

    tick++;
    if (tick > tick_round && edp_connect){ //心跳包, 每50对应约8秒
        sensors.requestTemperatures();
        data1 = sensors.getTempCByIndex(0);
        data1=data1*100;
        data1int=int(data1);
        sprintf(cdata1, "%d", data1int); 
        sprintf(cdata2, "%d", data2); 
        sprintf(cstate, "%d", state); 
        tick = 0;
        delay(500);
        packetSend(packetDataSaveTrans(NULL, "data1", cdata1)); //将新数据值上传至数据流
        delay(500);
        packetSend(packetDataSaveTrans(NULL, "data2", cdata2)); //将新数据值上传至数据流
        delay(500);
        //后备状态同步方案
        packetSend(packetDataSaveTrans(NULL, "switch", cstate)); //同步开关状态
        delay(500);
    }

    /*接收数据包*/
    while (WIFI_UART.available()){
        readEdpPkt(&rcv_pkt);
        if (isEdpPkt(&rcv_pkt)){
            pkt_type = rcv_pkt.data[0];
            switch (pkt_type) {
            case CMDREQ:
                char edp_command[50];
                char edp_cmd_id[40];
                long id_len, cmd_len, rm_len;
                char datastr[20];
                char val[10];
                memset(edp_command, 0, sizeof(edp_command));
                memset(edp_cmd_id, 0, sizeof(edp_cmd_id));
                edpCommandReqParse(&rcv_pkt, edp_cmd_id, edp_command, &rm_len, &id_len, &cmd_len);

                sscanf(edp_command, "%[^:]:%s", datastr, val);// switch:[1/0] 

                if (val[1] == '1')  Open();
                if (val[1] == '0')  Close();
                
                sprintf(cstate, "%d", state); 
                packetSend(packetDataSaveTrans(NULL, "switch", cstate)); //同步开关状态
                break;
            default:
                ;
                break;
            }
        }
    }
    
    if (rcv_pkt.len > 0)
        packetClear(&rcv_pkt);
    delay(150);
}

/*
* doCmdOk
* 发送命令至模块，从回复中获取期待的关键字
* keyword: 所期待的关键字
* 成功找到关键字返回true，否则返回false
*/
bool doCmdOk(String data, char* keyword){
    bool result = false;
    if (data != "")  { //对于tcp连接命令，直接等待第二次回复
        WIFI_UART.println(data);  //发送AT指令
    }
    if (data == "AT")   //检查模块存在
        delay(2000);
    else
        while (!WIFI_UART.available());  // 等待模块回复
    delay(200);
    if (WIFI_UART.find(keyword))   //返回值判断
        result = true;
    else
        result = false;
    while (WIFI_UART.available()) WIFI_UART.read();   //清空串口接收缓存
    delay(500); //指令时间间隔
    return result;
}

void Open(){ //开门
    digitalWrite(lock_pin, HIGH);
    state = 1;
    digitalWrite(close_pin,LOW);
    digitalWrite(open_pin,HIGH);
    delay(600);
    digitalWrite(close_pin,LOW);
    digitalWrite(open_pin,LOW);
}

void Close(){ //关门
    digitalWrite(lock_pin, LOW);
    state = 0;
    digitalWrite(close_pin,HIGH);
    digitalWrite(open_pin,LOW);
    delay(600);
    digitalWrite(close_pin,LOW);
    digitalWrite(open_pin,LOW);
}

/*
* readEdpPkt
* 从串口缓存中读数据到接收缓存
*/
bool readEdpPkt(edp_pkt* p)
{
    int tmp;
    if ((tmp = WIFI_UART.readBytes(p->data + p->len, sizeof(p->data))) > 0)
    {
        rcvDebug(p->data + p->len, tmp);
        p->len += tmp;
    }
    return true;
}

/*
* packetSend
* 将待发数据发送至串口，并释放到动态分配的内存
*/
void packetSend(edp_pkt* pkt)
{
    if (pkt != NULL)
    {
        WIFI_UART.write(pkt->data, pkt->len);    //串口发送
        WIFI_UART.flush();
        free(pkt);              //回收内存
    }
}

/*用于调试时断点查看rcv，实际工作中无用*/
void rcvDebug(unsigned char* rcv, int len)
{
    int i;
}
