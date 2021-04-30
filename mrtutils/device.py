#!/usr/bin/env python3
#
#@file device.py
#@brief device descripto
#@author Jason Berger
#@date 02/19/2019
#

import sys
import os
import yaml
import json
from mrtutils.mrtYamlHelper import *
from mrtutils.mrtTemplateHelper import *


sizeDict = {
    "uint8" : 1,
    "int8" : 1,
    "char" : 1,
    "uint16" : 2,
    "int16" : 2,
    "uint32" : 4,
    "int32" : 4,
    "int64" : 8,
    "uint64" : 8,
    "int" : 4,
    "string" : 16,
}

cNameDict = {
    "uint8" : "uint8_t",
     "int8" : "int8_t",
     "char" : "char",
     "string" : "char",
     "uint16" : "uint16_t",
     "int16" : "int16_t",
     "uint32" : "uint32_t",
     "int32" : "int32_t",
     "int64" : "int64_t",
     "uint64" : "uint64_t",
     "int" : "int",
     "float" : "float",
     "double" : "double",
     "enum" : "uint8_t",
     "flag" : "uint8_t"
 }

args = None
parser = None

class DevConfig:
    def __init__(self, node):
        self.field = 0
        self.val = 0 
        self.desc = ""
        self.regVals = []
        self.device = 0

        self.name = list(node.keys())[0]
        configItem = list(node.values())[0]
        if 'desc' in configItem:
            self.desc = configItem['desc']
        
        if 'registers' in configItem:
            for regNode in configItem['registers']:
                self.regVals.append(regNode)
        if 'regs' in configItem:
            for regNode in configItem['regs']:
                self.regVals.append(regNode)
    
    def getDesc(self,regVal, spacing =0):
        ret =""
        regName = list(regVal.keys())[0]
        regItem = list(regVal.values())[0]
        if type(regItem) is dict:
            if 'desc' in regItem:
                return "/* "+regItem['desc']+" */"
            else:
                ret+="/*"
                for key,value in regItem.items():
                    if not key == 'delay':
                        ret+= " "+key+": "+ str(value) +" ,"
                ret+=",*/"
                ret = ret.replace(",,", "")
        else:
            ret=""

        spaces = spacing - len(ret)
        ret+= (" " * spaces)
        
        return ret
    
    def getDelay(self, regVal, spacing = 0):
        regName = list(regVal.keys())[0]
        regItem = list(regVal.values())[0]
        ret =""
        spaces = 0
        if type( regItem) is dict:
            if 'delay' in regItem:
                ret+= "MRT_DELAY_MS({}); ".format(regItem['delay'])
                spaces = spacing - len(ret)
                ret+= (" " * spaces)
                ret+= " /* Delay for {} */ \\".format(regName)
                return ret

        return False

    
    def getDict(self):
        
        json_dict = {}
        for configReg in self.regVals:
            regName = list(configReg.keys())[0]
            regItem = list(configReg.values())[0]
            val = 0
            if regName in self.device.regs:
                curReg = self.device.regs[regName]
                if type( regItem) is dict:
                    val =0
                    for key,value in regItem.items():
                        if key in curReg.fieldDict:
                            curField = curReg.fieldDict[key]
                            if type(value) is str:
                                if value in curField.valDict:
                                    val = val | (curField.valDict[value].val << curField.offset) & curField.mask
                            else:
                                val = val | (value << curField.offset) & curField.mask
                    
                else: 
                    val = regItem

            json_dict[regName] = val 

        return json_dict    



class FieldVal:
    def __init__(self, node):
        self.field = 0
        self.val = 0 
        self.desc = ""

        self.name = list(node.keys())[0]
        valItem = list(node.values())[0]
        if 'val' in valItem:
            self.val = valItem['val']
        if 'desc' in valItem:
            self.desc = valItem['desc']
        if 'name' in valItem:
            self.desc = valItem['name']

    def getFieldValMacro(self, spacing = 0):
        ret = self.field.reg.device.prefix.upper() +"_"+self.field.reg.name.upper()+"_"+self.field.name.upper() +"_" + self.name.upper()
        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret
    
    def getFieldValMaskMacro(self, spacing = 0):
        ret = self.field.reg.device.prefix.upper() +"_"+self.field.reg.name.upper()+"_"+self.field.name.upper() +"_" + self.name.upper() + "_MASK"
        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret
    
    def getOffSetValue(self):
        return val << self.field.offset
    
    def getDict(self):
        json_dict ={"name": self.name, "val": self.val, "desc": self.desc}
        return json_dict
    
    def formatVal(self):
        if self.field.bitCount > 6 :
            return "x"+format(self.val, '02x')
        else:
            return "b"+format(self.val, '0'+str(self.field.bitCount)+'b')

 
