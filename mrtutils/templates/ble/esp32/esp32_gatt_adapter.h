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
#include "../mrt_gatt_interface.h"

/* Exported types ------------------------------------------------------------*/

typedef struct {
  esp_gatt_if_t gatts_if;
}esp32_adapter_ctx_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Sets gatt_if for profile so we can reference it
 * @param pro ptr to profile
 * @param gatts_if gatt interface
 */
void mrt_gatt_set_interface(mrt_gatt_pro_t* pro, esp_gatt_if_t gatts_if);

/**
 * @brief Sets the handles for attributes
 * 
 * @param pro ptr to profile
 * @param svc_uuid serviec uuid from ESP_GATTS_CREAT_ATTR_TAB_EVT event
 * @param handles array of handles
 * @param len number of handles
 */
mrt_status_t mrt_gatt_set_handles(mrt_gatt_pro_t* pro, esp_bt_uuid_t* svc_uuid,uint16_t* handles, int len );

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

#ifdef __cplusplus
}
#endif



