/**
 * @file ${obj.prefix}_svc.c
 * @brief ${obj.desc.strip()}
 * @date ${obj.profile.genTime}
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "${obj.prefix}_svc.h"

/* Service Constant-----------------------------------------------------------*/
MRT_GATT_DATA_ATTR ${obj.prefix}_svc_t ${obj.prefix}_svc;

/* Macros --------------------------------------------------------------------*/

/* UUIDS ---------------------------------------------------------------------*/
const uint8_t ${"{0}_SVC_UUID[] = {{{1}}}".format(obj.name.upper(), obj.uuidArray() )};
% for char in obj.chars:
% if char.perm.lower() != 'w':
const uint8_t ${"{0}_CHR_UUID[] = {{{1}}}".format(char.name.upper(), char.uuidArray() )};
%endif
% endfor

/* Attr Table -----------------------------------------------------------------*/

static uint16_t ${"{0}_svc_handle_table[IDX_{1}_NB] = ".format(obj.prefix.lower(),obj.prefix.upper())}

const esp_gatts_attr_db_t ${"{0}_svc_attr_db[IDX_{1}_NB] = ".format(obj.prefix.lower(),obj.prefix.upper())}
{
    /* Service Declaration  */
    [${"IDX_{0}_SVC".format(obj.prefix.upper())}]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(${"{0}_SVC_UUID".format(obj.name.upper())}), (uint8_t *)&${"{0}_SVC_UUID".format(obj.name.upper())}}},
% for char in obj.chars:

    /* ${char.name} Declaration */
    [${"IDX_{0}_{1}_CHAR".format(obj.prefix.upper(), char.name.upper())}]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* ${char.name} Value */
    [${"IDX_{0}_{1}_VAL".format(obj.prefix.upper(), char.name.upper())}] =
    {{ESP_GATT_AUTO_RSP}, {${obj.uuidType.replace("bit","").replace("e","ESP_UUID_LEN_")}, (uint8_t *)&GATTS_CHAR_UUID_TEST_A, ${char.props().replace("MRT_GATT_PROP_", "ESP_GATT_PERM_")},
       ${"{0}_svc->m{1}.mSize, {0}_svc->m{1}.mCache.mLen,{0}_svc->m{1}.mCache.mData".format(obj.prefix, t.camelCase(char.name))}  }},

    %if 'n' in char.perm.lower():
    /* ${char.name} CCCD */
    [${"IDX_{0}_{1}_CCCD".format(obj.prefix.upper(), char.name.upper())}]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},
    %endif
% endfor
};


/* Functions -----------------------------------------------------------------*/
void ${obj.prefix}_svc_init()
{   
    ${"gatt_init_svc(&{0}_svc->mSvc, {1}, {2}_SVC_UUID, {3}, NULL);".format(obj.prefix, obj.uuidType, obj.name.upper(), len(obj.chars))}
% for char in obj.chars:
    ${"gatt_init_char(&{0}_svc->mSvc , &{0}_svc->m{1},{2},  (uint8_t*)&{3}_CHR_UUID, {4}, {5}, {0}_{6}_handler);".format(obj.prefix, t.camelCase(char.name), char.uuidType, char.name.upper(), char.size(), char.props(), char.name.lower())}
%endfor
    
}

void ${obj.prefix}_svc_set_handles(uint16_t* handles, int len)
{
    int cur = 0;

    ${obj.prefix}_svc.mHandle = handles[cur++];

    for(uint32_t i = 0; i < ${obj.prefix}_svc.mCharCount; i++)
    {

    }
}


/* Getters -------------------------------------------------------------------*/

% for char in obj.chars:
%if (char.type == 'string') or (char.arrayLen > 1):
${"{0}* {1}_get_{2}()".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
{
   if(!${"{0}_{1}_cache_valid()".format(obj.prefix,char.name.lower())})
   {
       return NULL;
   }

   return  ${"(({0}*) {1}_svc.m{2}.mCache.mData);".format(t.cTypeDict[char.type], obj.prefix, t.camelCase(char.name))}
}

%else:
${"{0} {1}_get_{2}()".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
{
   if(!${"{0}_{1}_cache_valid()".format(obj.prefix,char.name.lower())})
   {
       return 0;
   }

   return  ${"*(({0}*) {1}_svc.m{2}.mCache.mData);".format(t.cTypeDict[char.type], obj.prefix, t.camelCase(char.name))}
}

%endif
% endfor


/* Initialization Handler----------------------------------------------*/
__attribute__((weak)) void ${obj.prefix}_svc_post_init_handler(void)
{

}

/* Characteristic Event Handlers----------------------------------------------*/
% for char in obj.chars:
__attribute__((weak)) ${"mrt_status_t {0}_{1}_handler(mrt_gatt_evt_t* event)".format(obj.prefix,char.name.lower())}
{
    //${t.cTypeDict[char.type]} val = *((${t.cTypeDict[char.type]}*) event->mData.data); /* Cast to correct data type*/

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
