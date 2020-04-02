/**
 * @file ${obj.prefix}_svc.h
 * @brief ${obj.desc.strip()}
 * @date ${obj.profile.genTime}
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "Utilities/Interfaces/GattServer/gatt_server.h"

/* Exported types ------------------------------------------------------------*/
typedef struct{
    mrt_gatt_svc_t mSvc;
% for char in obj.chars:
    ${t.padAfter("mrt_gatt_char_t m" + t.camelCase(char.name)+";", 45)}/* ${char.desc} */
% endfor
    /*user-block-struct-start*/
    /*user-block-struct-end*/
} ${obj.prefix}_svc_t;

/* Exported constants --------------------------------------------------------*/
extern ${obj.prefix}_svc_t ${obj.prefix}_svc;

/* Initializer----------------------------------------------------------------*/
void ${obj.prefix}_svc_init(${obj.prefix}_svc_t* ${obj.prefix}_svc);

/* Getters and Setters--------------------------------------------------------*/
% for char in obj.chars:
% if char.perm.lower() != 'w':
%if char.type == 'string':
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 45)}${"MRT_GATT_UPDATE_CHAR(&{0}_svc.m{1}, (uint8_t*)(val), strlen(val))".format(obj.prefix, t.camelCase(char.name))}
%else:
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 45)}${"MRT_GATT_UPDATE_CHAR(&{0}_svc.m{1}, (uint8_t*)(val), {2})".format(obj.prefix, t.camelCase(char.name),char.size())}
%endif
%endif
% endfor

#ifdef __cplusplus
}
#endif
