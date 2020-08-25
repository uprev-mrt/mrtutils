/**
  * @file ${obj.name.lower()}_slave.h
  * @author generated by mrt-device utility 
  * @link [https://github.com/uprev-mrt/mrtutils/wiki/mrt-device]
  * @brief Device driver for ${obj.name} device
  *
  *
  */

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "${obj.name.lower()}_regs.h"
/*user-block-top-start*/
/*user-block-top-end*/

#define SLAVE_REG_PERM_R    0x01
#define SLAVE_REG_PERM_W    0x02
#define SLAVE_REG_PERM_X    0x04
#define SLAVE_REG_PERM_RW   0x03

#define SLAVE_REG_ACESS_R    0x80
#define SLAVE_REG_ACESS_W    0x40

#define SLAVE_STATE_ADDRESS  0x00
#define SLAVE_STATE_DATA     0x01

/*******************************************************************************
  Struct                                                                                
*******************************************************************************/

typedef uint8_t addr_t;



#pragma pack(push)
#pragma pack(1)

typedef struct{
    uint8_t mFlags;
    uint8_t mSize;
    addr_t mAddr;
} slave_reg_t;




typedef struct{
    struct
    {
% for key,reg in obj.regs.items():
        ${t.padAfter(t.cTypeDict[reg.type] + " m" + obj.camelCase(reg.name)+";" + (" "*(20 - len(obj.camelCase(reg.name))) ), 45)} //${reg.desc}
% endfor
    } mData;    
    slave_reg_t mRegs[${obj.name.upper()}_REG_COUNT];
    slave_reg_t* mCurrentReg;
    uint8_t mCursor;        //Register cursor
    uint8_t mAddrBytes;     //Number of address bytes received so far
    addr_t mAddress;        //Current address
    uint8_t mState;         //state of register server
    uint8_t mFlags;         //flags for handling
/*user-block-struct-start*/
/*user-block-struct-end*/
}${obj.name.lower()}_slave_t;

#pragma pack(pop)

/**
 * @brief initializes ${obj.name} slave registers
 * @param fifoDepth size of rxFifo to use
 */
void ${obj.prefix.lower()}_slave_init();

/**
 * @brief initializes ${obj.name} slave registers
 * @param dev ptr to ${obj.name} device
 */
void ${obj.prefix.lower()}_slave_process();

/**
 * @brief feed byte into device
 * @param data byte to feed 
 */
void ${obj.prefix.lower()}_slave_put( uint8_t data );

/**
 * @brief gets next byte
 * @param data byte to feed 
 */
uint8_t ${obj.prefix.lower()}_slave_get(void);

/**
 * @brief ends transaction and resets state
 * 
 */
void ${obj.prefix.lower()}_slave_end_transaction(void);




/*user-block-bottom-start*/
/*user-block-bottom-end*/

#ifdef __cplusplus
}
#endif
