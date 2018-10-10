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


//the bc95 module;
#include <at.h>  


typedef void (*fnhandle)(u8_t *data, s32_t len);
#define cn_bc95_cachelen 128
struct bc95_cb
{
    bool_t      sndenable;
    u8_t        sndbuf[cn_bc95_cachelen];  //used for the receive
    u8_t        rcvbuf[cn_bc95_cachelen];
    fnhandle    rcvhandle;
    //for the debug
    u32_t   rcvlen;
    u32_t   sndlen;
};
static struct bc95_cb   s_bc95_cb;

//use this function to register a handle to deal with the received data
void bc95_regester_receivehandle(void *handle)
{
    s_bc95_cb.rcvhandle = handle;
    return;
}

//do the module reboot here:used by the module init;true success while false failed
static bool_t bc95_nrb(void)
{
    const char *cmd = "AT+NRB\r";
    const char *index = "REBOOTING";
    s32_t ret = 0;

    ret = at_command((u8_t *)cmd,strlen(cmd),index,NULL,0,1000*5);
    if(ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    } 
} 

//connect to the network
static bool_t bc95_cgatt(void)
{
    const char *cmd = "AT+CGATT?\r";
    const char *index = "CGATT:1";
    s32_t ret = 0;

    ret = at_command((u8_t *)cmd,strlen(cmd),index,NULL,0,1000*5);
    if(ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    } 
} 
//set the receive mode
static bool_t bc95_nnmi(void)
{
    const char *cmd = "AT+NNMI=1\r";
    const char *index = "OK";
    s32_t ret = 0;

    ret = at_command((u8_t *)cmd,strlen(cmd),index,NULL,0,1000*5);
    if(ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    } 
} 
//set the ncdp
static bool_t bc95_ncdp(const char *server)
{
    char cmd[64];
    const char *index = "OK";
    s32_t ret = 0;

    memset(cmd,0,64);
    snprintf(cmd,64,"AT+NCDP=%s\r",server);
    ret = at_command((u8_t *)cmd,strlen(cmd),index,NULL,0,1000*5);
    if(ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    } 
} 


static int bc95_str_to_hex(const unsigned char *bufin, int len, char *bufout)
{
    int i = 0;
    if (NULL == bufin || len <= 0 || NULL == bufout)
    {
        return -1;
    }
    for(i = 0; i < len; i++)
    {
        sprintf(bufout+i*2, "%02X", bufin[i]);
    }
    return 0; 
}


//send data
bool_t bc95_send(u8_t *buf,s32_t len, u32_t timeout)
{
    s32_t ret = 0;
    const char *cmd = "AT+NMGS=";
    const char *index = "OK";
    if (NULL == buf || len >= 128 ||(false == s_bc95_cb.sndenable))
    {
        return false;
    }
    memset(s_bc95_cb.sndbuf, 0, cn_bc95_cachelen);
    snprintf((char *)s_bc95_cb.sndbuf,cn_bc95_cachelen,"%s%d,",cmd,len);    
    ret = bc95_str_to_hex((unsigned char *)buf, len, (char *)&s_bc95_cb.sndbuf[strlen((char *)s_bc95_cb.sndbuf)]);
    s_bc95_cb.sndbuf[strlen((char *)s_bc95_cb.sndbuf)]='\r';
    ret = at_command((u8_t *)s_bc95_cb.sndbuf,strlen((char *)s_bc95_cb.sndbuf),index,NULL,0,timeout);
    if(ret > 0)
    {
        return true;
    }
    else
    {
        return false;
    } 
}

/* ============================================================
func name   :  neul_bc95_hex_to_str
discription :  convert char to hex string 
               
param       :  bufin @ input param , the data that need to convert
               len @ input param, data length
               bufout @ output param, hex string data 
return      :  0 convert ok , !0 not ok
Revision    : 
=============================================================== */
static int bc95_hex_to_str(const char *bufin, int len, char *bufout)
{
    int i = 0;
    unsigned char tmp2 = 0x0;
    unsigned int tmp = 0;
    if (NULL == bufin || len <= 0 || NULL == bufout)
    {
        return -1;
    }
    for(i = 0; i < len; i = i+2)
    {
        tmp2 =  bufin[i];
        tmp2 =  tmp2 <= '9'?tmp2-0x30:tmp2-0x37;
        tmp =  bufin[i+1];
        tmp =  tmp <= '9'?tmp-0x30:tmp-0x37;
        bufout[i/2] =(tmp2<<4)|(tmp&0x0F);

    }
    return 0; 
}

