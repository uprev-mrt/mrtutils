/**
 * @file mrt_gatt_interface.h
 * @author Jason Berger
 * @brief Abstract/Generic Gatt server.  
 * @version 0.1
 * @date 2020-02-21
 * 
 * 
 */

#ifndef MRT_GATT_INTERFACE_H_
#define MRT_GATT_INTERFACE_H_
#ifdef __cplusplus
extern "C"
{
#endif



/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

// This allows the interface to be used outside of the MRT framework
#ifdef MRT_PLATFORM
    #define MRT_GATT_MODULE_ENABLED
    #include "Platforms/Common/mrt_platform.h"
#endif //MRT_PLATFORM

/* Macros   ------------------------------------------------------------------*/


// This allows the interface to be used outside of the MRT framework
#ifndef MRT_PLATFORM
    typedef uint32_t mrt_status_t;
    #define MRT_STATUS_OK 0
    #define MRT_STATUS_ERROR 1
    #define MRT_STATUS_NOT_IMPLEMENTED 2
#endif //MRT_PLATFORM

#ifndef MRT_GATT_DATA_ATTR   /*Some platforms use pre-proccessor directives to place service data in memory sections*/
#define MRT_GATT_DATA_ATTR 
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


#define MRT_UUID_LEN_16 2
#define MRT_UUID_LEN_32 4
#define MRT_UUID_LEN_128 16

#define MRT_GATT_EVT_NONE          0x00, /* None/unknown */     
#define MRT_GATT_EVT_VALUE_WRITE   0x01, /* Value is being written */
#define MRT_GATT_EVT_VALUE_READ    0x02, /* Value is being read */
#define MRT_GATT_EVT_CCCD_WRITE    0x03, /* Descriptor is being written (ususally to enable notifications)*/
#define MRT_GATT_EVT_CCCD_READ     0x04  /* Descriptor is  being read*/

/* Exported types ------------------------------------------------------------*/


/* UUID struct */
typedef struct{
    union {
        uint16_t m16Bit;            //16bit uuid
        uint8_t m128Bit[16];        //128bit uuid
    } mUuid;
    uint8_t mLen;                   //UUID len in bytes
} mrt_gatt_uuid_t;


/* Forward Declare for self referencing function pointers */
typedef struct mrt_gatt_svc_t mrt_gatt_svc_t;                             //forward declare for self referencing callback
typedef struct mrt_gatt_char_t mrt_gatt_char_t;                            //forward declare for self referencing callback
typedef struct mrt_gatt_evt_t mrt_gatt_evt_t;                             //forward declare for self referencing callback


typedef mrt_status_t (*mrt_gatt_svc_callback)(mrt_gatt_svc_t* svc, mrt_gatt_evt_t* event); 
typedef mrt_status_t (*mrt_gatt_char_callback)(mrt_gatt_evt_t* event); 

/* gatt Event struct*/
typedef struct{
    uint8_t mType;              /*Type of event*/
    uint16_t mHandle;           /* Handle*/
    mrt_gatt_char_t* mChar;     /*Characteristic for event*/
    struct{
        uint8_t* data;          /* ptr to data for writes (or notify on client)*/
        int len;                /* len of data in bytes*/
    }mData;
    void* mCtx;                 /* Allows struct to reference platform specific objec if needed */
    mrt_status_t mStatus;       /* Status */
} mrt_gatt_evt_t;

typedef struct{
    uint16_t mChar;             //Character declaration handle
    uint16_t mValue;            //Value handle 
    uint16_t mCCCD;             //Client Config handle
}mrt_gatt_handles_t;


/**
 * Gatt Characteristic Struct
 * 
 * This is a single data field on the gatt server
 */ 
struct mrt_gatt_char_t{
    mrt_gatt_uuid_t mUuid;          //UUID of Characteristic
    uint16_t mSize;                 //size of Characteristic
    mrt_gatt_handles_t mHandles;               //Handles of Characteristic
    uint8_t mProps;                 //Permissions of characteristic
    uint8_t mSecurity;              //Security flags for characteristic
    bool mNotificationsEnable;      //Indicates if Notifications are enabled in CCCD
    struct{
        uint8_t* mData;              // Cached data
        uint8_t mLen;                    // len of last cached data (data invalid if 0)
    }mCache;
    uint16_t mCCCD;
    mrt_gatt_svc_t* mSvc;           //ptr to service
    mrt_gatt_char_callback cbEvent; //characteristic event callback
    const char* mName;              // Name
    void* mCtx;                     //Allows struct to reference other objects in adapters.
};

/**
 * Gatt Service Struct
 * 
 * A service is just a collection of related characteristics
 */ 
struct mrt_gatt_svc_t{
    mrt_gatt_uuid_t mUuid;          //UUID of Service
    uint16_t  mHandle;              //Handle
    mrt_gatt_char_t** mChars;       //Array of characteristics   
    uint16_t mCharCount;            //Number of characterestics
    uint16_t mMaxCharCount;         //max number of characteristics
    uint16_t mAttrCount;             //Number of attributes in service. 
    uint8_t mSecurity;              //Security flags for service, if set this will override characteristic level security flags
    mrt_gatt_pro_t* mPro;           //Ptr to profile
    mrt_gatt_svc_callback cbEvent;  //Service event callback
    const char* mName;              // Name
    void* mCtx;                     //Allows struct to reference other objects in adapters.
};


/**
 * Gatt Profile Struct 
 * 
 * A Profile is a collection of services
 */
struct mrt_gatt_pro_t{
    uint32_t mId;                 //Some platforms use an ID when running multiple profiles
    mrt_gatt_svc_t** mSvcs;       //Array of Services   
    uint16_t mSvcCount;           //Number of Services
    uint16_t mMaxSvcCount;        //max number of Services
    const char* mName;              // Name
    void* mCtx;                   //Allows struct to reference other objects in adapters.
};



/* Exported functions ------------------------------------------------------- */

/**
 * @brief Initialize profile struct
 * @param profile - ptr to profile
 * @param serviceCount - number of services in profile
 * @param id - profile ID
 * @param name - name of characteristic
 */
mrt_status_t mrt_gatt_init_pro(mrt_gatt_pro_t* pro, uint16_t serviceCount, uint32_t id, const char* name);   


/**
 * @brief Adds a service to the profile struct
 * @param profile ptr to profile
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_add_svc(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc );

/**
 * @brief Initialize service struct
 * @param svc - ptr to struct
 * @param uuidType - uuid type (e16Bit or e128Bit)
 * @param arrUuid - array of bytes representing UUID
 * @param charCount - number of characteristics in service 
 * @param cbEvent - callback event (unused for now)
 * @param name - name of characteristic
 */
mrt_status_t mrt_gatt_init_svc(mrt_gatt_svc_t* svc, uint8_t uuidType, const uint8_t* arrUuid, uint16_t charCount, mrt_gatt_svc_callback cbEvent, const char* name);

/**
 * @brief Initializes a characteristic and adds it to a service
 * @param chr - ptr to characteristic
 * @param svc - ptr to service
 * @param uuidType - uuid type (e16Bit or e128Bit)
 * @param arrUuid - array of bytes representing UUID
 * @param size - size of data in bytes
 * @param props - properties (READ,WRITE,NOTIFY etc)
 * @param cbEvent - callback handler for gatt event 
 * @param name - name of characteristic
 */
mrt_status_t mrt_gatt_init_char(mrt_gatt_svc_t* svc, mrt_gatt_char_t* chr, uint8_t uuidType, const uint8_t* arrUuid, uint16_t size, uint8_t props, mrt_gatt_char_callback cbEvent,const char* name );

/**
 * @brief Sets the default security flags for intializing characteristics  
 * @param securityFlags new default flags
 * @return mrt_status_t 
 * @note must be called before initializing characteristics
 */
mrt_status_t mrt_gatt_set_default_security(uint8_t securityFlags);


/**
 * @brief checks if characteristic contains the handle
 * 
 * @param chr 
 * @param handle 
 * @return true if handle matches any of the characteristic handles
 */
bool mrt_gatt_char_has_handle(mrt_gatt_char_t* chr, uint16_t handle);

/**
 * @brief looks for a service in a profile by service uuid
 * @param pro ptr to profile
 * @param uuid uuid to match
 * @return ptr to service or NULL if not found 
 */
mrt_gatt_svc_t* mrt_gatt_lookup_svc_uuid(mrt_gatt_pro_t* pro, mrt_gatt_uuid_t* uuid);

/**
 * @brief looks for a chacateristic in a service or profile by uuid.
 * @param pro ptr to profile, NULL if searching service
 * @param svc ptr to service, NULL if searching profile
 * @param uuid uuid to match
 * @return ptr to characteristic or NULL if not found
 */
mrt_gatt_char_t* mrt_gatt_lookup_char_uuid(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc, mrt_gatt_uuid_t* uuid);

/**
 * @brief looks for a service in a profile by service handle
 * @param pro ptr to profile
 * @param handle handle to match
 * @return ptr to service or NULL if not found 
 */
mrt_gatt_svc_t* mrt_gatt_lookup_svc_handle(mrt_gatt_pro_t* pro, uint16_t handle);

/**
 * @brief looks for a chacateristic in a service or profile by handle.
 * @param pro ptr to profile, NULL if searching service
 * @param svc ptr to service, NULL if searching profile
 * @param handle handle to match
 * @return ptr to characteristic or NULL if not found
 */
mrt_gatt_char_t* mrt_gatt_lookup_char_handle(mrt_gatt_pro_t* pro, mrt_gatt_svc_t* svc, uint16_t handle);



/* Deinit Functions ------------------------------------------------------- */

/**
 * These functions free up resources allocated during initialization
 * Because the Gatt Server will be up for the full lifecycle of most applications, they are not needed
 * They are mainly used for detecting memory leaks in unit testing
 */
void mrt_gatt_deinit_pro(mrt_gatt_prof_t* pro);
void mrt_gatt_deinit_svc(mrt_gatt_svc_t* svc);
void mrt_gatt_deinit_chr(mrt_gatt_char_t* chr);


/* Weak Functions -------------------------------------------------------- */


/**
 * @brief Updates the characteristic value
 * @param chr ptr to char
 * @param data data to update with
 * @param len length of data in bytes
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_update_char_val(mrt_gatt_char_t* chr, uint8_t* data, uint16_t len);

/**
 * @brief Gets the characteristic and updates the local cache (chr->mCache.mData) from the actual data on the device
 * @param chr ptr to char
 * @return status
 */
mrt_status_t mrt_gatt_get_char_val(mrt_gatt_char_t* chr);

/**
 * @brief This functions registers the service with the actual ble platform
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_svc(mrt_gatt_svc_t* svc);

/**
 * @brief This functions registers the characteristic with the actual ble platform 
 *
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_char(mrt_gatt_char_t* chr);


#ifdef __cplusplus
}
#endif
#endif /*MRT_GATT_INTERFACE_H_*/