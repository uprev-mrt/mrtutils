#!/usr/bin/env python3
#
#@file gatt_profile
#@brief Gatt Profile descriptor
#@author Jason Berger
#@date 02/19/2019
#

import sys
import os
import yaml
import re
import json
import datetime
import urllib.request
from mrtutils.mrtYamlHelper import *
import xml.etree.ElementTree as ET 


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
    "utfs8": 16
}

svcIconDict = {
    "Device" : "fa-desktop",
    "Battery" : "fa-battery-three-quarters",
    "Environmental" : "fa-leaf"
}

charIconDict = {
    "Level" : "fa-battery-half",
    "Firmware_Revision" : "fa-code",
    "Temperature" : "fa-thermometer-empty",
    "Humidity" : "fa-tint",
}


typeDict ={
    "utf8s" : "string",
    "sint16" : "uint16",
    "sint" : "int",
    "sint8" : "int8"
}

attrDict = {
    "description" : "desc",
    "characteristics" : "chars",
    "values" : "vals",
    "value" : "val",
    "permissions" : "perm"
}

def getXml(url):
    headers={ 'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.47 Safari/537.36'}
    req = urllib.request.Request(url,data=None, headers = headers)
    file = urllib.request.urlopen(req)
    data = file.read()
    file.close()
    root = ET.fromstring(data.decode("utf-8"))

    return root

def insert_dash(string, index):
    return string[:index] + '-' + string[index:]

def uuidStr(val , short = False):
    arr =[]
    if type(val) != str:
        val = "0000%0.4X-0000-1000-8000-00805f9b34fb" % val
        ret = val
    else:  
        val = val.replace('-','')
        arr = [val[i:i+2] for i in range(0, len(val), 2)]
        ret = ''.join(arr)

        if len(ret) > 4:
            ret = insert_dash(ret, 8)
            ret = insert_dash(ret, 13)
            ret = insert_dash(ret, 18)
            ret = insert_dash(ret, 23)
    
    if short:
        ret = ret[4:8]

    return  ret.lower() 

def setIfEmpty(obj, field, new):
    if obj.__dict__[field] is None and new is not None:
        obj.__dict__[field] = new.text.strip()

class GattValue:
    def __init__(self):
        self.name = ""
        self.value = 0
        self.desc = None
    
    def parseYaml(self, node):
        yamlGetAttributes(node, attrDict, self)

    def getDict(self):

        json_dict = { "name": self.name, "val": self.value }

        return json_dict

