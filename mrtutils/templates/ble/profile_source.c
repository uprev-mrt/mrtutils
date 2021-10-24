/**
 * @file ${obj.name.lower()}_profile.h
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "${obj.name.lower()}_profile.h"


/*user-block-top-start*/
/*user-block-top-end*/

/* Private Variables ---------------------------------------------------------*/
MRT_GATT_DATA_ATTR ${obj.name.lower()}_profile_t ${obj.name}_profile;


/* Functions -----------------------------------------------------------------*/

mrt_status_t ${obj.name.lower()}_profile_init(void)
{
    mrt_gatt_init_pro(&${obj.name}_profile.mPro, ${len(obj.services)}, 0);
    
    /* Initialize all services */
    %for svc in obj.services:
    ${"{0}_profile.m{1} = {2}_svc_init(&{0}.mPro);".format(obj.name.lower(),t.camelCase(svc.name), svc.prefix)}
    %endfor
    
    return MRT_STATUS_OK;
}

mrt_status_t ${obj.name.lower()}_profile_register(void)
{
    mrt_status_t status;
    for(uint16_t i =0; i < ${obj.name}_profile.mSvcCount; i++ )
    {
        status = mrt_gatt_register_svc(${obj.name}_profile.mPro.mSvcs[i]);
        if(status != MRT_STATUS_OK)
        {
            break;
        }

    }

    return status;
}



/*user-block-functions-start*/
/*user-block-functions-end*/

