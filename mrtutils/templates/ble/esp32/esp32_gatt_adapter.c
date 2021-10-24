/**
  *@file esp32_gatt_adapter.c
  *@brief header for 
  *@author Jason Berger
  *@date 10/16/21
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "esp32_gatt_adapter.h"

#define GATT_ADAPTER_TAG "MRT"


/* Private Macros ------------------------------------------------------------*/

/**
 * @brief defines an attribute in the attribute table
 */
#define DEF_ATTR_DB_T(_attr, _auto_rsp, _uuidlen, _uuid, _perm, _maxlen, _len, _val ) {\\

    _attr.attr_control.auto_rsp =   _auto_rsp;\\

    _attr.att_desc.uuid_length  =   _uuidlen;\\

    _attr.att_desc.uuid_p       =   (uint8_t*) _uuid;\\

    _attr.att_desc.perm         =   _perm;\\

    _attr.att_desc.max_length   =   _maxlen;\\

    _attr.att_desc.length       =   _len;\\

    _attr.att_desc.value        =   (uint8_t *) _val ;\\

 }

void print_attr(esp_gatts_attr_db_t* attr )
{
  char buf[512];
  int cur = 0;

  cur+= sprintf(&buf[cur], " AutoRsp: %d\n", attr->attr_control.auto_rsp );
  cur+= sprintf(&buf[cur], " UUID Len: %d\n", attr->att_desc.uuid_length );
  cur+= sprintf(&buf[cur], " UUID P: %p\n", attr->att_desc.uuid_p );
  cur+= sprintf(&buf[cur], " PERM: %d\n", attr->att_desc.perm );
  cur+= sprintf(&buf[cur], " max_length: %d\n", attr->att_desc.max_length );
  cur+= sprintf(&buf[cur], " len: %d\n", attr->att_desc.length );
  cur+= sprintf(&buf[cur], " val: %p\n", attr->att_desc.value );

  ESP_LOGI(GATT_ADAPTER_TAG, "%s", buf );
}

void print_svc(mrt_gatt_svc_t* svc)
{
   char buf[512];
  int cur = 0;

  cur+= sprintf(&buf[cur], " Name: %s\n", svc->name );
  cur+= sprintf(&buf[cur], " Name: %d\n", svc->handle );
  cur+= sprintf(&buf[cur], " Name: %d\n", svc->attrCount );
  cur+= sprintf(&buf[cur], " Name: %d\n", svc->charCount );


  ESP_LOGI(GATT_ADAPTER_TAG, "%s", buf );
}

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

mrt_status_t mrt_gatt_set_handles(mrt_gatt_svc_t* svc,uint16_t* handles, int len )
{
  int cur = 0;

  if(svc == NULL)
  {
    ESP_LOGE(GATT_ADAPTER_TAG, "Error, NULL service" );
    return MRT_STATUS_ERROR;
  }


  if(len != svc->attrCount)
  {
    ESP_LOGE(GATT_ADAPTER_TAG, "Unexpected Handle count. expected %d, received %d",svc->attrCount, len);
    return MRT_STATUS_ERROR;
  }

  svc->handle = handles[cur++];

  for(uint32_t i = 0; i < svc->charCount; i++)
  {
      //ESP_LOGI(GATT_ADAPTER_TAG, "Char '%s' :  %x", svc->chars[i]->name, handles[cur]);
      svc->chars[i]->handles.char_handle = handles[cur++];
      //ESP_LOGI(GATT_ADAPTER_TAG, "Char '%s' -> Val: %x", svc->chars[i]->name, handles[cur]);
      svc->chars[i]->handles.val_handle = handles[cur++];

      if((svc->chars[i]->props & MRT_GATT_PROP_NOTIFY) > 0)
      {
      //ESP_LOGI(GATT_ADAPTER_TAG, "Char '%s' ->CCCD: %x", svc->chars[i]->name, handles[cur]);
        svc->chars[i]->handles.cccd_handle = handles[cur++];  
      }
  }

  return MRT_STATUS_OK;
}