class GattCharacteristic(object):
    def __init__(self):
        self.name = ""
        self.uri = ""
        self.desc= None
        self.vals = []
        self.isEnum = False 
        self.isMask = False
        self.valsFormat = "0x%0.2X"
        self.type = 'uint8'
        self.isStandard = False
        self.perm = None
        self.url =""
        self.uuid = None
        self.uuidType = "e128Bit"
        self.icon = ''
        self.coef = 1
        self.unit = ''
        self.arrayLen = 1
        self.nextVal = 0
        self.style = ''
    
    def uuidArray(self):
        val = self.uuid 
        arr =[]
        if type(val) != str:
            val = "%0.4X" % val
            self.uuidType ="e16Bit"
        
        val = val.replace('-','')
        arr = [val[i:i+2] for i in range(0, len(val), 2)]
        ret = ', 0x'.join(arr[::-1])

        return "0x"+ ret 
    
    def size(self):
        return sizeDict[self.type] * self.arrayLen
    
    def uuidStr(self):
        
        if type(self.uuid) is str:
            return self.uuid.split('-')[1]
        else:
            return "%0.4X" % self.uuid
    
    def props(self):
        ret = ""

        props = []
        if 'r' in self.perm.lower():
            props.append('MRT_GATT_PROP_READ')
        if 'w' in self.perm.lower():
            props.append('MRT_GATT_PROP_WRITE')
        if 'n' in self.perm.lower():
            props.append('MRT_GATT_PROP_NOTIFY')
        if 'i' in self.perm.lower():
            props.append('MRT_GATT_PROP_INDICATE')
        
        if len(props) > 0:
            return " | ".join(props)
        else:
            return "MRT_GATT_PROP_NONE"


    
    def loadUri(self,uri):
        print("Pulling " + uri)
        self.url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/" + uri + ".xml"
        root = getXml(self.url)
        self.uri = uri
        self.isStandard = True

        self.name = root.find('./Value/Field').attrib['name'].replace(' ', '_')
        self.uuid = int(root.attrib['uuid'], 16)
        sigType = root.find('./Value/Field/Format').text
        if sigType in typeDict:
            sigType = typeDict[sigType]
        
        
        self.type = sigType

        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary') )
        setIfEmpty(self, 'desc',root.find('./InformativeText'))
        setIfEmpty(self, 'desc',root.find('./Value/Field/InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./Value/Field/InformativeText/Summary') )
        setIfEmpty(self, 'desc',root.find('./Value/Field/InformativeText'))
        if self.desc is None:
            self.desc = "No Descriptions"

        self.desc = self.desc.replace('\n','').replace('\t','')
    
    def loadServiceUri(self, uri):
        url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/" + uri + ".xml"
        root = getXml(url)
 
        xmlChar = root.find('./Characteristics/Characteristic[@type=\"' + self.uri + '\"]')

        sigPerm = ""
        if xmlChar.find('./Properties/Read').text in ['Mandatory','Optional']:
            sigPerm += "R"
        if xmlChar.find('./Properties/Write').text in ['Mandatory','Optional']:
            sigPerm += "W"
        if xmlChar.find('./Properties/Notify').text in ['Mandatory','Optional']:
            sigPerm += "N"
        if xmlChar.find('./Properties/Notify').text in ['Mandatory','Optional']:
            sigPerm += "I"
        self.perm = sigPerm

        if self.name in svcIconDict:
                self.icon = charIconDict[self.name]

    
    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])

        yamlGetAttributes(node, attrDict, self)

        if self.type in ['flag','flags','mask','bits']:
            self.isMask = True

        if self.type in ['enum','enums']:
            self.isEnum = True
        
        m = re.search('\*([0-9]*)', self.type)
        if(m):
            if(m.group(1) != ''):
                self.arrayLen = int(m.group(1))
            self.type = self.type[0:m.start()]

        if 'vals' in node:
            valNodes = yamlNormalizeNodes( node['vals'], 'name','desc')
            for valNode in valNodes:
                newVal = GattValue()
                newVal.parseYaml(valNode)
                self.addVal(newVal)
    
    def addVal(self, val):

        if self.isMask:
            val.value = 1 << self.nextVal
            strType = 'uint8'
            if len(self.vals) > 8:
                self.valsFormat = "0x%0.4X"
                strType = 'uint16'
            if len(self.vals) > 16:
                self.valsFormat = "0x%0.8X"
                strType = 'uint32'
            if len(self.vals) > 32:
                print( "Error maximum flags per field is 32")
            self.type = strType
        else:
            val.value = self.nextVal 
            self.type = 'uint8'
        
        self.nextVal = self.nextVal +1
        self.vals.append(val)
    
    def getDict(self):
        val_arr = [] #array of gattValues for enums/masks

        for val in self.vals:
            val_arr.append(val.getDict())


        json_dict = { "name": self.name, "id": self.name.replace(' ', '_'), "size": sizeDict[self.type], "arrayLen": self.arrayLen, "uuid": uuidStr(self.uuid), "short_uuid": uuidStr(self.uuid, True), "uri":self.uri ,"url": self.url, "type": self.type, "unit": self.unit, "coef":self.coef, "icon" : self.icon, "uuid_type": self.uuidType,  "perm": self.perm, "desc": self.desc.rstrip()  , "vals": val_arr}

        if(self.isEnum):
            json_dict['type'] = 'enum'

        if(self.isMask):
            json_dict['type'] = 'mask'

        if(self.style != ''):
            json_dict['style'] = self.style

        return json_dict

