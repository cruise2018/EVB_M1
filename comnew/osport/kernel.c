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

#if 0
//include the file which implement the function
#include "los_task.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_memory.h"

#include "osport.h"

void task_sleepms(s32_t ms)
{
    LOS_TaskDelay(ms);//which tick is ms
    return;
}
s32_t task_create(const char *name,fnTaskEntry fnTask,s32_t stackisize,void *stack,void *args,s32_t prior)
{
    int ret =-1;
    UINT32 uwRet = LOS_OK;
    UINT32  handle;
    TSK_INIT_PARAM_S task_init_param;
	
    task_init_param.uwArg = (unsigned int)args;
    task_init_param.usTaskPrio = prior;
    task_init_param.pcName =(char *) name;
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)fnTask;
    task_init_param.uwStackSize = stackisize;
    uwRet = LOS_TaskCreate(&handle, &task_init_param);
    if(LOS_OK != uwRet){
        return ret;
    }
    ret = handle;
    return ret;
}


//creat a mutex for the os
bool_t  mutex_create(mutex_t *mutex)
{
    if(LOS_OK == LOS_MuxCreate((UINT32 *)mutex))
    {
        return true;
    }
    else
    {
        return false;
    }
}
//lock the mutex
bool_t  mutex_lock(mutex_t mutex)
{
    if(LOS_OK == LOS_MuxPend(mutex,LOS_WAIT_FOREVER))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//unlock the mutex
bool_t  mutex_unlock(mutex_t mutex)
{
    if(LOS_OK == LOS_MuxPost(mutex))
    {
        return true;
    }
    else
    {
        return false;
    }
}
//delete the mutex
bool_t  mutex_del(mutex_t *mutex)
{
    if(LOS_OK == LOS_MuxDelete((UINT32)*mutex))
    {
        return true;
    }
    else
    {
        return false;
    }
}
//semp of the os
bool_t  semp_create(semp_t *semp,s32_t limit,s32_t initvalue)
{
    extern UINT32 osSemCreate (UINT16 usCount, UINT16 usMaxCount, UINT32 *puwSemHandle);
    if(LOS_OK == osSemCreate(initvalue,limit,(UINT32 *)semp))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool_t  semp_pend(semp_t semp,u32_t timeout)
{
    if(LOS_OK == LOS_SemPend(semp,timeout))
    {
        return true;
    }
    else
    {
        return false;
    }
}
bool_t  semp_post(semp_t semp)
{
    if(LOS_OK == LOS_SemPost(semp))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool_t  semp_del(semp_t *semp)
{   
    if(LOS_OK == LOS_SemDelete((UINT32)*semp))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void *malloc(int size)
{
    void *ret = NULL;
    
    if(size > 0)
    {
         ret = LOS_MemAlloc(m_aucSysMem0,size);
    }

    return ret;
}

bool_t free(void *addr)
{
    if(LOS_OK == LOS_MemFree(m_aucSysMem0,addr))
    {
        return true;
    }
    else
    {
        return false; 
    }
}

#endif