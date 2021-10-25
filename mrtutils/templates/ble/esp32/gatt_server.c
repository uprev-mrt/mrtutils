/**
 * @file ${obj.name.lower()}_gatt_server.c
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#include "${obj.name.lower()}_gatt_server.h"



/* Macros --------------------------------------------------------------------*/


#define ADV_CONFIG_FLAG             (1 << 0)
#define SCAN_RSP_CONFIG_FLAG        (1 << 1)

/*user-block-top-start*/
#define ${obj.name.upper()}_PROFILE_ID 0
#define ${obj.name.upper()}_PROFILE_TAG          "${obj.name.upper()}_PROFILE"
#define ${obj.name.upper()}_DEVICE_NAME          "${obj.name.upper()}_DEVICE"


/* Private Variables ---------------------------------------------------------*/
MRT_GATT_DATA_ATTR ${obj.name.lower()}_profile_t ${obj.name.lower()}_profile;
mrt_profile_ctx_t ${obj.name.lower()}_profile_ctx; 

static uint8_t adv_config_done       = 0;

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval        = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance          = 0x00,
    .manufacturer_len    = 0,    //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //test_manufacturer,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 2,
    .p_service_uuid      = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp        = true,
    .include_name        = true,
    .include_txpower     = true,
    .min_interval        = 0x0006,
    .max_interval        = 0x0010,
    .appearance          = 0x00,
    .manufacturer_len    = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = 2,
    .p_service_uuid      = NULL,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t data_adv_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min         = 0x20,
    .adv_int_max         = 0x40,
    .adv_type            = ADV_TYPE_IND,
    .own_addr_type       = BLE_ADDR_TYPE_PUBLIC,
    .channel_map         = ADV_CHNL_ALL,
    .adv_filter_policy   = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/*user-block-top-end*/


/* Exported Functions --------------------------------------------------------*/

mrt_status_t ${obj.name.lower()}_profile_init(void)
{
    //initialize profile
    mrt_gatt_init_pro(&${obj.name.lower()}_profile.mPro, ${len(obj.services)}, 0, "${obj.name}");
    ${obj.name.lower()}_profile.mPro.id = ${obj.name.upper()}_PROFILE_ID;

    //Assign it a context to manage interface and connection ID
    ${obj.name.lower()}_profile.mPro.ctx = (void*) &${obj.name.lower()}_profile_ctx; 
    
    /* Initialize all services */
    %for svc in obj.services:
    ${"{0}_profile.m{1} = {2}_svc_init(&{0}_profile.mPro);".format(obj.name.lower(),t.camelCase(svc.name), svc.prefix)}
    %endfor


    /* Call Post init handlers - These must be called or ESPs build system will not properly override the event handlers- */
    %for svc in obj.services:
    ${"{0}_svc_post_init_handler();".format(svc.prefix)}
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

    //Initialize profile
    ${obj.name.lower()}_profile_init();
    

    adv_data.service_uuid_len = 16;
    scan_rsp_data.service_uuid_len = 16;
    adv_data.p_service_uuid = ${obj.name.lower()}_profile.mPro.svcs[1]->uuid.val;
    scan_rsp_data.p_service_uuid = ${obj.name.lower()}_profile.mPro.svcs[1]->uuid.val;


    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return ret;
    }

    //Register event handler
    ret = esp_ble_gatts_register_callback(${obj.name.lower()}_gatts_evt_handler);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gatts register error, error code = %x", ret);
        return ret;
    }

    ret = esp_ble_gap_register_callback(${obj.name.lower()}_gap_evt_handler);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gap register error, error code = %x", ret);
        return ret;
    }


    //Register "App", which will start the process and generate a ESP_GATTS_REG_EVT event
    ret = esp_ble_gatts_app_register(${obj.name.lower()}_profile.mPro.id);
    if (ret){
        ESP_LOGE(${obj.name.upper()}_PROFILE_TAG , "gatts app register error, error code = %x", ret);
        return ret;
    }


    return ret;
}




void ${obj.name.lower()}_gatts_evt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    static mrt_gatt_evt_t mrt_evt;
    static mrt_gatt_uuid_t mrt_uuid;
    static mrt_gatt_svc_t* svc;

    static esp_err_t ret; 

    switch (event) {
        case ESP_GATTS_REG_EVT:

            //Set Device name
            esp_ble_gap_set_device_name(${obj.name.upper()}_DEVICE_NAME );

            //Set adv data
            ret = esp_ble_gap_config_adv_data(&adv_data);
            if (ret){
                ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "config adv data failed, error code = %x", ret);
            }
            adv_config_done |= ADV_CONFIG_FLAG;
            //config scan response data
            ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
            if (ret){
                ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "config scan response data failed, error code = %x", ret);
            }
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;

            //Set interface for profile
            ${obj.name.lower()}_profile_ctx.gatts_if = gatts_if;

            //Register all of the services in our profile
            ${obj.name.lower()}_profile_register_services(gatts_if);
            break;
        case ESP_GATTS_READ_EVT:
        case ESP_GATTS_WRITE_EVT:
            mrt_evt =  mrt_gatt_handle_evt(&${obj.name.lower()}_profile.mPro,event, param);

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

            //Set connection id for context 
            ${obj.name.lower()}_profile_ctx.conn_id = param->connect.conn_id;
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
            /**
             * Every time an attribute table is registered, it will trigger this event and pass back an array of 'handles'
             * These arrays are passed to the profile to sort them out and assign them to characteristics
             */
            if (param->create.status == ESP_GATT_OK)
            {   
                //Convert UUID to mrt format, and lookup service
                mrt_uuid = mrt_gatt_convert_uuid(&param->add_attr_tab.svc_uuid );
                svc = mrt_gatt_lookup_svc_uuid(&${obj.name.lower()}_profile.mPro, &mrt_uuid );

                ESP_LOGI(${obj.name.upper()}_PROFILE_TAG , "Create attr table for %s, Handle: %x", svc->name, param->add_attr_tab.num_handle);
                
                //Set handles for attribtues
                mrt_gatt_set_handles(svc ,param->add_attr_tab.handles, param->add_attr_tab.num_handle );

                //Start Service
                esp_ble_gatts_start_service(svc->handle);

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

void ${obj.name.lower()}_gap_evt_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{

    ESP_LOGE(EASYRIDER_PROFILE_TAG, "GAP event = %x\n",event);
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(${obj.name.upper()}_PROFILE_TAG, "ADV Data Set!");
            adv_config_done &= (~ADV_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
            if (adv_config_done == 0){
                esp_ble_gap_start_advertising(&adv_params);
            }
            break;
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising start successfully or failed */
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "advertising start failed");
            }else{
                ESP_LOGI(${obj.name.upper()}_PROFILE_TAG, "advertising start successfully");
            }
            break;
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(${obj.name.upper()}_PROFILE_TAG, "Advertising stop failed");
            }
            else {
                ESP_LOGI(${obj.name.upper()}_PROFILE_TAG, "Stop adv successfully\n");
            }
            break;
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(${obj.name.upper()}_PROFILE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
            break;
        default:
            break;
    }
}


void task_${obj.name.lower()}_gatts( void * pvParameters )
{
  ${obj.name.lower()}_gatts_start();

  for(;;)
  {
    
  }
}



/*user-block-functions-start*/
/*user-block-functions-end*/

