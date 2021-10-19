/**
 * @file ${obj.name.lower()}_gatt_server.c
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#include "${obj.name.lower()}_gatt_server.h"




/*user-block-top-start*/
#define ${obj.name.upper()}_PROFILE_ID 0
#define ${obj.name.upper()}_PROFILE_TAG          "${obj.name.upper()}_PROFILE"
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
MRT_GATT_DATA_ATTR ${obj.name.lower()}_profile_t ${obj.name.lower()}_profile;


/* Private Functions ---------------------------------------------------------*/


/* Exported Functions --------------------------------------------------------*/

mrt_status_t ${obj.name.lower()}_profile_init(void)
{
    mrt_gatt_init_pro(&${obj.name.lower()}_profile.mPro, ${len(obj.services)}, 0, "${obj.name}");
    
    /* Initialize all services */
    %for svc in obj.services:
    ${"{0}_profile.m{1} = {2}_svc_init(&{0}_profile.mPro);".format(obj.name.lower(),t.camelCase(svc.name), svc.prefix)}
    %endfor
    
    return MRT_STATUS_OK;
}

mrt_status_t ${obj.name.lower()}_profile_register_services(esp_gatt_if_t gatts_if)
{
    mrt_status_t status = MRT_STATUS_ERROR;
    for(uint16_t i =0; i < ${obj.name.lower()}_profile.mPro.svcCount; i++ )
    {
        status = mrt_gatt_register_svc(${obj.name.lower()}_profile.mPro.svcs[i],gatts_if);
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

    ${obj.name.lower()}_profile_init();
    ${obj.name.lower()}_profile.mPro.id = ${obj.name.upper()}_PROFILE_ID;

    //Register event handler
    ret = esp_ble_gatts_register_callback(${obj.name.lower()}_gatts_evt_handler);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gatts register error, error code = %x", ret);
        return ret;
    }

#ifdef CONFIG_MRT_USE_MRT_GAP_EVENT_HANDLER
    ret = esp_ble_gap_register_callback(${obj.name.lower()}_gap_evt_handler);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gap register error, error code = %x", ret);
        return ret;
    }
#endif

    //Register "App", which will start the process and generate a ESP_GATTS_REG_EVT event
    ret = esp_ble_gatts_app_register(${obj.name}_profile.mPro.id);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gatts app register error, error code = %x", ret);
        return ret;
    }


    return ret;
}


void ${obj.name.lower()}_gatts_evt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    static mrt_gatt_evt_t mrt_evt;
    switch (event) {
        case ESP_GATTS_REG_EVT:

            //Set Device name
            esp_ble_gap_set_device_name(${obj.name.upper()}_DEVICE_NAME );

            //Register all of the services in our profile
            ${obj.name.lower()}_profile_register_services(gatts_if);
            break;
        case ESP_GATTS_READ_EVT:
        case ESP_GATTS_WRITE_EVT:
            mrt_evt =  mrt_gatt_handle_evt(&${obj.name}_profile.mPro,event, param);

            // if(mrt_evt.type == MRT_GATT_EVT_CCCD_WRITE)
            // {

            // }

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
            ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "ESP_GATTS_CONNECT_EVT");
            #ifdef ENABLE_SECURE_BT_PAIRING
				ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "ESP_GATTS_CONNECT_EVT: Secure Pairing Enabled");
                if(isInPairingMode) // Allow all incoming connections while in paring mode
                {
					/* start security connect with peer device when receive the connect event sent by the master */
					esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
                }
				else // If we are not in pairing mode and this device is not allowed, then disconnect
				{
					if (!check_bonded_devices(param->connect.remote_bda))
                    {
                            
                        ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "Invalid remote address."); 
                        esp_ble_gap_disconnect(param->connect.remote_bda);
                    } 
                    else
                    {
                        /* start security connect with peer device when receive the connect event sent by the master */
                        esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
                    }
				}
			#else
				ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "ESP_GATTS_CONNECT_EVT: Secure Pairing Disabled");
				/* start security connect with peer device when receive the connect event sent by the master */
				esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_MITM);
			#endif                
            break;
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "ESP_GATTS_DISCONNECT_EVT");
            /* start advertising again when missing the connect */
            esp_ble_gap_start_advertising(&data_adv_params);
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
            ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "Create attr tab : handle = %x",param->add_attr_tab.num_handle);
            if (param->create.status == ESP_GATT_OK)
            {
                /**
                 * Every time an attribute table is registered, it will trigger this event and pass back an array of 'handles'
                 * These arrays are passed to the profile to sort them out and assign them to characteristics
                 */
                mrt_gatt_set_handles(&${obj.name}_profile.mPro,&param->add_attr_tab.svc_uuid ,param->add_attr_tab.handles, param->add_attr_tab.num_handle );
            }
            else
            {
                ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , " Create attribute table failed, error code = %x", param->create.status);
            }
        break;
    }
        default:
           break;
    }
}



/*user-block-functions-start*/
/*user-block-functions-end*/