class RegField:
    def __init__(self, node):
        self.reg = 0
        self.mask = 0XFFFFFFFF
        self.desc = ""
        self.vals = []
        self.valDict = {}
        self.isFlag = False
        self.bitCount = 0
        self.startBit =0
        self.flat = False

        if 'name' in node:
            self.name = node['name']
        if 'mask' in node:
            self.mask = node['mask']
        if 'bit' in node:
            self.mask = 1 << node['bit']
        if 'values' in node:
            for valNode in node['values']:
                newVal = FieldVal(valNode)
                self.addVal(newVal)
        if 'vals' in node:
            for valNode in node['vals']:
                newVal = FieldVal(valNode)
                self.addVal(newVal)
        if 'desc' in node:
            self.desc = node['desc']
        
        if self.getSize() == 1:
            self.isFlag = True
        
        self.bitCount = self.getSize()
        self.offset = self.getOffset()
        self.startBit = self.bitCount + self.offset

    def getSize(self):
        count = 0
        n = self.mask
        while (n): 
            count += n & 1
            n >>= 1
        return count 

    def getOffset(self):
        check = self.mask
        count = 0
        while not check & 1:
            check = check >> 1
            count+=1
    
        return count
    
    def addVal(self, fieldVal):
        fieldVal.field = self
        self.vals.append(fieldVal)
        self.valDict[fieldVal.name] = fieldVal

    def getFieldMaskMacro(self, spacing = 0):
        ret = self.reg.device.prefix.upper() +"_"+self.reg.name.upper()+"_"+self.name.upper() +"_FIELD_MASK"

        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret
    
    def getFieldOffsetkMacro(self, spacing = 0):
        ret = self.reg.device.prefix.upper() +"_"+self.reg.name.upper()+"_"+self.name.upper() +"_FIELD_OFFSET"

        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret

    def getFieldFlagMacro(self, spacing = 0):
        ret = self.reg.device.prefix.upper() +"_"+ self.reg.name.upper()+"_"+self.name.upper()

        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret

    def getToolTip(self):
        message = ""

        for val in self.vals:
            if self.bitCount > 6 :
                message +="\n\tx" + format(val.val, '02x') +" = " + val.desc
            else:
                message +="\n\tb" + format(val.val, '0'+str(self.bitCount)+'b') +" = " + val.desc
        
        return self.desc, message

    def getRstTable(self):
        
        t =TemplateHelper()
        lName =12
        lVal =12
        lDesc =12
        lines = []

        for val in self.vals:
            if len(val.name) > lName:
                lName = len(val.name) 

            if len(val.formatVal()) > lVal:
                lVal = len(reg.formatVal())

            if len(val.desc) > lDesc:
                lDesc = len(val.desc)
        
        lName +=2
        lVal +=2
        lDesc +=2

        divLine = "+" + ("-"* lName) + "+" + ("-"* lVal) + "+" + ("-"* lDesc)  + "+"
        titleLine = "|" + t.padAfter("Name", lName) + "|" + t.padAfter("Address", lVal) + "|" + t.padAfter("Description", lDesc)  + "|"
        headLine = divLine.replace("-","=")

        lines.append(divLine)
        lines.append(titleLine)
        lines.append(headLine)

        for val in self.vals:
            line = "|" + t.padAfter(val.name, lName) + "|" + t.padAfter(val.formatVal(), lVal) +"|" + t.padAfter(val.desc, lDesc)  + "|"
            lines.append(line)
            lines.append(divLine)


        return '\n'.join(lines)

    def getDict(self):
        val_arr = []
        for val in self.vals:
            val_arr.append(val.getDict())

        json_dict = { "mask": self.mask, "size": self.bitCount, "offset": self.offset, "desc": self.desc, "vals": val_arr}

        return json_dict
    
    
        

