#!/usr/bin/python3

import ctypes


#
# class disable_file_system_redirection:
#     _disable = ctypes.windll.kernel32.Wow64DisableWow64FsRedirection
#     _revert = ctypes.windll.kernel32.Wow64RevertWow64FsRedirection
#     def __enter__(self):
#         self.old_value = ctypes.c_long()
#         self.success = self._disable(ctypes.byref(self.old_value))
#     def __exit__(self, type, value, traceback):
#         if self.success:
#             self._revert(self.old_value)


import sys
import os
from sys import platform
import os.path
import re
import subprocess
import markdown
from threading import Thread
from urllib.request import urlopen
import yaml

def getGitFileText(url, file):
    txt=""
    try:
        str = 'git archive --remote=' + url +' HEAD '+ file + ' | tar -xOf - ' + file
        txt = subprocess.check_output(str,shell=True, encoding='utf8')
    except subprocess.CalledProcessError as e:
        print(" Repo at " + url+ " does not contain " + file)

    return txt

def getBitbucketFile(account, repo, file):
    txt=""
    try:
        response = urlopen("https://bitbucket.org/" + account + "/" + repo + "/raw/master/" + file)
        html_content = response.read()
        encoding = response.headers.get_content_charset('utf-8')
        txt = html_content.decode(encoding)
    except:
        print("bitbucket.org/" + account + "/" + repo + "/raw/master/" + file + "  not found")
    
    return txt

class Submodule:
    def __init__(self, path, url):
        self.url = url
        self.mrtprops = {} 
        self.git_url = url
        self.path = path
        self.nodes = path.split('/')
        self.name = self.nodes[-1]
        self.exists = False
        self.requirements = []
        self.readme="x"
        self.repo_name = url.split('/')[-1].replace(".git","")


    def checkFor(self,mods):
        global relativePath
        self.exists = False
        for mod in mods:
            if(self.name == mod.name):
                self.exists = True


    def getReadMe(self):
        if self.url.find("bitbucket"):
            self.readme = getBitbucketFile('uprev',self.repo_name, 'README.md')
        else:
            self.readme = getGitFileText('ssh://' + self.url.replace(':','/'), 'README.md')

        if(self.readme == ""):
            self.readme = "# No README.md is available for this module"

    def getYamlInfo(self):

        yamlText = ""

        try:
            if self.url.find("bitbucket"):
                yamlText = getBitbucketFile('uprev',self.repo_name, 'mrt.yml')
            else:
                yamlText = getGitFileText('ssh://' + self.url.replace(':','/'), 'mrt.yml')

            if not yamlText == "":
                self.mrtprops = yaml.load(yamlText)
        except:
            print("couldnt load yaml")
    
    def getProp(self,key):
        if key in self.mrtprops:
            return self.mrtprops[key]
        else:
            return None




    def gatherRequirements(self):
            self.requirements = []
            #read in all modules
            regex = re.compile(r'>Requires: (.*?)')
            reqs = regex.findall(self.readme)
            for req in requirements:
                self.requirements.append(req[1])


class RepoDirectory:
    def __init__(self, name,lvl):
        self.name = name
        self.dirs = {}
        self.mods = {}
        self.mod = Submodule('','')
        self.isMod = False
        self.lvl = lvl

    def setMod(self,mod):
        self.mod = mod
        self.isMod = True


    def add(self, mod, lvl):

        nodes = mod.path.split('/')[lvl:]

        #print nodes[lvl:]

        if(len(nodes) > 1):
            node = nodes[0]
            if not node in self.dirs:
                self.dirs[node] = RepoDirectory(node, lvl)
            self.dirs[node].add(mod,lvl+1)
            if node in self.mods:
                self.dirs[node].setMod(self.mods[node])
                del self.mods[node]
        else:
            if mod.name in self.dirs:
                self.dirs[mod.name].set(mod)
            else:
                self.mods[mod.name] = mod

    # def addModToTree(self, mod):
    #     nodes = mod.path.split('/')
    #     lvl = 0
    #     dir = self 
    #     for node in nodes:
    #         if node in dir.dirs:

    def printout(self,lvl):
        start =""
        for i in range(lvl):
            start = start+"  "
            sys.stdout.write("──")

        sys.stdout.write(self.name + "\n")

        for mod in self.mods.values():
            if mod == list(self.mods)[-1]:
                sys.stdout.write(start + "└─%─" +mod.name +"\n\n")
            else:
                sys.stdout.write(start + "├──" +mod.name +"\n")

        for dir in self.dirs.values():
            dir.printout(lvl+1)

    def getKConfigString(self, lvl, title = "Config"):
        fileTxt = ""
        if lvl == 0:
            fileTxt += "mainmenu  \"{0}\"\n".format(title)
        else:
            fileTxt += "menu \"{0}\"\n".format(self.name)

        
        for dir in self.dirs.values():
            fileTxt+= dir.getKConfigString(lvl+1)

        for mod in self.mods.values():
            fileTxt += "config ENABLE_{0}\n".format(mod.name.upper())
            fileTxt += "\tbool \"{0}\"\n".format(mod.name)

            
            if mod.exists:
                fileTxt += "\tdefault y\n"
            else:
                fileTxt += "\tdefault n\n"
            
            for req in mod.requirements:
                fileTxt+="\tselect ENABLE_{0}".format(req.upper())

            if not mod.getProp("description") == None:
                fileTxt +="\thelp\n\t\t{0}\n".format(mod.getProp("description"))

            fileTxt += "\n"

        if not lvl == 0:
            fileTxt += "endmenu\n\n"
        return fileTxt


