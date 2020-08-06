/**
 * @file app_${obj.prefix}_svc.c
 * @brief Application layer for ${obj.name} service
 * @date ${obj.profile.genTime}
 * 
 */

/*user-block-includes-start -------------------------------------------------*/
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
%if (char.type == 'string') or (char.arrayLen > 1):
    //${t.cTypeDict[char.type]}* val = ((${t.cTypeDict[char.type]}*) event->mData.data); /* Cast to correct data type*/
%else: 
    //${t.cTypeDict[char.type]} val = *((${t.cTypeDict[char.type]}*) event->mData.data); /* Cast to correct data type*/
%endif

% if char.isEnum> 0:
    //switch(*ptrVal)
    //{
    % for val in char.vals:
    //    case ${t.padAfter(obj.prefix +"_"+char.name + "_"+val.name+":" , 45).upper()}  /* ${val.desc} */
    //        //TODO Handle ${val.name}
    //        break;
    % endfor
    //    default: 
    //        break;
    //}

%endif
% if char.isMask> 0:
    % for val in char.vals:
    //if(val & ${t.padAfter(obj.prefix +"_"+char.name + "_"+val.name+")" , 45).upper()}  /* ${val.desc} */
    //{
    //  //TODO Handle ${val.name} 
    //}
    % endfor

%endif
    //TODO Handle ${char.name} Characteristic events
    return MRT_STATUS_OK;
}

% endfor
