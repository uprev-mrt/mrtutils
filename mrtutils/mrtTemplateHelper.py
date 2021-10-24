#!/usr/bin/env python

import sys
import os
from sys import platform
from mako.template import Template
import pkgutil
from datetime import date

import re

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

cTypeDict = {
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

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

class CodeReplacer:
    def __init__(self):
        self.input =""
        self.blocks = {}
        self.blocksUsed = {}
        
    
    def loadText(self, text, regex=r"\*user-block-(.*?)-start(.*?)\*user-block-\1-end\*"):
        rx = re.compile(regex,re.DOTALL)

        for match in rx.finditer(text):
            token = match.groups(0)[0]
            block = match.group()
            self.blocks[token] = block
            self.blocksUsed[token] = False
            # print("**************************************************")
            # print( "token: "+token)
            # print("**************************************************")
            # print(block)
            # print("**************************************************")
            


    def insertBlocks(self, text, regex=r"\*user-block-(.*?)-start(.*?)\*?user-block-\1-end\*"):
        rx = re.compile(regex,re.DOTALL)
        
        for match in rx.finditer(text):
            token = match.groups(0)[0]
            if token in self.blocks:
                self.blocksUsed[token] = True
                text = text.replace(match.group(), self.blocks[token])
        
        
        return text

    def printDrops(self):
        
        for token, used in self.blocksUsed.items():
            if not used:
                print( bcolors.WARNING +" ******************************** Code block dropped:" + token+ " *************************************" + bcolors.ENDC)
                print( bcolors.OKCYAN + self.blocks[token] + bcolors.ENDC )

class TemplateHelper:
    def __init__(self):
        self.sizeDict = sizeDict
        self.cTypeDict = cTypeDict

    def getDate(self):
        return date.today().strftime("%m/%d/%y")


    def buildTemplate(object, templateFile, outputFile, replacePattern = r"@file (.*?\.)(.*?) \*/"):
        exists= False
        cr = CodeReplacer()
        newContents =""
        if os.path.isfile(outputFile):
            exists = True
            curFile = open(outputFile, "r")
            text = curFile.read()
            cr.loadText(text,replacePattern)
            cr.loadText(text)

        template = Template(pkgutil.get_data('mrtutils',templateFile) )
        newContents = "\n".join(template.render(obj = object, t = TemplateHelper()).splitlines())
        newContents = cr.insertBlocks(newContents,replacePattern)
        newContents = cr.insertBlocks(newContents)
        cr.printDrops()
        text_file = open( outputFile , "w")
        text_file.write(newContents)
        text_file.close()

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
    
    def padAfter(self, text, spacing):
        ret = text
        if spacing > 0:
            spaces = spacing - len(ret)
            ret = ret + (" " * spaces)
        return ret 





