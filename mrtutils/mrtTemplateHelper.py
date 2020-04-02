#!/usr/bin/env python

import sys
import os
from sys import platform
import re

class CodeReplacer:
    def __init__(self):
        self.input =""
        self.blocks = {}
        self.blocksUsed = {}
        
    
    def loadText(self, text, regex=r"\*user-block-(.*?)-start\*(.*?)\*user-block-\1-end\*"):
        rx = re.compile(regex,re.DOTALL)

        for match in rx.finditer(text):
            token = match.groups(0)[0]
            block = match.group()
            self.blocks[token] = block
            self.blocksUsed[token] = False


    def insertBlocks(self, text, regex=r"\*user-block-(.*?)-start\*(.*?)\*?user-block-\1-end\*"):
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
                print(" Code block dropped: " + token)
                print(self.blocks[token])

class TemplateHelper:
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





