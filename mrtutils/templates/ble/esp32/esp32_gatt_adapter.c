/**
  *@file esp32_gatt_adapter.c
  *@brief header for 
  *@author Jason Berger
  *@date 10/16/21
  */

/* Includes ------------------------------------------------------------------*/
#include "esp32_gatt_adapter.h"

#define GATT_ADAPTER_TAG "MRT"


/* Private Variables ---------------------------------------------------------*/
static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY | ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;


/* Exported functions ------------------------------------------------------- */
void mrt_gatt_set_interface(mrt_gatt_pro_t* pro, esp_gatt_if_t gatts_if)
{

  esp32_adapter_ctx_t* ctx = malloc(sizeof(esp32_adapter_ctx));
  ctx->gatts_if = gatts_if;

  pro->mRef = (void*) ctx;

}

mrt_status_t mrt_gatt_set_handles(mrt_gatt_pro_t* pro, esp_bt_uuid_t* svc_uuid,uint16_t* handles, int len )
{
  int cur = 0;

  //Convert uuid to mrt 
  mrt_gatt_uuid_t uuid = {
    .mLen = svc_uuid->len,
    .mUuid = svc_uuid->uuid
  };

  //Find service in profile by uuid
  mrt_gatt_svc_t* svc = mrt_gatt_lookup_svc_uuid(pro, &uuid);

  if(svc == NULL)
  {
    ESP_LOGI(GATT_ADAPTER_TAG, "Failed to find Service" );
    return MRT_STATUS_ERROR;
  }

  if(len != svc->mAttrCount)
  {
    ESP_LOGI(GATT_ADAPTER_TAG, "Unexpected Handle count. expected %d, received %d",svc->mAttrCount, len);
    return MRT_STATUS_ERROR;
  }

  svc->mHandle = handles[cur++];

  for(uint32_t i = 0; i < svc->mCharCount; i++)
  {
      svc->mChars[i]->mHandles.mchar = handles[cur++];
      svc->mChars[i]->mHandles.mValue = handles[cur++];

      if((svc->mChars[i]->mProps | MRT_GATT_PROP_NOTIFY) > 0)
      {
        svc->mChars[i]->mHandles.mCCCD = handles[cur++];  
      }
  }

  return MRT_STATUS_OK;
}


