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

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void ${obj.name.lower()}_profile_init(void);

/*user-block-functions-start*/
/*user-block-functions-end*/

#ifdef __cplusplus
}
#endif
