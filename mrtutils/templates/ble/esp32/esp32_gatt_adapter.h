/**
  *@file esp32_gatt_adapter.h
  *@brief header for 
  *@author Jason Berger
  *@date 10/16/21
  */

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include <stddef.h>
#include "mrt_gatt_interface.h"
#include "esp_log.h"
#include "esp_gatt_common_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Sets the handles for attributes
 * 
 * @param svc serviec to assign handles to
 * @param handles array of handles
 * @param len number of handles
 */
mrt_status_t mrt_gatt_set_handles(mrt_gatt_svc_t* svc,uint16_t* handles, int len );


/**
 * @brief Converts the esp gatt uuid to the mrt_gatt_uuid_t
 * @param event esp gatt event type
 * @param param esp gatt event parameters
 * @return converted mrt_gatt_evt_t
 */
mrt_gatt_uuid_t mrt_gatt_convert_uuid( esp_bt_uuid_t* esp_uuid);

/**
 * @brief Converts the esp gatt event to the mrt_gatt_evt_t so that it can be handled by the mrt_gatt_interface
 * @param event esp gatt event type
 * @param param esp gatt event parameters
 * @return converted mrt_gatt_evt_t
 */
mrt_gatt_evt_t mrt_gatt_convert_evt( esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param);

/**
 * @brief Handles the gatt event, and dispatches the correct app_svc handler
 * @param pro ptr to profile 
 * @param event esp gatt event type
 * @param param esp gatt event parameters
 * @return converted mrt_gatt_evt_t
 */
mrt_gatt_evt_t mrt_gatt_handle_evt(mrt_gatt_pro_t* pro, esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param);

/**
 * @brief Registers a service with the ble system by creating an attribute table
 * 
 * @param svc ptr to service
 * @param gatts_if  gatt server interface id
 * @return mrt_status_t 
 */
mrt_status_t mrt_gatt_register_svc(mrt_gatt_svc_t* svc,esp_gatt_if_t gatts_if);

/**
 * @brief print out a uuid for debug
 * 
 * @param esp_uuid 
 * @param mrt_uuid 
 */
void mrt_gatt_print_uuid(esp_bt_uuid_t* esp_uuid, mrt_gatt_uuid_t* mrt_uuid);

#ifdef __cplusplus
}
#endif

