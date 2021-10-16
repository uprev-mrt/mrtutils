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

mrt_status_t mrt_gatt_init_profile(mrt_gatt_pro_t* pro, uint16_t serviceCount, uint32_t id)
{
    pro->mId = id;
    pro->mSvcCount = 0;
    pro->mSvcs = (mrt_gatt_svc_t**)malloc(sizeof(mrt_gatt_svc_t*) * serviceCount);
    pro->mMaxSvcCount = serviceCount;

    return MRT_STATUS_OK;
}


mrt_status_t mrt_gatt_add_service(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc )
{   

    if(pro->mSvcCount >= pro->mMaxSvcCount)
    {
        return MRT_STATUS_ERROR; // Profile is already full
    }

    pro->mSvcs[profile->mSvcCount++] = svc;
    svc->mPro = pro;

    return MRT_STATUS_OK;
}


mrt_status_t mrt_gatt_init_svc(mrt_gatt_svc_t* svc, uint8_t uuidType, const uint8_t* arrUuid, uint16_t charCount, mrt_gatt_svc_callback cbEvent)
{
     /* Set UUID */
    svc->mUuid.mLen = uuidType;
    if(uuidType == MRT_UUID_LEN_16)
    {   
        memcpy((uint8_t*)&svc->mUuid.m16Bit, arrUuid, 2);
    }
    else
    {
        memcpy((uint8_t*)&svc->mUuid.m128Bit, arrUuid, 16);
    }

    /* malloc memory for characteristic descriptors */
    svc->mCharCount = 0;
    svc->mMaxCharCount = charCount;
    svc->mChars = (mrt_gatt_char_t**)malloc(sizeof(mrt_gatt_char_t*) * charCount);
    svc->cbEvent = cbEvent;
    svc->mSecurity = MRT_GATT_SECURITY_NONE;    //Default to no security
    svc->mAttrCount = 1; //One attribute is needed for the service declaration

    return MRT_STATUS_OK;
}

mrt_status_t mrt_gatt_init_char(mrt_gatt_svc_t* svc, mrt_gatt_char_t* chr, uint8_t uuidType, const uint8_t* arrUuid, uint16_t size, uint8_t props, mrt_gatt_char_callback cbEvent  )
{
    if(svc->mCharCount < svc->mMaxCharCount)
    {
        /* Set UUID */
        chr->mUuid.mLen = uuidType;
        chr->mHandles.mChar = 0;
        chr->mHandles.mValue = 0;
        chr->mHandles.mCCCD = 0;
        if(uuidType == MRT_UUID_LEN_16)
        {   
            memcpy((uint8_t*)&chr->mUuid.m16Bit, arrUuid, 2);
        }
        else
        {
            memcpy((uint8_t*)&chr->mUuid.m128Bit, arrUuid, 16);
        }

        chr->mSize = size;
        chr->mSecurity = default_security;    //use default
        chr->mProps = props;
        chr->cbEvent = cbEvent;
        chr->mNotificationsEnable = false;
        chr->mSvc = svc;
        chr->mCache.mData = (uint8_t*)malloc(size);
        chr->mCache.mLen = 0;
        svc->mChars[svc->mCharCount++] = chr;   /*Add ptr to list for looping through*/

        svc->mAttrCount +=2; //2 attribute for characteristic declaration and value 
        if(chr->mProps | MRT_GATT_PROP_NOTIFY)
        {
            svc->mAttrCount ++; //If Characteristic has notifications, then we add another attribute for the CCCD
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

mrt_gatt_svc_t* mrt_gatt_lookup_svc(mrt_gatt_pro_t* pro, mrt_gatt_uuid_t* uuid)
{
    for(uint32_t i=0; i < pro->mSvcCount; i++ )
    {
        if(pro->mSvcs[i]->mUuid.mLen == uuid->mLen)
        {
            
            if(memcmp((void*)pro->mSvcs[i]->mUuid.mUuid, (void*) &uuid->mUuid, uuid->mLen  ) ==0)
            {
                return pro->mSvcs[i];
            }
        }
    }

    return NULL;
}

mrt_gatt_char_t* mrt_gatt_lookup_char(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc, mrt_gatt_uuid_t* uuid)
{
    if(svc == NULL)
    {
        svc = mrt_gatt_lookup_svc(pro,uuid)
    }

    for(uint32_t i=0; i < svc->mCharCount; i++ )
    {
        if(svc->mChars[i]->mUuid.mLen == uuid->mLen)
        {
            
            if(memcmp((void*)svc->mChars[i]->mUuid.mUuid, (void*) &uuid->mUuid, uuid->mLen  ) ==0)
            {
                return svc->mChars[i];
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
void mrt_gatt_deinit_pro(mrt_gatt_prof_t* pro)
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
    if(len > chr->mSize)
    {
        return MRT_STATUS_ERROR;
    }

    memcpy(chr->mCache.mData, val, len);
    chr->mCache.mLen = len;

    return MRT_STATUS_OK;
}

/**
 * @brief This weakly defined function just reads the cache.
 *        Each platform will override this with its own function
 */
__attribute__((weak)) mrt_status_t mrt_gatt_get_char_val(mrt_gatt_char_t* chr);
{


    return MRT_STATUS_OK;
}


__attribute__((weak)) mrt_status_t mrt_gatt_register_svc(mrt_gatt_svc_t* svc)
{
    return MRT_STATUS_NOT_IMPLEMENTED; 
}

__attribute__((weak)) mrt_status_t mrt_gatt_register_char(mrt_gatt_char_t* chr)
{
    return MRT_STATUS_NOT_IMPLEMENTED; 
}

#endif //MRT_NO_WEAK




e