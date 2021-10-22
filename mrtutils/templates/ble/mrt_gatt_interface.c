/**
 * @file mrt_gatt_interface.c
 * @author Jason Berger
 * @brief Abstract/Generic Gatt server.  
 * @version 0.1
 * @date 2020-02-21
 * 
 * 
 */


/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "mrt_gatt_interface.h"

/* Private Variables ---------------------------------------------------------*/
uint8_t default_security = MRT_GATT_SECURITY_NONE;    //Default to no security

/* Private Functions ---------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/

mrt_status_t mrt_gatt_init_pro(mrt_gatt_pro_t* pro, uint16_t serviceCount, uint32_t id, const char* name)
{
    pro->id = id;
    pro->name = name;
    pro->svcCount = 0;
    pro->svcs = (mrt_gatt_svc_t**)malloc(sizeof(mrt_gatt_svc_t*) * serviceCount);
    pro->maxSvcCount = serviceCount;

    return MRT_STATUS_OK;
}


mrt_status_t mrt_gatt_add_svc(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc )
{   

    if(pro->svcCount >= pro->maxSvcCount)
    {
        return MRT_STATUS_ERROR; // Profile is already full
    }

    pro->svcs[pro->svcCount++] = svc;
    svc->pro = pro;

    return MRT_STATUS_OK;
}


mrt_status_t mrt_gatt_init_svc(mrt_gatt_svc_t* svc, uint8_t uuidLen, const uint8_t* arrUuid, uint16_t charCount, mrt_gatt_svc_callback cbEvent,const char* name)
{
     /* Set UUID */
    svc->uuid.len = uuidLen;
    svc->name = name;
    memcpy((uint8_t*) svc->uuid.val, arrUuid, uuidLen);
   

    /* malloc memory for characteristic descriptors */
    svc->charCount = 0;
    svc->maxCharCount = charCount;
    svc->chars = (mrt_gatt_char_t**)malloc(sizeof(mrt_gatt_char_t*) * charCount);
    svc->cbEvent = cbEvent;
    svc->security = MRT_GATT_SECURITY_NONE;    //Default to no security
    svc->attrCount = 1; //One attribute is needed for the service declaration

    return MRT_STATUS_OK;
}

mrt_status_t mrt_gatt_init_char(mrt_gatt_svc_t* svc, mrt_gatt_char_t* chr, uint8_t uuidLen, const uint8_t* arrUuid, uint16_t size, uint8_t props, mrt_gatt_char_callback cbEvent,const char* name  )
{
    if(svc->charCount < svc->maxCharCount)
    {
        /* Set UUID */
        chr->uuid.len = uuidLen;
        chr->handles.char_handle = 0;
        chr->handles.val_handle = 0;
        chr->handles.cccd_handle = 0;
        chr->name = name;
        memcpy((uint8_t*)chr->uuid.val, arrUuid, uuidLen);

        chr->size = size;
        chr->security = default_security;    //use default
        chr->props = props;
        chr->cbEvent = cbEvent;
        chr->notificationsEnable = false;
        chr->svc = svc;
        chr->data.value = (uint8_t*)malloc(size);
        chr->data.len = 0;
        svc->chars[svc->charCount++] = chr;   /*Add ptr to list for looping through*/

        svc->attrCount +=2; //2 attribute for characteristic declaration and value 
        if((chr->props & MRT_GATT_PROP_NOTIFY) > 0)
        {
            svc->attrCount ++; //If Characteristic has notifications, then we add another attribute for the CCCD
        }
    }
    else 
    {
        return MRT_STATUS_ERROR; //tried to add more characteristics than max
    }


    return MRT_STATUS_OK;
}

mrt_status_t mrt_gatt_set_default_security(uint8_t securityFlags)
{
    default_security = securityFlags;

    return MRT_STATUS_OK;
}

bool mrt_gatt_char_has_handle(mrt_gatt_char_t* chr, uint16_t handle)
{
    if((chr->handles.val_handle == handle) || (chr->handles.char_handle == handle) || (chr->handles.cccd_handle == handle))
    {
        return true;
    }

    return false;
}

