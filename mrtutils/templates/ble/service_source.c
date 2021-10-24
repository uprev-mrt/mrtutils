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

/* UUIDS----------------------------------------------------------------------*/
const uint8_t ${"{0}_SVC_UUID[] = {{{1}}}".format(obj.name.upper(), obj.uuidArray() )};
% for char in obj.chars:
% if char.perm.lower() != 'w':
const uint8_t ${"{0}_CHR_UUID[] = {{{1}}}".format(char.name.upper(), char.uuidArray() )};
%endif
% endfor


/* Initializer-----------------------------------------------------------------*/
${obj.prefix}_svc_t* ${obj.prefix}_svc_init(mrt_gatt_pro_t* pro)
{   
    ${'mrt_gatt_init_svc(&{0}_svc.mSvc, {1}, {2}_SVC_UUID, {3}, NULL, "{4}");'.format(obj.prefix, obj.uuidType, obj.name.upper(), len(obj.chars), obj.name)}
% for char in obj.chars:
    ${'mrt_gatt_init_char(&{0}_svc.mSvc , &{0}_svc.m{1},{2},  (uint8_t*)&{3}_CHR_UUID, {4}, {5}, {0}_{6}_handler, "{7}");'.format(obj.prefix, t.camelCase(char.name), char.uuidType, char.name.upper(), char.size(), char.props(), char.name.lower(), char.name)}
%endfor

    if(pro != NULL)
    {   
        ${"mrt_gatt_add_svc(pro,&{0}_svc.mSvc);".format(obj.prefix)}
    }

    return &${obj.prefix}_svc;
    
}


/* Getters and Setters--------------------------------------------------------*/

% for char in obj.chars:
%if (char.type == 'string') or (char.arrayLen > 1):
${"{0}* {1}_get_{2}()".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
{
   if(!${"{0}_{1}_cache_valid()".format(obj.prefix,char.name.lower())})
   {
       return NULL;
   }

   return  ${"(({0}*) {1}_svc.m{2}.data.value);".format(t.cTypeDict[char.type], obj.prefix, t.camelCase(char.name))}
}

%else:
${"{0} {1}_get_{2}()".format(t.cTypeDict[char.type], obj.prefix,char.name.lower())}
{
   if(!${"{0}_{1}_cache_valid()".format(obj.prefix,char.name.lower())})
   {
       return 0;
   }

   return  ${"*(({0}*) {1}_svc.m{2}.data.value);".format(t.cTypeDict[char.type], obj.prefix, t.camelCase(char.name))}
}

%endif
% endfor


/* Weak Characteristic Event Handlers-----------------------------------------*/
% for char in obj.chars:
__attribute__((weak)) ${"mrt_status_t {0}_{1}_handler(mrt_gatt_evt_t* event)".format(obj.prefix,char.name.lower())}
{

%if (char.arrayLen > 1):
%if (char.type == 'string'):
    //char* val = (char*) event->data.value); /* Cast to correct data type*/
%else:
    //${"{0}_{1}_t* vals = ({0}_{1}_t*) event.data.value;".format(obj.prefix,char.name.lower())}  /* Cast to correct data type*/
    //${"uint32_t len = event.data.len/sizof({0}_{1}_t);".format(obj.prefix,char.name.lower())}   /* Get length of array*/
%endif
%else:
    //${"{0}_{1}_t val = *(({0}_{1}_t*) event.data.value);".format(obj.prefix,char.name.lower())} /* Cast to correct data type*/
%endif


% if char.isEnum> 0:
    //switch(val)
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
