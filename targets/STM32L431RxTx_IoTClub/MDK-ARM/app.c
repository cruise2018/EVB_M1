/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/

#include <osport.h>
#include <DHT11_BUS.h>

//show the led welcome information
extern void led_display(void); 

//bc95 interface
extern bool_t bc95_init(const char *server);
extern bool_t bc95_send(u8_t *buf,s32_t len, u32_t timeout);
extern void bc95_regester_receivehandle(void *handle);

//led control
extern void led_on(void);
extern void led_off(void);

//enum all the message type
typedef enum
{
    en_msgid_net = 0,
    en_msgid_dht11 ,
    en_msgid_cmd_led,
}en_app_msgid;

//this function used to deal all the received message here
//message format:msgid+content
static void  deal_rcvmsg(u8_t *data, s32_t len)
{
    u8_t msgid;
    char *str;
    printf("msg:len:%d msgid:%d data:%s \r\n",len,*data,(char *)(data+1));
    msgid = *data;
    switch (msgid)
    {
        case en_msgid_cmd_led:
            str = (char *)data+1;
            if(0 == strcmp(str,"ON"))
            {
                led_on();
            }
            else if(0 == strcmp(str,"OFF"))
            {
                led_off();
            }
            else
            {
                printf("LED CMD ERROR\r\n");
            }
            break;
        default:
            printf("UNKOWN MSGID\r\n");
            break;   
    }
    return;
}

//this is app main function:loop
u32_t app_main(void *args)
{
    u8_t sendbuf[32];
    u8_t msgid;
    DHT11_Data_TypeDef  DHT11_Data;

    led_display();
    bc95_init("139.159.140.34");
    bc95_regester_receivehandle(deal_rcvmsg);
    DHT11_Init();   //do reinilialize    
    
    while(1)
    {
        //sample the data and report
        if(DHT11_Read_TempAndHumidity(&DHT11_Data)==SUCCESS)
        {
            printf("READ DHT11 OK:Humidity:%.1f Temprature:%.1f \r\n",DHT11_Data.humidity,DHT11_Data.temperature);
            memset(sendbuf,0,32);
            msgid = en_msgid_dht11;
            sendbuf[0] = msgid;
            snprintf((char *)&sendbuf[1],32,"%.1f%.1f",DHT11_Data.temperature,DHT11_Data.humidity);
            if(bc95_send(sendbuf,1+strlen((char *)&sendbuf[1]),2*1000))
            {
                printf("sendmsg OK:%s\n\r",(char *)sendbuf);
            }
            else
            {
                printf("sendmsg ERR:%s\n\r",(char *)sendbuf);
            }
        }
        else
        {
            printf("∂¡»°DHT11–≈œ¢ ß∞‹\r\n");
            DHT11_Init();   //do reinilialize
            continue;
        }   
        task_sleepms(20*1000);   
    }
}

//export a shell command to send data
#include <shell.h>
static s32_t shell_appsend(s32_t argc, const char *argv[])
{
    u8_t sendbuf[64];
    u8_t msgid;
    
    if(argc != 3)
    {
        printf("paras error\n\r");
        return -1;
    }
    if((argv[1][0] >'9')||(argv[1][0] <'0'))
    {
        printf("msgid error\n\r");
        return -1;
    }
    msgid = argv[1][0]-'0';
    memset(sendbuf,0,64);
    sendbuf[0] = msgid;
    strncpy((char *)&(sendbuf[1]),argv[2],62);
    
    if(bc95_send(sendbuf,1+strlen(argv[2]),2*1000))
    {
        printf("%s: send success\n\r",__FUNCTION__);
    }
    else
    {
        printf("%s: send failed\n\r",__FUNCTION__);
    }
    
    return 0;

}
OSSHELL_EXPORT_CMD(shell_appsend,"appsend","appsend msgid data");
