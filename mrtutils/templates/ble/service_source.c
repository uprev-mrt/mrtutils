/**
 * @file ${obj.prefix}_svc.c
 * @brief ${obj.desc.strip()}
 * @date ${obj.profile.genTime}
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "${obj.prefix}_svc.h"

/* user-block-includes-start--------------------------------------------------*/

/* user-block-includes-end */


/* Service Constant-----------------------------------------------------------*/
${obj.prefix}_svc_t ${obj.prefix}_svc;

/* UUIDS----------------------------------------------------------------------*/
const uint8_t ${"{0}_SVC_UUID[] = {{{1}}}".format(obj.name.upper(), obj.uuidArray() )};
% for char in obj.chars:
% if char.perm.lower() != 'w':
const uint8_t ${"{0}_CHR_UUID[] = {{{1}}}".format(char.name.upper(), char.uuidArray() )};
%endif
% endfor

/* Characteristic Event Handlers----------------------------------------------*/
% for char in obj.chars:
${"mrt_status_t {0}_{1}_handler(mrt_gatt_evt_t* event)".format(obj.prefix,char.name.lower())}
{

}

% endfor

/* Service Event Handler------------------------------------------------------*/
${"mrt_status_t {0}_svc_handler(mrt_gatt_evt_t* event)".format(obj.prefix)}
{

}

/* Initializer-----------------------------------------------------------------*/
void ${obj.prefix}_svc_init(${obj.prefix}_svc_t* ${obj.prefix}_svc)
{   
    ${"gatt_init_svc(&{0}_svc->mSvc, {1}, {2}_SVC_UUID, {3}, {0}_svc_handler);".format(obj.prefix, obj.uuidType, obj.name.upper(), len(obj.chars))}
% for char in obj.chars:
    ${"gatt_init_char(&{0}_svc->mSvc , &{0}_svc->m{1},{2},  (uint8_t*)&{3}_CHR_UUID, {4}, {5}, {0}_{6}_handler);".format(obj.prefix, t.camelCase(char.name), char.uuidType, char.name.upper(), char.size(), char.props(), char.name.lower())}
%endfor
    
    MRT_GATT_REGISTER_SERVICE(&${obj.prefix}_svc->mSvc);

}