class DeviceReg:
    def __init__(self,name):
        self.name = name
        self.addr = 0
        self.type ="uint8"
        self.perm = "RW"
        self.desc = ""
        self.default = 0
        self.hasDefault = False
        self.fields = []
        self.fieldDict = {}
        self.flags = []
        self.size = 1
        self.device = 0
        self.hasFlags = False
        self.hasFields = False
        self.configs = {}
    
    def addField(self, field):
        field.reg = self
        if field.isFlag:
            self.hasFlags = True
        else:
            self.hasFields = True

        self.fieldDict[field.name] = field
        self.fields.append(field)
    
    def addDefaultField(self):

        fieldNode = {
            "name": self.name,
            "desc": self.desc
        }

        if self.size == 1:
            fieldNode['mask'] = 0xFF
        if self.size == 2: 
            fieldNode['mask'] = 0xFFFF
        if self.size == 4: 
            fieldNode['mask'] = 0xFFFFFFFF

        field = RegField(fieldNode)
        field.flat = True
        self.addField(field)
    
    def formatHex(self, val):
        return self.device.formatHex(val, self.size)

    def printAddr(self):
        val = "{0:#0{1}X}".format(self.addr,(self.device.addrSize *2) + 2)
        val = val.replace("X","x")
        return val
    
    def getNextfieldByStartBit(self, startBit):

        found = False
        for field in self.fields:
            if field.startBit == startBit:
                return field , field.bitCount
        
        search = startBit
        while search >= 0:
            search = search -1 
            for field in self.fields:
                if field.startBit == search:
                    return False , startBit - search
        
        return False, startBit

    
    
    def printFieldMap(self):
        ret =""
        i = self.size * 8
        while i > 0:
            field , nextStart = self.getNextfieldByStartBit(i)
            if field:
                ret = ret+"<td class=\"field\" colspan=\""+str(field.bitCount)+"\">" +field.name+"</td>\n"
                i -= field.bitCount
            else :
                ret = ret+"<td class=\"empty\" colspan=\""+ str( nextStart)+"\"></td>\n"
                i -= nextStart
        
        return ret
    
    
    def getAddrMacro(self, spacing = 0):
        ret = self.device.prefix.upper() +"_REG_"+self.name.upper()+"_ADDR"
        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret 
    
    def getDefaultMacro(self, spacing = 0):
        ret = self.device.prefix.upper() +"_"+self.name.upper()+"_DEFAULT"
        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret

        

    def getRstTable(self):
        
        t = TemplateHelper()
        cols = self.size * 8 
        bits = cols -1
        cellWidth = 3
        fieldBitMap = {}

        if len(self.fields) == 0:
            cellWidth = len(self.name)

        for field in self.fields:
            fieldBitMap[field.startBit] = field
            if len(field.name) > cellWidth:
                cellWidth = len(field.name)
        
        lines = ["+------------","|Bit         ", "+============", "| **Field**  ","+------------"]

        for i in range(bits,-1,-1):
            lines[0] += '+' + ("-"* cellWidth)
            lines[1] += '|' + t.padAfter(str(i),cellWidth) 
            lines[2] += '+' + ("="* cellWidth)

            if i in fieldBitMap:
                lines[3] += "|"+ t.padAfter(fieldBitMap[i].name, cellWidth) 
                lines[4] += '+' + ("-"* cellWidth)
            else: 
                if i == bits:
                    if len(self.fields) == 0:
                        lines[3] += '|' + t.padAfter(self.name,cellWidth)
                    else:
                        lines[3] += '|' + (" "* cellWidth)
                    lines[4] += '+' + ("-"* cellWidth)
                else:
                    lines[3] += ' ' + (" "* cellWidth)
                    lines[4] += '-' + ("-"* cellWidth)
        
        lines[0]+="+"
        lines[1]+="|"
        lines[2]+="+"
        lines[3]+="|"
        lines[4]+="+"

        return '\n'.join(lines)




        

    def getDict(self):
        field_dict = {}

        for field in self.fields:
            field_dict[field.name] = field.getDict();

        json_dict = {"name": self.name, "addr": self.addr, "type": self.type, "size": self.size * 8, "perm": self.perm.upper(), "default":self.default, "hasDefault":self.hasDefault, "desc": self.desc, "fields": field_dict}
        return json_dict
    



