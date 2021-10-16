/**
 * @file ${obj.name.lower()}_svc.h
 * @brief ${obj.desc.strip()}
 * @date ${obj.genTime}
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "esp_gap_ble_api.h"

%for svc in obj.services:
#include "svc/${svc.prefix}_svc.h"
%endfor

/*user-block-top-start*/

#define ${obj.name.upper()}_APP_ID 0
/*user-block-top-end*/

/* Exported constants --------------------------------------------------------*/

#define ${obj.name.upper()}_SERVICE_COUNT ${len(obj.services)}

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


void ${obj.name.lower()}_profile_init(void);
void ${obj.name.lower()}_profile_create_services(esp_gatt_if_t gatts_if);
void ${obj.name.lower()}_profile_set_handles(esp_bt_uuid_t* svc_uuid,uint16_t* handles, int len );

void ${obj.name.lower()}_gatt_write_handler(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void ${obj.name.lower()}_gatt_read_handler(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
void ${obj.name.lower()}_gatts_evt_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,esp_ble_gatts_cb_param_t *param);

/*user-block-functions-start*/
/*user-block-functions-end*/

#ifdef __cplusplus
}
#endif
