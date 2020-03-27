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

typeDict ={
    "utf8s" : "string"
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

def setIfEmpty(obj, field, new):
    if obj.__dict__[field] is None and new is not None:
        obj.__dict__[field] = new.text

class GattValue:
    def __init__(self):
        self.name = ""
        self.value = 0
        self.desc = None
    
    def parseYaml(self, node):
        yamlGetAttributes(node, attrDict, self)

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
    
    def loadUri(self,uri):
        self.url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/" + uri + ".xml"
        root = getXml(self.url)
        self.uri = uri
        self.isStandard = True

        self.name = root.attrib['name']
        self.uuid = int(root.attrib['uuid'], 16)
        sigType = root.find('./Value/Field/Format').text
        if sigType in typeDict:
            sigType = typeDict[sigType]
        
        
        self.type = sigType

        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary') )

        self.desc = self.desc.replace('\n','').replace('\t','')
    
    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])

        yamlGetAttributes(node, attrDict, self)

        if self.type in ['flag','flags','mask','bits']:
            self.isMask = True

        if self.type in ['enum','enums']:
            self.isEnum = True

        if 'vals' in node:
            valNodes = yamlNormalizeNodes( node['vals'], 'name','desc')
            for valNode in valNodes:
                newVal = GattValue()
                newVal.parseYaml(valNode)
                self.addVal(newVal)
    
    def addVal(self, val):
        self.vals.append(val)

        if self.isMask:
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

class GattService(object):
    def __init__(self):

        #load uri first so we can override attributes as needed 
        
        self.name = "unnamed"
        self.chars = []
        self.desc = None
        self.isStandard = False
        self.url =""
        self.nextUuid = 0
    
    def loadUri(self, uri):
        self.url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/" + uri + ".xml"
        root = getXml(self.url)
        self.isStandard = True

        self.name = root.attrib['name']
        self.uuid = int(root.attrib['uuid'],16)
        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary'))


        for xmlChar in root.findall("./Characteristics/Characteristic"):
            if xmlChar.find('./Requirement').text == 'Mandatory':
                newChar = GattCharacteristic()
                newChar.loadUri(xmlChar.attrib['type'])

                sigPerm = ""
                if xmlChar.find('./Properties/Read').text in ['Mandatory','Optional']:
                    sigPerm += "R"
                if xmlChar.find('./Properties/Write').text in ['Mandatory','Optional']:
                    sigPerm += "W"
                if xmlChar.find('./Properties/Notify').text in ['Mandatory','Optional']:
                    sigPerm += "N"
                if xmlChar.find('./Properties/Notify').text in ['Mandatory','Optional']:
                    sigPerm += "I"
                newChar.perm = sigPerm
                self.chars.append(newChar)
        
        self.desc = self.desc.replace('\n','').replace('\t','')

    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])
            self.nextUuid = self.uuid + 1
            yamlGetAttributes(node, attrDict, self)
        else:
            yamlGetAttributes(node, attrDict, self)
            uuidSplit = self.uuid.split('-')
            self.nextUuid = int(uuidSplit[6], 16) + 1
        
        if 'chars' in node:
            charNodes = yamlNormalizeNodes( node['chars'], 'name','uri')
            for charNode in charNodes:
                newChar = GattCharacteristic()
                newChar.parseYaml(charNode)
                if newChar.uuid is None:
                    newChar.uuid = self.nextUuid
                    self.nextUuid += 1
                self.addChar(newChar)
    
    def addChar(self, char):
        self.chars.append(char)

class GattProfile(object):
    def __init__(self):
        self.name= "unnamed"
        self.services = []
        self.genTime = datetime.datetime.now().strftime("%m/%d/%y")

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
        self.services.append(service)




if __name__ == "__main__":
    prof = GattProfile()
    prof.parseYaml('templates/ble/gatt_descriptor_template.yml')
    





        



        