class Device:
    def __init__(self, name):
        self.name = name
        self.prefix = ""
        self.bus = "I2C"
        self.i2c_addr = 0
        self.addrSize =1
        self.aiMask = 0 #auto incrment mask 
        self.regs = {}
        self.datasheet =""
        self.desc=""
        self.digikey_pn =""
        self.smallestReg = 4 
        self.largestReg = 1
        self.configs = {}
        self.defaults = {}
        self.nextAddr = 0
        self.storageSize = 0

    def addReg(self, reg):
        reg.device = self
        if reg.size < self.smallestReg:
            self.smallestReg = reg.size
        
        if reg.size > self.largestReg:
            self.largestReg = reg.size
        
        if reg.hasDefault:
            self.defaults[reg.name] = reg.default

        self.regs[reg.name] = reg
    
    def addConfig(self,config):
        config.device = self
        self.configs[config.name] = config

    def getConfigLine(self,configReg,spacing =0, macro = False):
        ret = ""
        regName = list(configReg.keys())[0]
        regItem = list(configReg.values())[0]

        if regName in self.regs:
            curReg = self.regs[regName]
            if(macro):
                ret = self.prefix.lower() + "_write_reg( (dev), &(dev)->"
            else:
                ret = self.prefix.lower() + "_write_reg( dev, &dev->"
            ret+= "m"+ self.camelCase(regName) + ", "
            if type( regItem) is dict:
                val =0
                for key,value in regItem.items():
                    if key in curReg.fieldDict:
                        curField = curReg.fieldDict[key]
                        if type(value) is str:
                            if value in curField.valDict:
                                val = val | (curField.valDict[value].val << curField.offset) & curField.mask
                        else:
                            val = val | (value << curField.offset) & curField.mask
                
                ret+= self.formatHex(val, curReg.size)
            else: 
                ret+= self.formatHex(regItem, curReg.size)

            ret+=");"
        spaces = spacing - len(ret)
        ret+= (" "*spaces)
        return ret

    def getConfigRegVal(self,configReg):
        
        regName = list(configReg.keys())[0]
        regItem = list(configReg.values())[0]
        val = 0
        if regName in self.regs:
            curReg = self.regs[regName]
            if type( regItem) is dict:
                val =0
                for key,value in regItem.items():
                    if key in curReg.fieldDict:
                        curField = curReg.fieldDict[key]
                        if type(value) is str:
                            if value in curField.valDict:
                                val = val | (curField.valDict[value].val << curField.offset) & curField.mask
                        else:
                            val = val | (value << curField.offset) & curField.mask
                
            else: 
                val = regItem

        return regName, val
        

    def formatHex(self, val, size):
        val = "{0:#0{1}X}".format(val,(size *2) + 2)
        val = val.replace("X","x")
        return val

    def camelCase(self, text):
        out =""
        cap = True 
        for char in text:
            if char != '_':
                if cap:
                    out+= char.upper()
                else:
                    out+= char.lower()
                cap = False
            else:
                cap = True
            
        return out

    def parseYAML(self,yamlFile):
        data = open(yamlFile)
        objDevice = yaml.load(data , Loader=yaml.FullLoader)

        if 'name' in objDevice:
            self.name = objDevice['name']
        
        if 'prefix' in objDevice:
            self.prefix = objDevice['prefix']
        
        if 'bus' in objDevice:
            self.bus = objDevice['bus']
        
        if 'digikey_pn' in objDevice:
            self.digikey_pn = objDevice['digikey_pn']

        if 'mfg_pn' in objDevice:
            self.mfg_pn = objDevice['mfg_pn']
        
        if 'desc' in objDevice:
            self.desc = objDevice['desc']
        
        if 'description' in objDevice:
            self.desc = objDevice['description']
        
        if 'i2c_addr' in objDevice:
            self.i2c_addr = objDevice['i2c_addr']

        if 'datasheet' in objDevice:
            self.datasheet = objDevice['datasheet']
        
        if 'addr_size' in objDevice:
            self.addr_size = objDevice['addr_size']
        
        if 'ai_mask' in objDevice:
            self.aiMask = objDevice['ai_mask'] #int(objDecive['ai_mask'],0)
        


        if 'registers' in objDevice:
            regs = yamlNormalizeNodes( objDevice['registers'], 'reg_name','addr')
            for regItem in regs:
                newReg = DeviceReg(regItem['reg_name'])
                
                if 'type' in regItem:
                    newReg.type = regItem['type'].replace("_t","")
                    newReg.size = sizeDict[newReg.type.replace("_t","")]
                if 'size' in regItem:
                    newReg.size = regItem['size']
                if 'perm' in regItem:
                    newReg.perm = regItem['perm'].upper()
                if 'desc' in regItem:
                    newReg.desc = regItem['desc']
                if 'name' in regItem:
                    newReg.desc = regItem['name']
                if 'default' in regItem:
                    newReg.default = regItem['default']
                    newReg.hasDefault = True
                
                
                if 'addr' in regItem:
                    newReg.addr = regItem['addr'] #int(regItem['addr'],0) 
                else:
                    newReg.addr = self.nextAddr

                self.nextAddr = newReg.addr +  newReg.size

                self.storageSize += newReg.size
                self.addReg(newReg)      
        
        if 'fields' in objDevice:
            for propNode in objDevice['fields']:
                if type(propNode) is dict:
                    regName = list(propNode.keys())[0]
                    propItem = list(propNode.values())[0]
                    if regName in self.regs:
                        curReg = self.regs[regName]  
                        fieldNodes = yamlNormalizeNodes(propItem, 'name', 'mask')
                        for fieldNode in fieldNodes:
                            newField = RegField(fieldNode)
                            curReg.addField(newField)
                else:
                    regName = propNode
                    if regName in self.regs:
                        self.regs[regName].addDefaultField()
        
        if 'configs' in objDevice:
            for configNode in objDevice['configs']:
                configItem = DevConfig(configNode)
                self.addConfig(configItem)
        
        if 'configurations' in objDevice:
            for configNode in objDevice['configurations']:
                configItem = DevConfig(configNode)
                self.addConfig(configItem)

        #if a register has no fields but has a default value, make it a flat reg/field
        for key,value in self.regs.items():
            if len(value.fields) == 0 and value.hasDefault:
                value.addDefaultField() 

        print("Parsed device: " + self.name )
        print( "registers: " + str(len(self.regs)))

    def getDict(self):

        reg_dict ={}
        cfg_dict = {}
        for key, reg in self.regs.items():
            reg_dict[key] = reg.getDict()
        
        cfg_dict['Default'] = self.defaults
        for key, cfg in self.configs.items():
            cfg_dict[key] = cfg.getDict()

        json_dict = {"name": self.name, "regs": reg_dict, "configs": cfg_dict, "currentVals": self.defaults, "smallestReg" : self.smallestReg * 8 , "largestReg": self.largestReg * 8}

        return json_dict

    def getRstTable(self):
        
        t =TemplateHelper()
        lName =12
        lAddr =12
        lType =12
        lAccess =12
        lDef =12
        lDesc =12
        lines = []

        for key,reg in self.regs.items():
            if len(reg.name) > lName:
                lName = len(reg.name) 

            if len(reg.printAddr()) > lAddr:
                lAddr = len(reg.printAddr())

            if len(reg.type) > lType:
                lType = len(reg.type)

            if len(reg.perm) > lAccess:
                lAccess = len(reg.perm)

            if len(reg.formatHex(reg.default)) > lDef:
                lDef = len(reg.formatHex(reg.default))

            if len(reg.desc) > lDesc:
                lDesc = len(reg.desc)
        
        lName +=2
        lAddr +=2
        lType +=2
        lAccess +=2
        lDef +=2
        lDesc +=2

        divLine = "+" + ("-"* lName) + "+" + ("-"* lAddr) + "+" + ("-"* lType)  + "+" + ("-"* lAccess) + "+" + ("-"* lDef) + "+" + ("-"* lDesc)  + "+"
        titleLine = "|" + t.padAfter("Name", lName) + "|" + t.padAfter("Address", lAddr) + "|" + t.padAfter("Type", lType)  + "|" + t.padAfter("Access", lAccess) + "|" + t.padAfter("Default", lDef) + "|" + t.padAfter("Description", lDesc)  + "|"
        headLine = divLine.replace("-","=")

        lines.append(divLine)
        lines.append(titleLine)
        lines.append(headLine)

        for key,reg in self.regs.items():
            line = "|" + t.padAfter(reg.name, lName) + "|" + t.padAfter(reg.printAddr(), lAddr) + "|" + t.padAfter(reg.type, lType)  + "|" + t.padAfter(reg.perm, lAccess) + "|" + t.padAfter(reg.formatHex(reg.default), lDef) + "|" + t.padAfter(reg.desc, lDesc)  + "|"
            lines.append(line)
            lines.append(divLine)


        return '\n'.join(lines)
    
    def printJSON(self):
        return json.dumps(self.getDict());





        



        