mrt_gatt_evt_t mrt_gatt_convert_evt(esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{
    mrt_gatt_evt_t mrt_evt; 

    mrt_evt.mType = MRT_GATT_EVT_NONE;
    mrt_evt.mCtx = (void*)param;          //Not currently used, but allows reference to the native esp_ble_gatts_cb_param_t in the event handler if needed
    
    switch(event)
    {
        case ESP_GATTS_WRITE_EVT:
            mrt_evt.mHandle = param->write.handle;
            mrt_evt.mData.len = param->write.len;
            mrt_evt.mData.data = param->write.value;

            //Check if it is writing the value attribute, or the CCCD
            if(mrt_evt.mChar->mHandles.mValue == param->write.handle)
            {
              mrt_evt.mType = MRT_GATT_EVT_WRITE;
            }
            else
            {
              mrt_evt.mType = MRT_GATT_EVT_CCCD_WRITE;
            }
            break;
        case ESP_GATTS_READ_EVT:       
            mrt_evt.mType = MRT_GATT_EVT_VALUE_READ;
            mrt_evt.mHandle = param->read.handle
            break;
        default:
            mrt_evt.mType = MRT_GATT_EVT_NONE;
            mrt_evt.mData.len = 0;
            mrt_evt.mData.data = NULL;
            mrt_evt.mHandle = 0;
            break;
        
    }   

    return mrt_evt;
}

mrt_gatt_evt_t mrt_gatt_handle_evt(mrt_gatt_pro_t* pro, esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{
  //Convert event data
  mrt_gatt_evt_t mrt_evt = mrt_gatt_convert_evt(event, param);

  //Lookup characteristic
  mrt_evt.mChar = mrt_gatt_lookup_char_handle(pro, NULL, mrt_evt.mHandle);

  if(mrt_evt.mChar == NULL)
  {
      ESP_LOGE(GATT_ADAPTER_TAG, "Failed to find characteristic with handle %04X", mrt_evt.mHandle );
      return;
  }

  //If it is a 'Write' event, check if it is writing the CCCD and change event type if needed
  if((mrt_evt.mType == MRT_GATT_EVT_WRITE) && (mrt_evt.mChar->mHandles.mCCCD == mrt_evt.handle)
  {
    mrt_evt.mType = MRT_GATT_EVT_CCCD_WRITE;
  }

  /**
   * ** BY DEFAULT WE ONLY PASS WRITE EVENTS TO THE CHARACTERISTIC CALLBACK HANDLERS **
   * To pass all events to the handler, comment out the following line
   */
  if(mrt_evt.mType != MRT_GATT_EVT_WRITE){return mrt_evt;}


  //By default only call the event callback for write events because READ events are auto-response by default
  if((mrt_evt.mChar != NULL) && (mrt_evt.mType != MRT_GATT_EVT_NONE))
  {
     mrt_evt.mStatus = mrt_evt.mChar->cbEvent(&mrt_evt);
  }

  return mrt_evt;
}

/* Override Funtions for mrt_gatt_interface  ---------------------------------*/

/**
 * @brief Updates the characteristic value
 * @param chr ptr to char
 * @param data data to update with
 * @param len length of data in bytes
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_update_char_val(mrt_gatt_char_t* chr, uint8_t* data, int len)
{
    //Update attribute
    esp_err_t error = esp_ble_gatts_set_attr_value(chr->mHandles.mValue, len, data);
    if (error != ESP_OK) {
        ESP_LOGI(GATT_ADAPTER_TAG, "UPDATE: Failed to set handle: (%x): GATT Char, %x", chr->mHandles.mValue, error);
        return MRT_STATUS_ERROR;
    }

    //Read back the value to make sure cache is in sync
    mrt_gatt_get_char_val(mrt_gatt_char_t* chr);

    return MRT_STATUS_OK;
}

/**
 * @brief Gets the characteristic and updates the local cache (chr->mCache.mData) from the actual data on the device
 * @param chr ptr to char
 * @return status
 */
mrt_status_t mrt_gatt_get_char_val(mrt_gatt_char_t* chr)
{
    //Read back the value to make sure cache is in sync
    esp_ble_gatts_get_attr_value(chr->mHandles.mValue, &chr->mCache.mLen, (const uint8_t **) &chr->mCache.mData);
}

/**
 * @brief This functions registers the service by generating a service table and registering it
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_svc(mrt_gatt_svc_t* svc)
{
  //Get context from profile
  esp32_adapter_ctx_t* ctx = (esp32_adapter_ctx_t*)svc->mPro->mCtx;
  uint32_t idx =0;

  //Create table //TODO verify it doesnt need to be allocated 
  esp_gatts_attr_db_t gatt_db[svc->mAttrCount]; 
  uint16_t esp_perm;
  uint8_t* esp_prop;


  //Service Attribute
  gatt_db[idx++] =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      svc->mUuid.mLen, svc->mUuid.mLen, (uint8_t *) svc->mUuid.mUuid}};

  for(uint32_t i=0; i < svc->mCharCount; i++)
  {
    mrt_gatt_char_t* = svc->mChars[i];

    esp_perm = 0;
    esp_prop = &char_prop_read;


    //Permissions 
    
    if(chr->mProps | MRT_GATT_PROP_READ) 
    {
      esp_perm |= ESP_GATT_PERM_READ;
    }
    else 
    {
      esp_prop = &char_prop_write;
    }
    if(chr->mProps | MRT_GATT_PROP_WRITE) 
    {
      esp_perm |= ESP_GATT_PERM_WRITE;
      esp_prop = &char_prop_read_write;
    }
    if(chr->mProps | MRT_GATT_PROP_NOTIFY) 
    {
      esp_perm |= ESP_GATT_PERM_NOTIFY;
      if(chr->mProps | MRT_GATT_PROP_WRITE)
      {
        esp_prop = &char_prop_read_write_notify;
      }
      else 
      {
        esp_prop = &char_prop_read_notify;
      }
    }

    //Characteristic Declaration Attribute
    gatt_db[idx++] =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      sizeof(uint8_t), sizeof(uint8_t), esp_prop}};

    //Characteristic Value Attribute
    gatt_db[idx++] =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, esp_perm,
      chr->mSize, chr->mCache.mLen, chr->mCache.mData }};

    if(chr->mProps | MRT_GATT_PROP_NOTIFY)
    {
          //Characteristic CCCD Attribute
          gatt_db[idx++] =  {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, esp_perm,
            sizeof(chr->mCCCD), sizeof(chr->mCCCD), (uint8_t*) &chr->mCCCD }};
    }


  }

  esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, ctx->gatts_if, svc->mAttrCount, 0);
            if (create_attr_ret){
                ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);

                return MRT_STATUS_ERROR;
            }

  return MRT_STATUS_OK;

}

/**
 * @brief This functions registers the characteristic with the actual ble platform 
 *
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_char(mrt_gatt_char_t* chr)
{ 
  //
  //  Because the characteristics are registered in mrt_gatt_register_svc, we dont have to register them individually 
  //
  return MRT_STATUS_OK;
}