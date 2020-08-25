/**
 * @file ${obj.name.lower()}_profile.h
 * @brief ${obj.desc}
 * @date ${obj.genTime}
 * 
 */

#include "${obj.name.lower()}_profile.h"

/*user-block-top-start*/
/*user-block-top-end*/


void ${obj.name.lower()}_profile_init(void)
{
    /* Initialize all services */
    %for svc in obj.services:
    ${svc.prefix}_svc_init(&${svc.prefix}_svc);
    %endfor

    /* Call post initialization once all services are up */
    %for svc in obj.services:
    ${svc.prefix}_svc_post_init_handler();
    %endfor
}

/*user-block-functions-start*/
/*user-block-functions-end*/

