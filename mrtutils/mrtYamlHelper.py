#!/usr/bin/env python

import sys
import os
from sys import platform
import yaml

##  Object can be defined a few ways, this function will normalize them
##
##   #0  {keykey: name, field1: val1, field2: val2} # this is what we want
##   #1  name: { field1: val1, field2: val2} = { keyKey: name,  field1: val1, field2: val2}
     #2  name: value                         = { keyKey: name, valKey: value}
     #3  name                                 = {keyKey: name, valKey: name}
## 
##
def yamlNormalizeNode(node, keyKey, valKey,allowSingle):
    retVal = {}

    if type(node) is dict: 
        #if the node is a dict with multiple values, it is a #0, already normalized
        if(len(node) > 1):
            return node

        #otherwise node is either a #1 or #2
        #get key/value
        nodeKey = list(node.keys())[0]
        nodeVal = list(node.values())[0]
        #if the dict has 1 key/value and the value is a dict we have a #1
        if type(nodeVal) is dict: 
            nodeVal[keyKey] = nodeKey
            return nodeVal
        else:
            if(allowSingle):
                retVal[keyKey] = nodeKey
                retVal[valKey] = nodeVal
                return retVal
            else:
                return node
    
    else:
        retVal[keyKey] = node 
        retVal[valKey] = node
        return retVal
    return node

def yamlNormalizeNodes(nodes, keyKey, valKey , allowSingle = True):

    retNodes = []
    for node in nodes:
        newNode = yamlNormalizeNode(node ,keyKey, valKey, allowSingle)
        retNodes.append(newNode)
    
    return retNodes

def yamlGetAttributes(node, attrDict, obj):

    for key, value in node.items():
        if type(value) is not list:
            if key in attrDict:
                newKey = attrDict[key]
                obj.__dict__[newKey] = value 
            else:
                obj.__dict__[key] = value

    