#define cn_bc95_rcvindex  "\r\n+NNMI:"
static s32_t bc95_rcvdeal(u8_t *data,s32_t len)
{
    s32_t ret = 0;
    s32_t datalen;
    
    char  *str;
    if(len <strlen(cn_bc95_rcvindex))
    {
        printf("%s:invalid frame:%d byte:%s\n\r",__FUNCTION__,len,(char *)data);
        return ret;
    }
    //now deal the data
    str = strstr((char *)data,":");
    if(NULL == str)
    {
        printf("%s:format error\n\r",__FUNCTION__);
        return ret; //format error
    }
    str++;
    datalen = 0;
    for (; *str <= '9' && *str >= '0' ;str++)
    {
        datalen = (datalen * 10 + (*str - '0'));
    }
    str++;
    //now this is data of hex string
    if((u32_t)(str + datalen*2 +2) > (u32_t)(data + len))
    {
        printf("%s:implement error\n\r",__FUNCTION__);
        return ret; //
    }
    if(datalen > cn_bc95_cachelen)
    {
        printf("%s:frame over: frame:%d  cachelen:%d \n\r",__FUNCTION__,datalen,cn_bc95_cachelen);
        return ret; //
    }
    memset(s_bc95_cb.rcvbuf,0,cn_bc95_cachelen);
    bc95_hex_to_str(str,datalen*2,(char *)s_bc95_cb.rcvbuf);
    printf("%s:appdata:%d bytes\n\r",__FUNCTION__,datalen);
    if(NULL != s_bc95_cb.rcvhandle)
    {
        s_bc95_cb.rcvhandle(s_bc95_cb.rcvbuf,datalen);
    }
    return len;    
}

#define cn_try_times  10
bool_t bc95_init(const char *server)
{
    bool_t ret = false;
    s32_t times = 0;
    while(1)
    {
        //do the module reset
        bc95_nrb();
        task_sleepms(5000); //wait for some time
        
        //wait for the cgatt
        printf("wait for the network. ");
        times = cn_try_times;
        for(times =0;times <cn_try_times;times++ )
        {
            ret = bc95_cgatt();
            if(ret)
            {
                break;
            }
            printf("\b%d",times);
            task_sleepms(2000);
        }
        if(false == ret)
        {
            printf("failed \n\r");
            continue;
        }
        printf("OK \n\r");

        //set the ncdp
        times = cn_try_times;
        printf("set ncdp and wait  ");
        for(times =0;times <cn_try_times;times++ )
        {
            ret = bc95_ncdp(server);
            if(ret)
            {
                break;
            }
            printf("\b%d",times);
            task_sleepms(2000);
        }
        if(false == ret)
        {
            printf("failed \n\r");
            continue;
        }
        printf("OK \n\r");
        
        //make the report auto
        printf("set nnmi and wait  ");
        times = cn_try_times;
        for(times =0;times <cn_try_times;times++ )
        {
            ret = bc95_nnmi();
            if(ret)
            {
                break;
            }
            printf("\b%d",times);
            task_sleepms(2000);
        }
        if(false == ret)
        {
            printf("  failed \n\r");
            continue;
        }
        printf("  OK \n\r");    
        break;
    }
   //reach here means everything is ok, we can go now
    memset(&s_bc95_cb,0,sizeof(s_bc95_cb));
    at_oobregister(bc95_rcvdeal,cn_bc95_rcvindex);
    s_bc95_cb.sndenable = true;
    
    printf("BC95 READY NOW\n\r");
    return true;
}


#include <shell.h>

static s32_t shell_bc95send(s32_t argc, const char *argv[])
{
    if(argc != 2)
    {
        printf("paras error\n\r");
        return -1;
    }
    if(bc95_send((u8_t *)argv[1],strlen(argv[1]),5*1000))
    {
        printf("%s: send success\n\r",__FUNCTION__);
    }
    else
    {
        printf("%s: send failed\n\r",__FUNCTION__);
    }
    
    return 0;

}
OSSHELL_EXPORT_CMD(shell_bc95send,"bc95send","bc95send data");





