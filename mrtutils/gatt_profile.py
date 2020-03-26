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

class GattCharacteristic(object):
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

        self.desc = self.desc.replace('\n','').replace('\t','')
    
    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])

        yamlGetAttributes(node, attrDict, self)

class GattService(object):
    def __init__(self):

        #load uri first so we can override attributes as needed 
        
        self.name = "unnamed"
        self.chars = []
        self.desc = None
    
    def loadUri(self, uri):
        url = "https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/" + uri + ".xml"
        root = getXml(url)

        self.name = root.attrib['name']
        self.uuid = root.attrib['uuid']
        setIfEmpty(self, 'desc',root.find('./InformativeText/Abstract'))
        setIfEmpty(self, 'desc',root.find('./InformativeText/Summary'))

        for xmlChar in root.findall("./Characteristics/Characteristic"):
            if xmlChar.find('./Requirement').text == 'Mandatory':
                newChar = GattCharacteristic()
                newChar.loadUri(xmlChar.attrib['type'])
                self.chars.append(newChar)
        
        self.desc = self.desc.replace('\n','').replace('\t','')

    def parseYaml(self, node):
        
        if 'uri' in node:
            self.loadUri(node['uri'])
        
        if 'chars' in node:
            charNodes = yamlNormalizeNodes( node['chars'], 'name','uri')
            for charNode in charNodes:
                newChar = GattCharacteristic()
                newChar.parseYaml(charNode)
                self.addChar(newChar)

        yamlGetAttributes(node, attrDict, self)
    
    def addChar(self, char):
        self.chars.append(char)

class GattProfile(object):
    def __init__(self):
        self.name= "unnamed"
        self.services = []


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
    prof.parseYaml('templates/gatt_descriptor_template.yml')
    s = json.dumps(prof.__dict__)
    print(s)





        



        