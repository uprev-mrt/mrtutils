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
%if obj.profile.platform == 'mrt':
#include "Utilities/Interfaces/GattServer/gatt_server.h"
%else:
#include "../interface/mrt_gatt_interface.h"
%endif 


/* Exported Macros -----------------------------------------------------------*/
% for char in obj.chars:
    % if len(char.vals) > 0:
/* ${char.name} Values */
        % for val in char.vals:
    #define ${t.padAfter(obj.prefix +"_"+char.name + "_"+val.name , 12).upper()} ${"0x{:04x}".format(val.value)}   // ${val.desc}
        % endfor

    %endif
% endfor

/* Exported types ------------------------------------------------------------*/

% for char in obj.chars:
%if (char.type != 'string'):
%if (char.arrayLen > 1):
    ${t.padAfter("typedef {0}*".format(t.cTypeDict[char.type]), 45)}${"{0}_{1}_t;".format(obj.prefix,char.name.lower())}
%else:
    ${t.padAfter("typedef {0}".format(t.cTypeDict[char.type]), 45)}${"{0}_{1}_t;".format(obj.prefix,char.name.lower())}
%endif
%endif
% endfor

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
${obj.prefix}_svc_t* ${obj.prefix}_svc_init(mrt_gatt_pro_t* pro);

void ${obj.prefix}_svc_register();





/* Getters and Setters--------------------------------------------------------*/

/* */
% for char in obj.chars:
% if char.perm.lower() != 'w':
%if char.type == 'string':
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 45)}${"mrt_gatt_update_char_val(&{0}_svc.m{1}, (uint8_t*)(val), strlen(val))".format(obj.prefix, t.camelCase(char.name))}
%if  (char.arrayLen > 1):
${ t.padAfter("#define {0}_set_{1}(val,len)".format(obj.prefix,char.name.lower()) , 45)}${"mrt_gatt_update_char_val(&{0}_svc.m{1}, (uint8_t*)(val), len)".format(obj.prefix, t.camelCase(char.name))}
%else:
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 45)}${"mrt_gatt_update_char_val(&{0}_svc.m{1}, (uint8_t*)(&({0}_{3}_t){val}), {2})".format(obj.prefix, t.camelCase(char.name),char.size(), char.name.lower())}
%endif
%endif
%endif
% endfor

/**
 * @brief get cached data for characteristics
 */
% for char in obj.chars:
%if (char.type == 'string') or (char.arrayLen > 1):
${"{0}* {1}_get_{2}();".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
%else:
${"{0} {1}_get_{2}();".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
%endif
% endfor

/**
 * @brief check if cache is valid
 */
% for char in obj.chars:
${t.padAfter("#define {0}_{1}_cache_valid()".format(obj.prefix,char.name.lower()) , 65)}${"({0}_svc.m{1}.data.len != 0)".format(obj.prefix,  t.camelCase(char.name))}
% endfor

/* Handlers ------------------------------------------------------------------*/
void ${obj.prefix}_svc_post_init_handler();

% for char in obj.chars:
${"mrt_status_t {0}_{1}_handler(mrt_gatt_evt_t* event);".format(obj.prefix,char.name.lower())}
%endfor

#ifdef __cplusplus
}
#endif