class Repo:
    def __init__(self, path, remote = False):
        self.url = path
        self.path = path
        self.mods =[]
        self.isRemote = remote
        self.dir = RepoDirectory("root",0)
        self.isBitbucket = False
        nodes = self.url.split('/')
        self.account = nodes[-2]
        self.name = nodes[-1].replace(".git","")
        if "bitbucket" in path:
            self.isBitbucket = True

    def setRelativePath(self,path):
        self.relativePath = path
    
    def crossCheckMods(self,repo):
        for mod in self.mods:
            mod.checkFor(repo.mods)

    def getYamlProps(self):
        for mod in self.mods:
            mod.getYamlInfo()

    def getSubModules(self):
        data = ""
        if self.isRemote:
            if(self.isBitbucket):
                data = getBitbucketFile(self.account, self.name, '.gitmodules').replace('\r', '').replace('\n', '').replace('\t', '')
            else:
                data = getGitFileText(self.url,'.gitmodules').replace('\r', '').replace('\n', '').replace('\t', '')
            
            #read in all modules
            regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
            modules = regex.findall(data)
            for mod in modules:
                if(mod[1] != "Config"):
                    self.mods.append(Submodule(mod[1], mod[2]))
        else:
            if os.path.isfile(self.path + "/.gitmodules"):
                file = open(self.path + "/.gitmodules", "r")
                data = file.read().replace('\r', '').replace('\n', '').replace('\t', '')
                #read in all modules
                regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
                modules = regex.findall(data)
                for mod in modules:
                    self.mods.append(Submodule(mod[1], mod[2]))
        
        for mod in self.mods:
            self.dir.add(mod, 0)

    def getReadMe(self):

        data = ""
        if self.isRemote:
            if(self.isBitbucket):
                nodes = self.url.split('/')
                name = nodes[-1]
                account = nodes[-2]
                data = getBitbucketFile(self.account, self.name, 'README.md')
            else:
                data = getGitFileText(self.url, 'README.md')

        return data

    def findMod(self, name):
        for mod in self.mods: 
            if mod.name.lower() == name.lower():
                return mod 
        
        return None

    def fetchReadmes(self):
        print ("Fetching README.md from modules")
        for mod in self.mods:
            mod.getReadMe()

    def addSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        print("Adding Submodule: " + mod.git_url)
        subprocess.check_output(['git','submodule','add', '-f',mod.git_url,  self.relativePath + mod.path] )
        os.chdir(prev_path)
        mod.exists = True

    def removeSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        print("Removing Submodule: " + mod.git_url)
        subprocess.check_output(['git','rm', '-f', self.relativePath + mod.path ])
        os.chdir(prev_path)
        mod.exists = False
    
    def getKConfigString(self):
        return self.dir.getKConfigString(0)

    def getConfigString(self):
        text = ""
        for mod in self.mods:
            if mod.exists:
                text += "CONFIG_ENABLE_{0}=y\n".format(mod.name.upper())
            else:
                text += "CONFIG_ENABLE_{0}=n\n".format(mod.name.upper())
        return text



