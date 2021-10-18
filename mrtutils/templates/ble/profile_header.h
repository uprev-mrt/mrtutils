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
%for svc in obj.services:
#include "svc/${svc.prefix}_svc.h"
%endfor

/*user-block-top-start*/
/*user-block-top-end*/
/* Exported Macros ------------------------------------------------------------*/

/* Exported types ------------------------------------------------------------*/

typedef struct{
    mrt_gatt_pro_t mPro;
%for svc in obj.services:
    ${t.padAfter("{0}_svc_t* m{1};".format(svc.prefix,t.camelCase(svc.name) ), 45)}/* ${svc.desc} */
%endfor
}${obj.name.lower()}_profile_t;

/* Exported constants --------------------------------------------------------*/
extern ${obj.name.lower()}_profile_t ${obj.name}_profile;
/* Exported functions ------------------------------------------------------- */
mrt_status_t ${obj.name.lower()}_profile_init(void);

mrt_status_t ${obj.name.lower()}_profile_register(void);

/*user-block-functions-start*/
/*user-block-functions-end*/

#ifdef __cplusplus
}
#endif
