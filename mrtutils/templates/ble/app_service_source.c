/**
 * @file app_${obj.prefix}_svc.c
 * @brief Application layer for ${obj.name} service
 * @date ${obj.profile.genTime}
 * 
 */

/* user-block-includes-start -------------------------------------------------*/
#include "Utilities/Interfaces/GattServer/gatt_server.h"
#include "svc/${obj.prefix}_svc.h"

/*user-block-includes-end*/


/* Post Init -----------------------------------------------------------------*/

/**
 * @brief Called after GATT Server is intialized
 */
void ${obj.prefix}_svc_post_init_handler(void)
{
    //TODO initialize characteristic values 
}

/* Characteristic Event Handlers----------------------------------------------*/

% for char in obj.chars:
/**
 * @brief Handles GATT event on ${char.name} Characteristic
 * @param event - ptr to mrt_gatt_evt_t event with data and event type
 */
${"mrt_status_t {0}_{1}_handler(mrt_gatt_evt_t* event)".format(obj.prefix,char.name.lower())}
{
    //TODO Handle ${char.name} Characteristic events
    return MRT_STATUS_OK;
}

% endfor
