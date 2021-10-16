/**
 * @file ${obj.name.lower()}_profile.h
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#include "${obj.name.lower()}_profile.h"



/*user-block-top-start*/

static esp_ble_adv_params_t data_adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/*user-block-top-end*/



void ${obj.name.lower()}_profile_init(void)
{
    /* Initialize all services */
    %for svc in obj.services:
    ${svc.prefix}_svc_init();
    %endfor

}

mrt_gatt_svc_t*

void ${obj.name.lower()}_profile_create_services(esp_gatt_if_t gatts_if);
{
    uint8_t id =0;

    /* Add Attribute tables for all services */
    %for svc in obj.services:
    ${"esp_ble_gatts_create_attr_tab({0}_svc_attr_db, gatts_if, IDX_{1}_NB, id++);".format(svc.prefix.lower(),svc.prefix.upper())}
    %endfor


}

void ${obj.name.lower()}_profile_set_handles(esp_bt_uuid_t* svc_uuid,uint16_t* handles, int len )
{
    /* Find Service with matching UUID and pass handles*/
    %for svc in obj.services:
    if((svc_uuid->len == ${obj.prefix}_svc.mUuid.mLen) && (memcmp( (uint8_t*)svc_uuid->uuid, (uint8_t*)${obj.prefix}_svc->mUuid.m16Bit, svc_uuid->len) == 0))
    {
        ${svc.prefix}_svc_set_handles(handles,len);
        return;
    }
    %endfor

}

void ${obj.name.lower()}_gatt_write_handler(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    //TODO dispatch from param->write.handle
}

void ${obj.name.lower()}_gatt_read_handler(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    //TODO dispatch from param->write.handle
}

void ${obj.name.lower()}_profile_set_handles(esp_bt_uuid_t svc_uuid,uint16_t* handles, int len )
{
    //TODO Dispatch
}

/*user-block-functions-start*/

static void ${obj.name.lower()}_gatts_evt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:

            esp_ble_gap_set_device_name(DeviceName);

            ${obj.name.lower()}_profile_create_services(gatts_if);
        case ESP_GATTS_READ_EVT:
            ${obj.name.lower()}_gatt_read_handler(gatts_if, param);
            break;
        case ESP_GATTS_WRITE_EVT:
            ${obj.name.lower()}_gatt_write_handler(gatts_if, param);
            break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            break;
        case ESP_GATTS_MTU_EVT:
            break;
        case ESP_GATTS_CONF_EVT:
            break;
        case ESP_GATTS_UNREG_EVT:
            break;
        case ESP_GATTS_DELETE_EVT:
            break;
        case ESP_GATTS_START_EVT:
            break;
        case ESP_GATTS_STOP_EVT:
            break;
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(GATTS_SERVER_TAG, "ESP_GATTS_CONNECT_EVT");
            #ifdef ENABLE_SECURE_BT_PAIRING
				ESP_LOGI(GATTS_SERVER_TAG, "ESP_GATTS_CONNECT_EVT: Secure Pairing Enabled");
                if(isInPairingMode) // Allow all incoming connections while in paring mode
                {
					/* start security connect with peer device when receive the connect event sent by the master */
					esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
					current_connection_id = param->connect.conn_id;
                }
				else // If we are not in pairing mode and this device is not allowed, then disconnect
				{
					if (!check_bonded_devices(param->connect.remote_bda))
                    {
                            
                        ESP_LOGI(GATTS_SERVER_TAG, "Invalid remote address."); 
                        esp_ble_gap_disconnect(param->connect.remote_bda);
                    } 
                    else
                    {
                        /* start security connect with peer device when receive the connect event sent by the master */
                        esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
                        current_connection_id = param->connect.conn_id;                
                    }
				}
			#else
				ESP_LOGI(GATTS_SERVER_TAG, "ESP_GATTS_CONNECT_EVT: Secure Pairing Disabled");
				/* start security connect with peer device when receive the connect event sent by the master */
				esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
				current_connection_id = param->connect.conn_id;
			#endif                
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(GATTS_SERVER_TAG, "ESP_GATTS_DISCONNECT_EVT");
            /* start advertising again when missing the connect */
            esp_ble_gap_start_advertising(&data_adv_params);
            current_connection_id = 0;
            break;
        case ESP_GATTS_OPEN_EVT:
            break;
        case ESP_GATTS_CANCEL_OPEN_EVT:
            break;
        case ESP_GATTS_CLOSE_EVT:
            break;
        case ESP_GATTS_LISTEN_EVT:
            break;
        case ESP_GATTS_CONGEST_EVT:
            break;
        case ESP_GATTS_SET_ATTR_VAL_EVT:
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
            ESP_LOGI(GATTS_SERVER_TAG, "Create attr tab : handle = %x",param->add_attr_tab.num_handle);
            if (param->create.status == ESP_GATT_OK)
            {
            
                ${obj.name.lower()}_profile_set_handles(&param->add_attr_tab.svc_uuid,param->add_attr_tab.handles, param->add_attr_tab.num_handle);

            }
            else
            {
                ESP_LOGE(GATTS_SERVER_TAG, " Create attribute table failed, error code = %x", param->create.status);
				errorLogUpdateRAM(ERRLOG_GATT_CREATE_ATT_TBL_FAIL, __LINE__, param->create.status);
            }
        break;
    }
        default:
           break;
    }
}



/*user-block-functions-end*/