mrt_gatt_svc_t* mrt_gatt_lookup_svc_uuid(mrt_gatt_pro_t* pro, mrt_gatt_uuid_t* uuid)
{
    for(uint32_t i=0; i < pro->svcCount; i++ )
    {
        if(pro->svcs[i]->uuid.len == uuid->len)
        {
            
            if(memcmp((void*)pro->svcs[i]->uuid.val, (void*) &uuid->val, uuid->len  ) ==0)
            {
                return pro->svcs[i];
            }
        }
    }

    return NULL;
}

mrt_gatt_char_t* mrt_gatt_lookup_char_uuid(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc, mrt_gatt_uuid_t* uuid)
{
    if(svc == NULL)
    {
        for(uint32_t i=0; i < pro->svcCount; i++ )
        {
            for(uint32_t a=0; a < pro->svcs[i]->charCount; a++ )
            {
                if(pro->svcs[i]->chars[a]->uuid.len == uuid->len)
                {
                    
                    if(memcmp((void*)pro->svcs[i]->chars[a]->uuid.val, (void*) &uuid->val, uuid->len  ) ==0)
                    {
                        return pro->svcs[i]->chars[a];
                    }
                }
            }
        }
    }
    else 
    {
        for(uint32_t i=0; i < svc->charCount; i++ )
        {
            if(svc->chars[i]->uuid.len == uuid->len)
            {
                
                if(memcmp((void*)svc->chars[i]->uuid.val, (void*) &uuid->val, uuid->len  ) ==0)
                {
                    return svc->chars[i];
                }
            }
        }
    }

    return NULL;
}

mrt_gatt_svc_t* mrt_gatt_lookup_svc_handle(mrt_gatt_pro_t* pro, uint16_t handle)
{
    for(uint32_t i=0; i < pro->svcCount; i++ )
    {
        if(pro->svcs[i]->handle == handle)
        {
            return pro->svcs[i];
        }
    }

    return NULL;
}

mrt_gatt_char_t* mrt_gatt_lookup_char_handle(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc, uint16_t handle)
{
    if(svc == NULL)
    {
        for(uint32_t i=0; i < pro->svcCount; i++ )
        {
            for(uint32_t a=0; a < pro->svcs[i]->charCount; a++ )
            {
                if(mrt_gatt_char_has_handle(pro->svcs[i]->chars[a],handle))
                {   
                    return pro->svcs[i]->chars[a];
                }
            }
        }
    }
    else 
    {
        for(uint32_t i=0; i < svc->charCount; i++ )
        {
            if(mrt_gatt_char_has_handle(svc->chars[i] ,handle))
            {
                return svc->chars[i];
            }
        }
    }

    return NULL;
}

/* Deinit Functions ------------------------------------------------------- */

/**
 * These functions free up resources allocated during initialization
 * Because the Gatt Server will be up for the full lifecycle of most applications, they are not needed
 * They are mainly used for detecting memory leaks in unit testing
 */
void mrt_gatt_deinit_pro(mrt_gatt_pro_t* pro)
{
    //TODO
}

void mrt_gatt_deinit_svc(mrt_gatt_svc_t* svc)
{
    //TODO
}

void mrt_gatt_deinit_chr(mrt_gatt_char_t* chr)
{
    //TODO
}   

/* Weak Functions -------------------------------------------------------- */

#ifndef MRT_NO_WEAK //Some platforms do not support weakly typed functions

/**
 * @brief This weakly defined function just updates the cache.
 *        Each platform will override this with its own function
 */
__attribute__((weak)) mrt_status_t mrt_gatt_set_char_val(mrt_gatt_char_t* chr, uint8_t* val, uint16_t len)
{
    if(len > chr->size)
    {
        return MRT_STATUS_ERROR;
    }

    memcpy(chr->data.value, val, len);
    chr->data.len = len;

    return MRT_STATUS_OK;
}

/**
 * @brief This weakly defined function just reads the cache.
 *        Each platform will override this with its own function
 */
__attribute__((weak)) mrt_status_t mrt_gatt_get_char_val(mrt_gatt_char_t* chr)
{


    return MRT_STATUS_OK;
}



#endif //MRT_NO_WEAK