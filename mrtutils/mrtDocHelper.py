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
        os.mkdir(obj.path +"/_templates") 
        os.mkdir(obj.path +"/pages") 
        os.mkdir(obj.path +"/assets") 
        os.mkdir(obj.path +"/assets/diagrams") 
        os.mkdir(obj.path +"/assets/images") 
        self.buildTemplate(obj, "templates/doc/layout.html", obj.path +"/_templates/layout.html")
        self.buildTemplate(obj, "templates/doc/Makefile", obj.path +"/Makefile")
        self.buildTemplate(obj, "templates/doc/index.rst", obj.path +"/index.rst")
        self.buildTemplate(obj, "templates/doc/conf.py", obj.path +"/conf.py")
        self.buildTemplate(obj, "templates/doc/pages/examples.rst", obj.path +"/pages/examples.rst")
        self.copyFile( "templates/doc/assets/diagrams/system.dio.png", obj.path +"/assets/diagrams/system.dio.png")
        self.copyFile( "templates/doc/assets/diagrams/button.puml", obj.path +"/assets/diagrams/button.puml")
        self.copyFile( "templates/doc/assets/diagrams/led.puml", obj.path +"/assets/diagrams/led.puml")
        self.copyFile( "templates/doc/assets/diagrams/mockup.dio.png", obj.path +"/assets/diagrams/mockup.dio.png")
        self.copyFile( "templates/doc/assets/images/logo.png", obj.path +"/assets/images/logo.png")
        self.copyFile( "templates/doc/assets/images/favicon.ico", obj.path +"/assets/images/favicon.ico")
        self.copyFile( "templates/doc/assets/images/remote_button.png", obj.path +"/assets/images/remote_button.png")