mrt_gatt_evt_t mrt_gatt_convert_evt(esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{
    mrt_gatt_evt_t mrt_evt; 

    mrt_evt.type = MRT_GATT_EVT_NONE;
    
    switch(event)
    {
        case ESP_GATTS_WRITE_EVT:
            mrt_evt.handle = param->write.handle;
            mrt_evt.data.len = param->write.len;
            mrt_evt.data.value = param->write.value;
            mrt_evt.type = MRT_GATT_EVT_VALUE_WRITE;
            
            break;
        case ESP_GATTS_READ_EVT:       
            mrt_evt.type = MRT_GATT_EVT_VALUE_READ;
            mrt_evt.handle = param->read.handle;
            break;
        default:
            mrt_evt.type = MRT_GATT_EVT_NONE;
            mrt_evt.data.len = 0;
            mrt_evt.data.value = NULL;
            mrt_evt.handle = 0;
            break;
        
    }   

    return mrt_evt;
}

mrt_gatt_uuid_t mrt_gatt_convert_uuid( esp_bt_uuid_t* esp_uuid)
{
  mrt_gatt_uuid_t uuid;
  uuid.len = esp_uuid->len;
  memcpy(uuid.uuid128, esp_uuid->uuid.uuid128, uuid.len);

  return uuid;
}


mrt_gatt_evt_t mrt_gatt_handle_evt(mrt_gatt_pro_t* pro, esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{

  //Convert event data
  mrt_gatt_evt_t mrt_evt = mrt_gatt_convert_evt(event, param);

  //Lookup characteristic
  mrt_evt.chr = mrt_gatt_lookup_char_handle(pro, NULL, mrt_evt.handle);

  if(mrt_evt.chr == NULL)
  {
      ESP_LOGE(GATT_ADAPTER_TAG, "Failed to find characteristic with handle %04X", mrt_evt.handle );
      return mrt_evt;
      
  }

  //ESP_LOGE(GATT_ADAPTER_TAG, "Event Type: %04X", mrt_evt.type );

  //If it is a 'Write' event, check if it is writing the CCCD and change event type if needed
  if((mrt_evt.type == MRT_GATT_EVT_VALUE_WRITE) && (mrt_evt.chr->handles.cccd_handle == mrt_evt.handle))
  {
    //ESP_LOGE(GATT_ADAPTER_TAG, "CCCD Write: %d, %04X", mrt_evt.data.len, *((uint16_t*)mrt_evt.data.value));
    mrt_evt.type = MRT_GATT_EVT_CCCD_WRITE;
    mrt_evt.chr->cccd = *((uint16_t*)mrt_evt.data.value);
  }

  /**
   * ** BY DEFAULT WE ONLY PASS WRITE EVENTS TO THE CHARACTERISTIC CALLBACK HANDLERS **
   * To pass all events to the handler, comment out the following line
   */
  if(mrt_evt.type != MRT_GATT_EVT_VALUE_WRITE){return mrt_evt;}


  //By default only call the event callback for write events because READ events are auto-response by default
  if((mrt_evt.chr != NULL) && (mrt_evt.type != MRT_GATT_EVT_NONE))
  {
    //ESP_LOGE(GATT_ADAPTER_TAG, "CALLBACK  Event Type: %04X", mrt_evt.type );
     mrt_evt.status = mrt_evt.chr->cbEvent(&mrt_evt);
  }

  return mrt_evt;
}

void mrt_gatt_print_uuid(esp_bt_uuid_t* esp_uuid, mrt_gatt_uuid_t* mrt_uuid)
{
  mrt_gatt_uuid_t uuid;
  char str[256] = {0};
  int cur = 0;

  if(mrt_uuid == NULL)
  {
    uuid = mrt_gatt_convert_uuid(esp_uuid);
    mrt_uuid = &uuid;
  }

  cur = sprintf(str,"UUID->: "); 
  for(int i=0; i < mrt_uuid->len; i++ )
  {
    cur+= sprintf(&str[cur],"%02X", mrt_uuid->val[i]);
  }

  ESP_LOGI(GATT_ADAPTER_TAG,"%s",str);

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
    esp_err_t error = esp_ble_gatts_set_attr_value(chr->handles.val_handle, len, data);
    if (error != ESP_OK) {
        ESP_LOGI(GATT_ADAPTER_TAG, "UPDATE: Failed to set handle: (%x): GATT Char, %x", chr->handles.val_handle, error);
        return MRT_STATUS_ERROR;
    }

    //Send notifications/indications if enabled 
    if(chr->cccd & (MRT_CCCD_NOTIFY_ENABLED | MRT_CCCD_INDICATE_ENABLED ))
    {
      error = esp_ble_gatts_send_indicate(((mrt_profile_ctx_t*)chr->svc->pro->ctx)->gatts_if ,     
                                          ((mrt_profile_ctx_t*)chr->svc->pro->ctx)->conn_id,
                                          chr->handles.val_handle,
                                          len,
                                          data,
                                          (chr->cccd & MRT_CCCD_INDICATE_ENABLED) >> 1);
    }

    //Read back the value to make sure cache is in sync
    mrt_gatt_get_char_val(chr);

    return MRT_STATUS_OK;
}

/**
 * @brief Gets the characteristic and updates the local cache (chr->cache.data) from the actual data on the device
 * @param chr ptr to char
 * @return status
 */
mrt_status_t mrt_gatt_get_char_val(mrt_gatt_char_t* chr)
{
    //Read back the value to make sure cache is in sync
    esp_ble_gatts_get_attr_value(chr->handles.val_handle, &chr->data.len, (const uint8_t **) &chr->data.value);

    return MRT_STATUS_OK;
}

/**
 * @brief This functions registers the service by generating a service table and registering it
 * @param svc ptr to service
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_svc(mrt_gatt_svc_t* svc,esp_gatt_if_t gatts_if)
{
  //Get context from profile
  uint32_t idx =0;

  //Create table //TODO verify it doesnt need to be allocated 
  esp_gatts_attr_db_t gatt_db[svc->attrCount]; 
  uint16_t esp_perm;
  uint8_t* esp_prop;


  //Service Attribute
  DEF_ATTR_DB_T(gatt_db[idx],ESP_GATT_AUTO_RSP, ESP_UUID_LEN_16,&primary_service_uuid,ESP_GATT_PERM_READ,
                svc->uuid.len, svc->uuid.len, svc->uuid.val);
  idx++;

  
  for(uint32_t i=0; i < svc->charCount; i++)
  {
    mrt_gatt_char_t* chr = svc->chars[i];

    esp_perm = 0;
    esp_prop = &char_prop_read;


    //Permissions 
    
    if(chr->props & MRT_GATT_PROP_READ) 
    {
      esp_perm |= ESP_GATT_PERM_READ;
    }
    else 
    {
      esp_prop = &char_prop_write;
    }
    if(chr->props & MRT_GATT_PROP_WRITE) 
    {
      esp_perm |= ESP_GATT_PERM_WRITE;
      esp_prop = &char_prop_read_write;
    }
    if(chr->props & MRT_GATT_PROP_NOTIFY) 
    {
      esp_perm |= ESP_GATT_PERM_READ;
      if(chr->props & MRT_GATT_PROP_WRITE)
      {
        esp_prop = &char_prop_read_write_notify;
      }
      else 
      {
        esp_prop = &char_prop_read_notify;
      }
    }

    //Characteristic Declaration Attribute
    DEF_ATTR_DB_T(gatt_db[idx],ESP_GATT_AUTO_RSP, ESP_UUID_LEN_16,&character_declaration_uuid,ESP_GATT_PERM_READ,
                  sizeof(uint8_t), sizeof(uint8_t), esp_prop);
    idx++;

    

    //Characteristic Value Attribute
    DEF_ATTR_DB_T(gatt_db[idx],ESP_GATT_AUTO_RSP, chr->uuid.len,chr->uuid.val,esp_perm,
                  chr->size, chr->data.len, chr->data.value);
    idx++;

    if(chr->props & MRT_GATT_PROP_NOTIFY)
    {
          /* Client Characteristic Configuration Descriptor */
          DEF_ATTR_DB_T(gatt_db[idx],ESP_GATT_AUTO_RSP, ESP_UUID_LEN_16,&character_client_config_uuid ,ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                        sizeof(uint16_t), sizeof(uint16_t), &chr->cccd );
          idx++;



    }




  }

  esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, svc->attrCount, 0);
            if (create_attr_ret){
                ESP_LOGE(GATT_ADAPTER_TAG, "create attr table failed, error code = %x", create_attr_ret);

                return MRT_STATUS_ERROR;
            }

  return MRT_STATUS_OK;

}