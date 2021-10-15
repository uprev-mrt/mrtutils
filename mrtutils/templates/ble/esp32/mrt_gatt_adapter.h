/**
 * @file mrt_gatt_adapter.h
 * @author Jason Berger
 * @brief Abstract/Generic Gatt server.  
 * @version 0.1
 * @date 2020-02-21
 * 
 * 
 */

#ifndef MRT_GATT_ADAPTER_H_
#define MRT_GATT_ADAPTER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#define MRT_GATT_PROP_NONE                           0x00
#define MRT_GATT_PROP_BROADCAST                      0x01
#define MRT_GATT_PROP_READ                           0x02
#define MRT_GATT_PROP_WRITE_WITHOUT_RESP             0x04
#define MRT_GATT_PROP_WRITE                          0x08
#define MRT_GATT_PROP_NOTIFY                         0x10
#define MRT_GATT_PROP_INDICATE                       0x20
#define MRT_GATT_PROP_SIGNED_WRITE                   0x40
#define MRT_GATT_PROP_EXT                            0x80

/* Security permissions for a characteristic.
 */
#define MRT_GATT_SECURITY_NONE          0x00 /* No security. */
#define MRT_GATT_SECURITY_AUTHEN_READ   0x01 /* Need authentication to read */
#define MRT_GATT_SECURITY_AUTHOR_READ   0x02 /* Need authorization to read */
#define MRT_GATT_SECURITY_ENCRY_READ    0x04 /* Need encryption to read */
#define MRT_GATT_SECURITY_AUTHEN_WRITE  0x08 /* Need authentication to write */
#define MRT_GATT_SECURITY_AUTHOR_WRITE  0x10 /* Need authorization to write */
#define MRT_GATT_SECURITY_ENCRY_WRITE   0x20 /* Need encryption to write */


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

#include "esp_gatts_api.h"


/* MrT Defines ---------------------------------------------------------------*/
typedef uint32_t mrt_status_t;
#define MRT_STATUS_OK 0
#define MRT_STATUS_ERROR 1
#define MRT_UPDATE_CHAR(chr,val,len) esp_ble_gatts_set_attr_value(chr->mHandle, len, val)




/* Exported types ------------------------------------------------------------*/

typedef enum{
    e16Bit,
    e128Bit
}uuid_type_e;

typedef enum{
    GATT_EVT_NONE           = 0x00, /* None/unknown */     
    GATT_EVT_VALUE_WRITE    = 0x01, /* Value is being written */
    GATT_EVT_VALUE_READ     = 0x02, /* Value is being read */
    GATT_EVT_DESCR_WRITE    = 0x03, /* Descriptor is being written (ususally to enable notifications)*/
    GATT_EVT_DESCR_READ    = 0x04  /* Descriptor is  being read*/
}mrt_gatt_evt_type_e;

/* UUID struct */
typedef struct{
    union {
        uint16_t m16Bit;            //16bit uuid
        uint8_t m128Bit[16];        //128bit uuid
    };
    uuid_type_e mType;              //type 16bit or 128bit
} mrt_gatt_uuid_t;


/* Forward Declare for self referencing function pointers */
typedef struct mrt_gatt_svc_t mrt_gatt_svc_t;                             //forward declare for self referencing callback
typedef struct mrt_gatt_char_t mrt_gatt_char_t;                             //forward declare for self referencing callback


/* gatt Event struct*/
typedef struct{
    mrt_gatt_evt_type_e mType;   /*Type of event*/
    mrt_gatt_char_t* mChar;      /*Characteristic for event*/
    struct{
        uint8_t* data;          /*ptr to data for writes (or notify on client)*/
        int len;                /* len of data in bytes*/
    }mData;
} mrt_gatt_evt_t;

typedef struct{
    uint16_t mChar;             //Character declaration handle
    uint16_t mValue;            //Value handle 
    uint16_t mCCCD;             //Client Config handle
}mrt_gatt_handles_t;

typedef mrt_status_t (*mrt_gatt_svc_callback)(mrt_gatt_svc_t* svc, mrt_gatt_evt_t* event); 
typedef mrt_status_t (*mrt_gatt_char_callback)(mrt_gatt_evt_t* event); 

/* gatt characteristic descriptor */
struct mrt_gatt_char_t{
    mrt_gatt_uuid_t mUuid;          //UUID of Characteristic
    uint16_t mSize;                 //size of Characteristic
    mrt_gatt_handles_t mHandle;               //Handles of Characteristic
    uint16_t mValHandle;               //Handle of Characteristic
    uint16_t mCCCDHandle;               //Handle of Characteristic
    uint8_t mProps;                 //Permissions of characteristic
    uint8_t mSecurity;              //Security flags for characteristic
    bool mNotificationsEnable;      //Indicates if Notifications are enabled in CCCD
    struct{
        uint8_t* mData;              // Cached data
        uint8_t mLen;                    // len of last cached data (data invalid if 0)
    }mCache;
    mrt_gatt_svc_t* mSvc;
    mrt_gatt_char_callback cbEvent; //characteristic event callback
};

/* gatt Service descriptor */
struct mrt_gatt_svc_t{
    mrt_gatt_uuid_t mUuid;          //UUID of Service
    uint16_t  mHandle;              //Handle
    mrt_gatt_char_t** mChars;       //Array of characteristics   
    uint16_t mCharCount;            //Number of characterestics
    uint16_t mMaxCharCount;         //max number of characteristics
    uint8_t mSecurity;              //Security flags for service, if set this will override characteristic level security flags
    mrt_gatt_svc_callback cbEvent;  //Service event callback
};

/* Exported constants --------------------------------------------------------*/
#ifndef MRT_GATT_DATA_ATTR   /*Some platforms use pre-proccessor directives to place service data in memory sections*/
#define MRT_GATT_DATA_ATTR 
#endif
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Initialize service struct
 * @param svc - ptr to struct
 * @param uuidType - uuid type (e16Bit or e128Bit)
 * @param arrUuid - array of bytes representing UUID
 * @param charCount - number of characteristics in service 
 * @param cbEvent - callback event (unused for now)
 */
mrt_status_t gatt_init_svc(mrt_gatt_svc_t* svc, uuid_type_e uuidType, const uint8_t* arrUuid, uint16_t charCount, mrt_gatt_svc_callback cbEvent);

/**
 * @brief Initializes a characteristic and adds it to a service
 * @param svc - ptr to service
 * @param chr - ptr to characteristic
 * @param uuidType - uuid type (e16Bit or e128Bit)
 * @param arrUuid - array of bytes representing UUID
 * @param size - size of data in bytes
 * @param props - properties (READ,WRITE,NOTIFY etc)
 * @param cbEvent - callback handler for gatt event 
 */
mrt_status_t gatt_init_char(mrt_gatt_svc_t* svc, mrt_gatt_char_t* chr, uuid_type_e uuidType, const uint8_t* arrUuid, uint16_t size, uint8_t props, mrt_gatt_char_callback cbEvent );

/**
 * @brief Sets the default security flags for intializing characteristics  
 * @param securityFlags new default flags
 * @return mrt_status_t 
 * @note must be called before initializing characteristics
 */
mrt_status_t gatt_set_default_security(uint8_t securityFlags);

/**
 * @brief Sets the default security flags for intializing characteristics  
 * @param securityFlags new default flags
 * @return mrt_status_t 
 * @note must be called before initializing characteristics
 */
mrt_status_t gatt_set_char_val(mrt_gatt_char_t* chr, uint8_t* val, int len);





#ifdef __cplusplus
}
#endif
#endif /*MRT_GATT_ADAPTER_H_*/
