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


#ifdef __cplusplus
}
#endif



