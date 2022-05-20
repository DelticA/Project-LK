#include "edp.c"

#define KEY  "KjUxkA8bV1JUnlVctGaVCsITH5Y="    //APIkey 
#define ID   "910385808"                          //设备ID
#define lock_pin 8
#define wifi_pin 13
#define _baudrate   115200
#define WIFI_UART   Serial

int tick = 200;
edp_pkt* pkt;

/*
* doCmdOk
* 发送命令至模块，从回复中获取期待的关键字
* keyword: 所期待的关键字
* 成功找到关键字返回true，否则返回false
*/
bool doCmdOk(String data, char* keyword)
{
    bool result = false;
    if (data != "")   //对于tcp连接命令，直接等待第二次回复
    {
        WIFI_UART.println(data);  //发送AT指令

    }
    if (data == "AT")   //检查模块存在
        delay(2000);
    else
        while (!WIFI_UART.available());  // 等待模块回复

    delay(200);
    if (WIFI_UART.find(keyword))   //返回值判断
    {
        result = true;
    }
    else
    {
        result = false;
    }
    while (WIFI_UART.available()) WIFI_UART.read();   //清空串口接收缓存
    delay(500); //指令时间间隔
    return result;
}

void setup()
{
    char buf[100] = { 0 };
    int tmp;

    pinMode(wifi_pin, OUTPUT); //wifi指示灯
    pinMode(lock_pin, OUTPUT);//物联网控制引脚

    WIFI_UART.begin(_baudrate);
    WIFI_UART.setTimeout(3000);    //设置find超时时间
    delay(3000);
    Serial.setTimeout(100);

    delay(2000);
    while (!doCmdOk("AT", "OK"));
    digitalWrite(wifi_pin, LOW);  
    digitalWrite(lock_pin, LOW);
    while (!doCmdOk("AT+CWMODE=3", "OK"));            //工作模式


    while (!doCmdOk("AT+CWJAP=\"123\",\"lfc12345\"", "OK"));
    while (!doCmdOk("AT+CIPSTART=\"TCP\",\"jjfaedp.hedevice.com\",876", "OK"));
    while (!doCmdOk("AT+CIPMODE=1", "OK"));           //透传模式
    while (!doCmdOk("AT+CIPSEND", ">"));              //开始发送
}

void loop()
{
    static int edp_connect = 0;
    bool trigger = false;
    edp_pkt rcv_pkt;
    unsigned char pkt_type;
    int i = 0, tmp;
    char num[10];

    int data1, data2;
    char cdata1[20], cdata2[20];

    /* EDP 连接 */
    if (!edp_connect)
    {
        while (WIFI_UART.available()) WIFI_UART.read(); //清空串口接收缓存
        packetSend(packetConnect(ID, KEY));             //发送EPD连接包
        while (!WIFI_UART.available());                 //等待EDP连接应答
        if ((tmp = WIFI_UART.readBytes(rcv_pkt.data, sizeof(rcv_pkt.data))) > 0)
        {
            rcvDebug(rcv_pkt.data, tmp);

            if (rcv_pkt.data[0] == 0x20 && rcv_pkt.data[2] == 0x00 && rcv_pkt.data[3] == 0x00)
            {
                edp_connect = 1;

                digitalWrite(13, HIGH);   // 使Led灭
            }
            else
                ;
        }
        packetClear(&rcv_pkt);
    }


    tick++;
    /*心跳包、上传数据包
    *data1、2即发送的数据
    */
    if (tick > 200 && edp_connect) //每50对应约8秒
    {
        data1 = 233;
        data2 = 666;
        sprintf(cdata1, "%d", data1); //int型转换char型
        sprintf(cdata2, "%d", data2); 
        tick = 0;
        delay(500);
        packetSend(packetDataSaveTrans(NULL, "data1", cdata1)); //将新数据值上传至数据流
        delay(500);
        packetSend(packetDataSaveTrans(NULL, "data2", cdata1)); //将新数据值上传至数据流
        delay(500);
    }


    /*接收数据包*/
    while (WIFI_UART.available())
    {
        readEdpPkt(&rcv_pkt);
        if (isEdpPkt(&rcv_pkt))
        {
            pkt_type = rcv_pkt.data[0];
            switch (pkt_type) 
            {
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
                /*
                Serial.println("edp_command:");
                Serial.println(edp_command);
                Serial.println("datastr:");
                Serial.println(datastr);
                Serial.println("val:");
                Serial.println(val);
                Serial.println("val[1]:");
                Serial.println(val[1]);*/

                if (val[0] == '1')   digitalWrite(lock_pin, HIGH);
                if (val[0] == '0')  digitalWrite(lock_pin, LOW);
                //if (atoi(val[1]) == 1)     digitalWrite(lock_pin, HIGH);
                //else                        digitalWrite(lock_pin, LOW);   // 使Led灭

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
