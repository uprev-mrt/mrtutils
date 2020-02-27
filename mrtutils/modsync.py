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
    # try:
    response = urlopen("https://bitbucket.org/" + account + "/" + repo + "/raw/master/" + file)
    html_content = response.read()
    encoding = response.headers.get_content_charset('utf-8')
    txt = html_content.decode(encoding)
    # except:
    #     print("bitbucket.org/" + account + "/" + repo + "/raw/master/" + file + "  not found")
    
    return txt

class Submodule:
    def __init__(self, path, url):
        self.url = url

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
        for mod in mods:
            if(self.url == mod.url):
                self.exists = True

    def getReadMe(self):
        print ("pulling readme for: " + self.url)

        if self.url.find("bitbucket"):
            self.readme = getBitbucketFile('uprev',self.repo_name, 'README.md')
        else:
            self.readme = getGitFileText('ssh://' + self.url.replace(':','/'), 'README.md')

        if(self.readme == ""):
            self.readme = "# No README.md is available for this module"


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

    def printout(self,lvl):
        start =""
        for i in range(lvl):
            start = start+"   "
            sys.stdout.write("---")

        sys.stdout.write(self.name + "\n")

        for mod in self.mods.values():
            sys.stdout.write(start + "+" +mod.name +"\n")

        for dir in self.dirs.values():
            dir.printout(lvl+1)


class Repo:
    def __init__(self, path):
        self.url = path
        self.path = path
        self.mods =[]
        self.isRemote = True
        self.dir = RepoDirectory("root",0)

    def setRelativePath(self,path):
        self.relativePath = path
        self.isRemote = False;

    def getSubModules(self):
        if self.isRemote:
            data = getGitFileText(self.url,'.gitmodules').replace('\r', '').replace('\n', '').replace('\t', '')
            #read in all modules
            regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
            modules = regex.findall(data)
            for mod in modules:
                if(mod[1] != "Config"):
                    self.mods.append(Submodule(mod[1], mod[2]))
        else:
            if os.path.isfile(self.path):
                file = open(self.path, "r")
                data = file.read().replace('\r', '').replace('\n', '').replace('\t', '')

                #read in all modules
                regex = re.compile(r'\[(.*?)].*?path = (.*?)url = (.*?\.git)')
                modules = regex.findall(data)
                for mod in modules:
                    self.mods.append(Submodule(mod[1], mod[2]))

    def getReadMe(self):
        if self.isRemote:
            data = getGitFileText(self.url, 'README.md')
        return data

    def fetchReadmes(self):
        print ("Fetching README.md from modules")
        for mod in self.mods:
            mod.getReadMe()

    def addSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        subprocess.check_output(['git','submodule','add', mod.git_url,  self.relativePath + mod.path] )
        os.chdir(prev_path)
        mod.exists = True

    def removeSubModule(self,mod):
        prev_path = os.getcwd()
        os.chdir(self.path)
        subprocess.check_output(['git','--git-dir',self.path,'submodule','rm', self.relativePath + mod.path ])
        os.chdir(prev_path)
        mod.exists = False
