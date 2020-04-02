/**
 * @file ${obj.name}_svc.h
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

#include "Utilities/Interfaces/GattServer.h"


/*user-block-top-start*/
/*user-block-top-end*/


/*******************************************************************************
  Sruct                                                                                
*******************************************************************************/
typedef struct{
    mrt_gatt_svc_t mService;            //ptr to base register-device 
% for char in obj.chars:
    ${t.padAfter("mrt_ble_char_t m" + t.camelCase(char.name)+";", 58)/*${char.desc}*/
% endfor
    ${t.padAfter("mrt_ble_char_t* mChars;",58)/*Pointer to Chars to make service iteratable*/
/*user-block-struct-start*/
/*user-block-struct-end*/
}${obj.name.lower()}_svc_t;


${obj.name}_svc_init(${obj.name.lower()}_svc_t);

#ifdef __cplusplus
}
#endif