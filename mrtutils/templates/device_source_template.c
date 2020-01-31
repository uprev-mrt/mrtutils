/**
  * @file ${obj.name.lower()}.c
  * @author generated by mrt-device.py
  * @brief register defintions for ${obj.name} device
  * 
  */

#include "${obj.name.lower()}.h"



/**
 * @brief initializes device
 * @param dev ptr to ${obj.name} device
 * @return status 
 */
static mrt_status_t ${obj.prefix.lower()}_init(${obj.name.lower()}_t* dev)
{   
    //Initialize Register Descriptors
% for key,reg in obj.regs.items():
    REG_INIT( dev->${"m" + obj.camelCase(reg.name)} , ${obj.prefix.upper() +"_REG_"+reg.name.upper()+"_ADDR"} , ${reg.type}, REG_PERM_${reg.perm}  );
% endfor

    /*user-block-init-start*/
    /*user-block-init-end*/

    return MRT_STATUS_OK;
}


% if "I2C" in obj.bus.upper():
mrt_status_t ${obj.prefix.lower()}_init_i2c(${obj.name.lower()}_t* dev, mrt_i2c_handle_t i2c)
{
    mrt_status_t status;

    status = init_i2c_register_device(&dev->mRegDev, i2c, ${obj.name.upper()}_I2C_ADDRESS, ${obj.name.upper()}_REG_ADDR_SIZE );

    ${obj.prefix.lower()}_init(dev);


    /*user-block-init-i2c-start*/
    /*user-block-init-i2c-end*/
    
    return MRT_STATUS_OK;
}
% endif
% if "SPI" in obj.bus.upper():
mrt_status_t ${obj.prefix.lower()}_init_spi(${obj.name.lower()}_t* dev, mrt_spi_handle_t spi, mrt_gpio_t chipSelect)
{
    mrt_status_t status;

    status = init_spi_register_device(&dev->mRegDev, spi, chipSelect, ${obj.name.upper()}_REG_ADDR_SIZE );

    ${obj.prefix.lower()}_init(dev);

    /*user-block-spi-start*/
    /*user-block-spi-end*/

    return MRT_STATUS_OK;
}
% endif

mrt_status_t ${obj.prefix.lower()}_test(${obj.name.lower()}_t* dev)
{
    /*user-block-test-start*/

    /*user-block-test-end*/
    return MRT_STATUS_ERROR;
}


/*user-block-bottom-start*/
/*user-block-bottom-end*/
