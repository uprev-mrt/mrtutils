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
import urllib.request
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

class GattCharacteristic:
    def __init__(self):
        self.name = ""
        self.uri = ""
        self.desc= None
    
    def loadUri(self,uri):
        print(uri)
        url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/" + uri + ".xml"
        root = getXml(url)
        self.uri = uri

        self.name = root.attrib['name']
        self.uuid = root.attrib['uuid']
        self.type = root.find('./Value/Field/Format').text
        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary') )

class GattService:
    def __init__(self):

        #load uri first so we can override attributes as needed 
        
        self.name = "unnamed"
        self.chars = []
    
    def loadUri(self, uri):
        url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/" + uri + ".xml"
        root = getXml(url)

        self.name = root.attrib['name']
        self.uuid = root.attrib['uuid']
        self.desc = root.find('./InformativeText/Abstract').text

        for xmlChar in root.findall("./Characteristics/Characteristic"):
            if xmlChar.find('./Requirement').text == 'Optional':
                newChar = GattCharacteristic()
                newChar.loadUri(xmlChar.attrib['type'])
                self.chars.append(newChar)

    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])

        yamlGetAttributes(node, attrDict, self)

class GattProfile:
    def __init__(self, file):
        self.name= "unnamed"
        self.services = []


    def parseYaml(self, file):
        data = open(file)
        nodeProfile = yaml.load(data, Loader = yaml.FullLoader)

        yamlGetAttributes(nodeProfile, attrDict, self)

        serviceNodes = yamlNormalizeNodes( objDevice['registers'], 'name','uri')
        for serviceNode in serviceNodes:
            newService = GattService(serviceNode)
            yamlGetAttributes(nodeProfile, attrDict, self)
            self.addService(newService)

    def addService(self, service):
        self.services.append(service)



svc = GattService()
svc.loadUri('org.bluetooth.service.device_information')




        



        