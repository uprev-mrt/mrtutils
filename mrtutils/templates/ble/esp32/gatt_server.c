/**
 * @file ${obj.name.lower()}_gatt_server.h
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#include "gatt_server.c"



/*user-block-top-start*/
#define ${obj.name.upper()}_PROFILE_ID 0;
#define ${obj.name.upper()}_DEVICE_NAME          "${obj.name.upper()}_DEVICE"

static esp_ble_adv_params_t data_adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/*user-block-top-end*/

/* Private Variables ---------------------------------------------------------*/
MRT_GATT_DATA_ATTR mrt_gatt_pro_t ${obj.name}_profile;


/* Private Functions ---------------------------------------------------------*/


/* Exported Functions --------------------------------------------------------*/

mrt_status_t ${obj.name.lower()}_profile_init(void)
{
    mrt_gatt_init_pro(&${obj.name}_profile.mPro, ${len(obj.services)}, 0, "${obj.name}");
    
    /* Initialize all services */
    %for svc in obj.services:
    ${"{0}_profile.m{1} = {2}_svc_init(&{0}_profile.mPro);".format(obj.name.lower(),t.camelCase(svc.name), svc.prefix)}
    %endfor
    
    return MRT_STATUS_OK;
}

mrt_status_t ${obj.name.lower()}_profile_register_services(void)
{
    mrt_status_t status;
    for(uint16_t i =0; i < ${obj.name}_profile.mSvcCount; i++ )
    {
        status = mrt_gatt_register_svc(${obj.name}_profile.mPro.mSvcs[i]);
        if(status != MRT_STATUS_OK)
        {
            break;
        }

    }

    return status;
}



esp_err_t ${obj.name.lower()}_gatts_start(void)
{
    esp_err_t ret;

    ${obj.name.lower()}_profile_init(void);
    ${obj.name}_profile.mId = ${obj.name.upper()}_PROFILE_ID;

    //Register event handler
    ret = esp_ble_gatts_register_callback(${obj.name.lower()}_gatts_evt_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return ret;
    }

    //Register "App", which will start the process and generate a ESP_GATTS_REG_EVT event
    ret = esp_ble_gatts_app_register(${obj.name}_profile.mPro.mId);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return ret;
    }



}


void ${obj.name.lower()}_gatts_evt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
        case ESP_GATTS_REG_EVT:

            //Set Device name
            esp_ble_gap_set_device_name(${obj.name.upper()}_DEVICE_NAME );

            //Register all of the services in our profile
            ${obj.name.lower()}_profile_register_services();

        case ESP_GATTS_READ_EVT:
        case ESP_GATTS_WRITE_EVT:
            mrt_gatt_evt_t mrt_gatt_handle_evt(mrt_gatt_pro_t* pro, esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param);
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
                /**
                 * Every time an attribute table is registered, it will trigger this event and pass back an array of 'handles'
                 * These arrays are passed to the profile to sort them out and assign them to characteristics
                 */
                mrt_gatt_set_handles(&${obj.name}_profile,&param->add_attr_tab.svc_uuid ,param->add_attr_tab.handles, param->add_attr_tab.num_handle );
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



/*user-block-functions-start*/
/*user-block-functions-end*/