class GattService(object):
    def __init__(self):

        #load uri first so we can override attributes as needed 
        
        self.name = "unnamed"
        self.chars = []
        self.desc = None
        self.isStandard = False
        self.url =""
        self.uri = None
        self.nextUuid = 0
        self.uuidType ="e128Bit"
        self.profile = ''
        self.icon = ''
    
    def loadUri(self, uri):
        print("Pulling " + uri)
        self.url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/" + uri + ".xml"
        root = getXml(self.url)
        self.isStandard = True
        self.uri = uri

        self.name = root.attrib['name'].replace(' ', '_')
        self.uuid = int(root.attrib['uuid'],16)
        self.uuidType = 'e16Bit'
        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary'))


        for xmlChar in root.findall("./Characteristics/Characteristic"):
            if xmlChar.find('./Requirement').text == 'Mandatory':
                newChar = GattCharacteristic()
                newChar.loadUri(xmlChar.attrib['type'])
                newChar.loadServiceUri(self.uri)
                self.chars.append(newChar)
        
        self.desc = self.desc.replace('\n','').replace('\t','').strip()


    def uuidArray(self):
        val = self.uuid 
        arr =[]
        if type(val) != str:
            val = "%0.4X" % val
            self.uuidType ="e16Bit"
        
        val = val.replace('-','')
        arr = [val[i:i+2] for i in range(0, len(val), 2)]
        ret = ', 0x'.join(arr[::-1])

        return "0x"+ ret 

    def parseYaml(self, node):
        
        uuidSplit = []
        if 'uri' in node:
            self.loadUri(node['uri'])
            self.nextUuid = self.uuid + 1
            yamlGetAttributes(node, attrDict, self)
            if self.name in svcIconDict:
                self.icon = svcIconDict[self.name]
        else:
            yamlGetAttributes(node, attrDict, self)
            uuidSplit = self.uuid.split('-')
            self.nextUuid = int(uuidSplit[1], 16) + 1
        
        if 'chars' in node:
            chars = node['chars']
            charNodes = yamlNormalizeNodes( node['chars'], 'name', 'uri', False)
            for charNode in charNodes:
                newChar = GattCharacteristic()
                newChar.parseYaml(charNode)
                if self.uri is not None:
                    newChar.loadServiceUri(self.uri)
                if newChar.uuid is None:                        #if no uuid is set, increment previous
                    uuidSplit[1] = "%0.4X" % self.nextUuid
                    newChar.uuid = '-'.join(uuidSplit)
                    self.nextUuid += 1
                if type(newChar.uuid) != str and self.uri is None:   #if it is a  16 bit uuid without a uri, it incremements from the next uuid of the service
                    uuidSplit[1] = "%0.4X" % newChar.uuid
                    self.nextUuid = newChar.uuid + 1
                    newChar.uuid = '-'.join(uuidSplit)
                self.addChar(newChar)
    
    def addChar(self, char):
        self.chars.append(char)

    def getDict(self):
        char_arr = []

        for char in self.chars:
            char_arr.append(char.getDict())
        
        json_dict = { "name": self.name, "id": self.name.replace(' ', '_'), "uuid": uuidStr(self.uuid),  "short_uuid": uuidStr(self.uuid, True), "uuid_type": self.uuidType, "desc": self.desc, "icon" : self.icon, "uri": self.uri, "url": self.url, "characteristics": char_arr}

        return json_dict
             

class GattProfile(object):
    def __init__(self):
        self.name= "unnamed"
        self.services = []
        self.genTime = datetime.datetime.now().strftime("%m/%d/%y")
    
    def nrfServices(self, uuidType):

        serviceItems =[]
        for service in self.services:
            if service.uuidType == uuidType:
                item = '\t\t"{0}" : {{\n\t\t\t"name":"{1}",\n\t\t\t}}'.format(uuidStr(service.uuid), service.name)
                serviceItems.append(item)
        
        ret = ',\n'.join(serviceItems)
        if ret != "":
            ret = "\n" + ret 
        
        return ret
    
    def nrfChars(self,uuidType):
        charItems =[]
        for service in self.services:
            for char in service.chars:
                if char.uuidType == uuidType:
                    item = '\t\t"{0}" : {{\n\t\t\t"name":"{1}",\n\t\t\t"format":"{2}"\n\t\t\t}}'.format(uuidStr(char.uuid), char.name, "TEXT" if (char.type == 'string') else "NO_FORMAT" )
                    charItems.append(item)
        
        ret = ',\n'.join(charItems)
        if ret != "":
            ret = "\n" + ret
        
        return ret

    def parseYaml(self, file):
        data = open(file)
        nodeProfile = yaml.load(data, Loader = yaml.FullLoader)

        yamlGetAttributes(nodeProfile, attrDict, self)

        serviceNodes = yamlNormalizeNodes( nodeProfile['services'], 'name','uri')
        for serviceNode in serviceNodes:
            newService = GattService()
            newService.parseYaml(serviceNode)
            self.addService(newService)

    def addService(self, service):
        service.profile = self
        self.services.append(service)
    
    def getDict(self):
        svc_arr = []

        for svc in self.services:
            svc_arr.append(svc.getDict())
        
        json_dict = { "name": self.name,  "services": svc_arr}
        return json_dict
             




if __name__ == "__main__":
    prof = GattProfile()
    prof.parseYaml('templates/ble/gatt_descriptor_template.yml')
    





        



        