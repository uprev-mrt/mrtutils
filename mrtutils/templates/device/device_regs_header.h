/**
  * @file ${obj.name.lower()}_regs.h
  * @author generated by mrt-device utility 
  * @link [https://github.com/uprev-mrt/mrtutils/wiki/mrt-device]
  * @brief values and constants for the ${obj.name} device registers
  * 
  */

% if "I2C" in obj.bus.upper():
#define ${obj.name.upper()}_I2C_ADDRESS ${ obj.formatHex(obj.i2c_addr, 1)}
% endif
#define ${obj.name.upper()}_REG_ADDR_SIZE ${ obj.addrSize}
#define ${obj.name.upper()}_REG_COUNT ${len(obj.regs)}


/*******************************************************************************
  Register Addresses                                                                              
*******************************************************************************/

% for key,reg in obj.regs.items():
#define ${reg.getAddrMacro(58)} ${reg.printAddr()} /* ${reg.desc} */
% endfor


/*******************************************************************************
  Fields                                                                              
*******************************************************************************/

% for key,reg in obj.regs.items():
% if len(reg.fields) > 0 :
/* ${reg.name} Register Fields */
    %if reg.hasFlags:
    /* ${reg.name} -> Flags */
    %endif
    %for field in reg.fields:
    %if field.isFlag:
    #define ${field.getFieldFlagMacro(54)} ${reg.formatHex(field.mask)} /* ${field.desc} */
    %endif
    %endfor
    %for field in reg.fields:
    %if not field.isFlag:
    /* ${reg.name} -> ${field.name} */
    #define ${field.getFieldMaskMacro(54)} ${reg.formatHex(field.mask)} /* ${field.desc} */
    #define ${field.getFieldOffsetkMacro(54)} ${reg.formatHex(field.offset)}
    %for val in field.vals:
      #define ${val.getFieldValMacro(52)} ${reg.formatHex(val.val)} /* ${val.desc} */
    %endfor
    %endif
    %endfor

% endif
% endfor

/*******************************************************************************
  Default Values                                                                              
*******************************************************************************/

% for key,reg in obj.regs.items():
% if reg.hasDefault:
#define ${reg.getDefaultMacro(58)} ${reg.formatHex(reg.default)}
% endif
% endfor

/*******************************************************************************
  Flag Set/Clear/Check                                                                             
*******************************************************************************/

/**
  *@brief sets flags on device
  *@param dev ptr to ${obj.name} device
  *@param reg ptr to register definition
  *@param mask mask of flags to set
  */
#define ${obj.prefix.lower() +"_set_flag"}(dev, reg, mask) regdev_set_flags(&(dev)->mRegDev,(reg), (mask))

/**
  *@brief clears flags on device
  *@param dev ptr to ${obj.name} device
  *@param reg ptr to register definition
  *@param mask mask of flags to set
  */
#define ${obj.prefix.lower() +"_clear_flag"}(dev, reg, mask) regdev_clear_flags(&(dev)->mRegDev,(reg), (mask))

/**
  *@brief checks flags on device
  *@param dev ptr to ${obj.name} device
  *@param reg ptr to register definition
  *@param mask mask of flags to set
  *@return true if all flags in mask are set 
  *@return false if any flags in mask are not set
  */
#define ${obj.prefix.lower() +"_check_flag"}(dev, reg, mask) regdev_check_flags(&(dev)->mRegDev,(reg), (mask))

/*******************************************************************************
  Field Getters                                                                              
*******************************************************************************/

% for key,reg in obj.regs.items():
% if "R" in reg.perm.upper():
% for field in reg.fields:
% if not field.isFlag:
/**
 * @brief reads the ${field.name} field from the ${reg.name} register 
 * @param dev ptr to ${obj.name} device
%for val in field.vals:
 * @return ${obj.prefix.upper() +"_"+reg.name.upper()+"_"+field.name.upper() +"_" + val.name.upper()} ${val.desc}
%endfor
 */
% if field.flat:
#define ${obj.prefix.lower() +"_get_"+field.name.lower()}(dev) regdev_read_field(&(dev)->mRegDev, &(dev)->${"m" + obj.camelCase(reg.name)}, ${field.getFieldMaskMacro()} )
%else:
#define ${obj.prefix.lower() +"_get_"+ reg.name.lower()+"_"+field.name.lower()}(dev) regdev_read_field(&(dev)->mRegDev, &(dev)->${"m" + obj.camelCase(reg.name)}, ${field.getFieldMaskMacro()} )
%endif
%endif
%endfor
%endif
% endfor


/*******************************************************************************
  Field Setters                                                                              
*******************************************************************************/

% for key,reg in obj.regs.items():
% if "W" in reg.perm.upper():
%for field in reg.fields:
% if not field.isFlag:
/**
 * @brief writes the ${field.name} field to the ${reg.name} register 
 * @param dev ptr to ${obj.name} device
%for val in field.vals:
 * @option ${obj.prefix.upper() +"_"+reg.name.upper()+"_"+field.name.upper() +"_" + val.name.upper()} ${val.desc}
%endfor
 */
% if field.flat:
#define ${obj.prefix.lower() +"_set_"+ field.name.lower()}(dev, val) regdev_write_field(&(dev)->mRegDev, &(dev)->${"m" + obj.camelCase(reg.name)}, ${field.getFieldMaskMacro()} , (val) )
%else:
#define ${obj.prefix.lower() +"_set_"+ reg.name.lower()+"_"+field.name.lower()}(dev, val) regdev_write_field(&(dev)->mRegDev, &(dev)->${"m" + obj.camelCase(reg.name)}, ${field.getFieldMaskMacro()} , (val) )
%endif
%endif
%endfor
%endif
% endfor

/*******************************************************************************
  Configs                                                                            
*******************************************************************************/

%for key,config in obj.configs.items():
/**
 * @brief ${config.desc.replace("\n","\n *        ")}
 * @param dev ptr to ${obj.name} device
 */
#define ${obj.prefix.upper()+"_LOAD_CONFIG_"+config.name.upper()}(dev) ${"\\"}
%for regVal in config.regVals:
${obj.getConfigLine(regVal, 48, True)} ${config.getDesc(regVal,32)} ${"\\"}
%if config.getDelay(regVal):
${config.getDelay(regVal,48)}
%endif
%endfor

%endfor
