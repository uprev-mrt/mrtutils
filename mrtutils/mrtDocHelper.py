#!/usr/bin/env python

import sys
import os
from mrtutils.mrtTemplateHelper import *
from mako.template import Template
import argparse
from datetime import date
import getpass



args = None
parser = None

class DocConfig:
    def __init__(self, path):
        self.path = path
        self.gentime = date.today().strftime("%m/%d/%y")
        self.user = getpass.getuser()


class DocHelper:
    def __init__(self):
        self.name = ""


    def copyFile(self, srcfile, outputFile):
        src_data = pkgutil.get_data('mrtutils',srcfile)
        dst = open( outputFile , "wb")
        dst.write(src_data)
        dst.close()
        print("Created " + outputFile)


    def buildTemplate(self, object, templateFile, outputFile, replacePattern = r"@file (.*?\.)(.*?) \*/"):
        exists= False
        cr = CodeReplacer()
        newContents =""
        if os.path.isfile(outputFile):
            exists = True
            curFile = open(outputFile, "r")
            text = curFile.read()
            cr.loadText(text)
            cr.loadText(text,replacePattern)

        template = Template(pkgutil.get_data('mrtutils',templateFile) )
        newContents = "\n".join(template.render(obj = object, t = TemplateHelper()).splitlines())
        newContents = cr.insertBlocks(newContents)
        newContents = cr.insertBlocks(newContents,replacePattern)
        cr.printDrops()
        text_file = open( outputFile , "w")
        text_file.write(newContents)
        text_file.close()
        print("Created " + outputFile)




    def buildDocStructure(self,path):

        obj = DocConfig(path)


        os.mkdir(obj.path) 
        os.mkdir(obj.path +"/pages") 
        os.mkdir(obj.path +"/assets") 
        os.mkdir(obj.path +"/assets/diagrams") 
        self.buildTemplate(obj, "templates/doc/Makefile", obj.path +"/Makefile")
        self.buildTemplate(obj, "templates/doc/index.rst", obj.path +"/index.rst")
        self.buildTemplate(obj, "templates/doc/conf.py", obj.path +"/conf.py")
        self.buildTemplate(obj, "templates/doc/samplepage.rst", obj.path +"/pages/samplepage.rst")
        self.copyFile( "templates/doc/samplediagram.dio.png", obj.path +"/assets/diagrams/samplediagram.dio.png")

