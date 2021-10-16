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
#include "adapter/mrt_gatt_adapter.h"

/* Exported Macros -----------------------------------------------------------*/
/* Types */
% for char in obj.chars:
    ${t.padAfter("#define {0}_{1}_t".format(obj.prefix,char.name.lower()) , 45)}${t.cTypeDict[char.type]}
% endfor

% for char in obj.chars:
    % if len(char.vals) > 0:
/* ${char.name} Values */
        % for val in char.vals:
    #define ${t.padAfter(obj.prefix +"_"+char.name + "_"+val.name , 45).upper()} ${"0x{:04x}".format(val.value)}   // ${val.desc}
        % endfor

    %endif
% endfor

/* Exported types ------------------------------------------------------------*/

enum {
    ${"IDX_{0}_SVC".format(obj.prefix.upper())},
% for char in obj.chars:
    ${"IDX_{0}_{1}_CHAR".format(obj.prefix.upper(), char.name.upper())},
    ${"IDX_{0}_{1}_VAL".format(obj.prefix.upper(), char.name.upper())},
    %if 'n' in char.perm.lower():
    ${"IDX_{0}_{1}_CCCD".format(obj.prefix.upper(), char.name.upper())},
    %endif
% endfor
    ${"IDX_{0}_NB".format(obj.prefix.upper())},
};

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
extern const esp_gatts_attr_db_t ${"{0}_svc_attr_db[IDX_{1}_NB] = ".format(obj.prefix.lower(),obj.prefix.upper())}

/* Exported Functions --------------------------------------------------------*/

void ${obj.prefix}_svc_init();

void ${obj.prefix}_svc_set_handles(uint16_t* handles, int len);



/**
 * @brief Called after server is intialized
 */
void ${obj.prefix}_svc_post_init(void);


/* Setters -------------------------------------------------------------------*/

% for char in obj.chars:
% if char.perm.lower() != 'w':
%if char.type == 'string':
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 55)}${"gatt_set_char_val(&{0}_svc.m{1}.m{1}, strlen(val), (uint8_t*) val);".format(obj.prefix, t.camelCase(char.name))}
%else:
%if char.arrayLen > 1:
${ t.padAfter("#define {0}_set_{1}(val, len)".format(obj.prefix,char.name.lower()) , 55)}${"gatt_set_char_val(&{0}_svc.m{1},len * sizeof({2}), (uint8_t*)(val))".format(obj.prefix, t.camelCase(char.name), t.cTypeDict[char.type])}
%else:
${ t.padAfter("#define {0}_set_{1}(val)".format(obj.prefix,char.name.lower()) , 55)}${"gatt_set_char_val(&{0}_svc.m{1},sizeof({2}), (uint8_t*)(val))".format(obj.prefix, t.camelCase(char.name),t.cTypeDict[char.type])}
%endif
%endif
%endif
% endfor


/* Getters -------------------------------------------------------------------*/

% for char in obj.chars:
%if (char.type == 'string') or (char.arrayLen > 1):
${"{0}* {1}_get_{2}();".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
%else:
${"{0} {1}_get_{2}();".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
%endif
% endfor


/* Characteristic Event Handlers----------------------------------------------*/
% for char in obj.chars:
${"void {0}_{1}_handler(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);".format(obj.prefix,char.name.lower())}
%endfor

#ifdef __cplusplus
}
#endif
